// Copyright (C) 2011 Red Hat, Inc. All rights reserved.
//
// This file is part of the thin-provisioning-tools source.
//
// thin-provisioning-tools is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// thin-provisioning-tools is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with thin-provisioning-tools.  If not, see
// <http://www.gnu.org/licenses/>.

#include "thin-provisioning/device_tree.h"
#include "thin-provisioning/metadata.h"

#include "persistent-data/file_utils.h"
#include "persistent-data/math_utils.h"
#include "persistent-data/space-maps/core.h"
#include "persistent-data/space-maps/disk.h"
#include "persistent-data/space-maps/disk_structures.h"

#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace base;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	using namespace superblock_detail;

	unsigned const METADATA_CACHE_SIZE = 1024;
	unsigned const BYTE_TO_SECTOR_SHIFT = 9;

	transaction_manager::ptr
	open_tm(block_manager<>::ptr bm) {
		space_map::ptr sm(new core_map(bm->get_nr_blocks()));
		sm->inc(SUPERBLOCK_LOCATION);
		transaction_manager::ptr tm(new transaction_manager(bm, sm));
		return tm;
	}

	void
	copy_space_maps(space_map::ptr lhs, space_map::ptr rhs) {
		for (block_address b = 0; b < rhs->get_nr_blocks(); b++) {
			uint32_t count = rhs->get_count(b);
			if (count > 0)
				lhs->set_count(b, rhs->get_count(b));
		}
	}

	// Returns possible locations of superblock backups according to the device size,
	// no matter what the actual metadata size is, to cope with superblock corruption.
	// FIXME: replace vector by run_set
	void locate_superblock_backups(block_manager<>::ptr bm,
				       std::vector<block_address> &locations) {
		// LVM_MAX_METADATA_BLOCKS is derived from LVM2's DM_THIN_MAX_METADATA_SIZE,
		// which is sligntly smaller than the kernel's limitation (i.e., MAX_METADATA_BLOCKS).
		// Note that QTS 4.1 uses 16GB metadata for 8KB metadata block size.
		std::vector<block_address> bounds;
		block_address max_nr_blocks = std::min(bm->get_nr_blocks(),
			static_cast<block_address>(persistent_data::sm_disk_detail::MAX_METADATA_BLOCKS));

		try {
			superblock_detail::superblock sb = read_superblock(bm, SUPERBLOCK_LOCATION);
			sm_disk_detail::sm_root v;
			sm_disk_detail::sm_root_disk const *d =
				reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.metadata_space_map_root_);
			sm_disk_detail::sm_root_traits::unpack(*d, v);
			max_nr_blocks = std::min(v.nr_blocks_, max_nr_blocks);
		} catch (std::exception &e) {
		}

		if (MD_BLOCK_SIZE == 8192) {
			// numbers of 8KB metadata block
			const block_address LVM_MAX_METADATA_BLOCKS[] = {
				2072576,
				16711680
			};
			if (max_nr_blocks > LVM_MAX_METADATA_BLOCKS[1]) {
				bounds.push_back(LVM_MAX_METADATA_BLOCKS[0]);
				bounds.push_back(LVM_MAX_METADATA_BLOCKS[1]);
				bounds.push_back(max_nr_blocks);
			} else if (max_nr_blocks > LVM_MAX_METADATA_BLOCKS[0]) {
				bounds.push_back(LVM_MAX_METADATA_BLOCKS[0]);
				bounds.push_back(max_nr_blocks);
			} else
				bounds.push_back(max_nr_blocks);
		} else {
			// numbers of 4KB metadata blocks
			const block_address LVM_MAX_METADATA_BLOCKS = 4145152;
			if (max_nr_blocks > LVM_MAX_METADATA_BLOCKS) {
				bounds.push_back(LVM_MAX_METADATA_BLOCKS);
				bounds.push_back(max_nr_blocks);
			} else
				bounds.push_back(max_nr_blocks);
		}

		block_address prev_end = 0;
		for (size_t i = 0; i < bounds.size(); ++i) {
			block_address begin = std::max(prev_end, bounds[i] - MAX_SUPERBLOCK_BACKUPS);
			block_address end = bounds[i];
			for (block_address b = begin; b < end; ++b)
				locations.push_back(b);
			prev_end = end;
		}
	}

	void wipe_superblock_backups(block_manager<>::ptr bm) {
		vector<block_address> sb_backup_locations;
		locate_superblock_backups(bm, sb_backup_locations);
		for (vector<block_address>::iterator it = sb_backup_locations.begin(); it != sb_backup_locations.end(); ++it)
			bm->write_lock_zero(*it);
		bm->flush();
	}

	void init_superblock_backups(block_manager<>::ptr bm, block_address location, unsigned int nr_backups) {
		for (block_address i = location; i < location + nr_backups; ++i) {
			block_manager<>::write_ref wr = bm->write_lock_zero(i, superblock_validator());
			superblock_disk *disk = reinterpret_cast<superblock_disk *>(wr.data());
			// The superblock backups must be initialized to all zero
			// except the csum, blocknr, and the magic fields.
			// We only need to update the magic field. The csum and blocknr
			// then will be filled by the superblock validator.
			disk->magic_ = to_disk<le64>(static_cast<uint64_t>(SUPERBLOCK_MAGIC));
		}
		bm->flush();
	}

	void migrate_superblock_backups(metadata* md,
					block_address origin_metadata_size,
					block_address new_metadata_size) {
		// reserve new superblock backups
		block_address extra_blocks = new_metadata_size - origin_metadata_size;
		block_address extra_begin = std::max(origin_metadata_size, new_metadata_size - MAX_SUPERBLOCK_BACKUPS);
		block_address extra_len = std::min(extra_blocks, static_cast<block_address>(MAX_SUPERBLOCK_BACKUPS));
		for (block_address b = extra_begin; b < extra_begin + extra_len; b++) {
			if (md->metadata_sm_->get_count(b)) {
				std::ostringstream ss;
				ss << "unexpected reference count at block " << b;
				throw std::runtime_error(ss.str());
			}
			md->metadata_sm_->set_count(b, 1);
		}

		// copy superblock backups to the new location
		block_manager<>::ptr bm = md->tm_->get_bm();
		block_address origin_begin = origin_metadata_size - MAX_SUPERBLOCK_BACKUPS;
		for (block_address i = 0; i < extra_len; ++i) {
			block_manager<>::read_ref rr = bm->read_lock(origin_begin + i, superblock_validator());
			block_manager<>::write_ref wr = bm->write_lock_zero(extra_begin + i, superblock_validator());
			::memcpy(wr.data(), rr.data(), MD_BLOCK_SIZE);
		}

		// discard unused superblock backups
		for (block_address b = origin_begin; b < origin_begin + extra_len; b++)
			md->metadata_sm_->set_count(b, 0);

		// adjust backup offset
		md->backup_offset_ = (md->backup_offset_ + extra_len - 1) % MAX_SUPERBLOCK_BACKUPS + 1;
	}

	void advance_backup_id(metadata *md) {
		md->backup_id_++;
		md->backup_offset_ = (md->backup_offset_ % MAX_SUPERBLOCK_BACKUPS) + 1;
	}

	void backup_superblock(metadata *md) {
		if (!(md->sb_.flags_ & THIN_FEATURE_SUPERBLOCK_BACKUP))
			return;

		block_address loc = md->metadata_sm_->get_nr_blocks() - md->backup_offset_;
		block_manager<>::write_ref wr = md->tm_->get_bm()->write_lock_zero(loc, superblock_validator());
		superblock_disk *disk = reinterpret_cast<superblock_disk *>(wr.data());
		superblock_traits::pack(md->sb_, *disk);
		disk->backup_id_ = to_disk<le64>(md->backup_id_);

		advance_backup_id(md);
	}

	// FIXME: replace vector by run_set
	void check_superblock_backups(block_manager<>::ptr bm,
				      vector<block_address> const &locations,
				      superblock_backup_profile &profile) {
		vector<block_address>::const_iterator it;
		for (it = locations.begin(); it != locations.end(); ++it) {
			try {
				superblock_detail::superblock sb = read_superblock(bm, *it);

				if (!(sb.flags_ & THIN_FEATURE_SUPERBLOCK_BACKUP))
					continue;
				++profile.backup_count_;
				if (sb.backup_id_ > profile.last_backup_id_ ||
				    profile.backup_count_ == 1) {
					profile.last_backup_id_ = sb.backup_id_;
					profile.last_blocknr_ = sb.blocknr_;
				}
			} catch (std::exception &e) {
			}
		}
	}

	void locate_backup_id(metadata *md) {
		using namespace sm_disk_detail;

		block_manager<>::ptr bm = md->tm_->get_bm();
		superblock const &sb = md->sb_;

		sm_disk_detail::sm_root v;
		sm_disk_detail::sm_root_disk const *d =
			reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sb.metadata_space_map_root_);
		sm_disk_detail::sm_root_traits::unpack(*d, v);

		vector<block_address> locations;
		for (block_address b = v.nr_blocks_ - MAX_SUPERBLOCK_BACKUPS;
		     b < v.nr_blocks_; b++)
			locations.push_back(b);

		superblock_backup_profile profile;
		check_superblock_backups(bm, locations, profile);

		if (profile.backup_count_) {
			md->backup_id_ = profile.last_backup_id_;
			md->backup_offset_ = v.nr_blocks_ - profile.last_blocknr_;
			advance_backup_id(md);
		} else {
			md->backup_id_ = 0;
			md->backup_offset_ = 1;
		}
	}
}

//----------------------------------------------------------------

metadata::metadata(std::string const &dev_path, open_type ot,
		   sector_t data_block_size, block_address nr_data_blocks,
		   unsigned int nr_superblock_backups,
		   sector_t tier_block_size,
		   std::vector<block_address> const& tier_data_blocks)
{
	switch (ot) {
	case OPEN:
		tm_ = open_tm(open_bm(dev_path, block_manager<>::READ_ONLY));
		sb_ = read_superblock(tm_->get_bm());

		if (sb_.version_ > 4)
			throw runtime_error("unknown metadata version");

		metadata_sm_ = open_metadata_sm(*tm_, &sb_.metadata_space_map_root_);
		tm_->set_sm(metadata_sm_);

		data_sm_ = open_disk_sm(*tm_, static_cast<void *>(&sb_.data_space_map_root_));

		details_ = device_tree_adapter::ptr(
			new device_tree_adapter(*tm_, sb_.device_details_root_, sb_.version_));

		mappings_top_level_ = dev_tree::ptr(
			new dev_tree(*tm_, sb_.data_mapping_root_,
				     mapping_tree_detail::mtree_ref_counter(tm_)));

		mappings_ = mapping_tree::ptr(
			new mapping_tree(*tm_, sb_.data_mapping_root_,
					 mapping_tree_detail::block_time_ref_counter(data_sm_)));

		if (sb_.clone_root_)
			clone_counts_ = clone_tree::ptr(
				new clone_tree(*tm_, sb_.clone_root_, uint32_traits::ref_counter()));

		if (sb_.tier_num_ && sb_.pool_mapping_root_) {
			vector<void const*> tier_data_sm_roots;
			tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier0_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier1_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier2_data_space_map_root_));
			tier_data_sm_roots.resize(sb_.tier_num_);
			tier_data_sm_ = open_multiple_disk_sm(*tm_, tier_data_sm_roots);
			tier_tree_ = tiering_tree::ptr(
				new tiering_tree(*tm_, sb_.pool_mapping_root_,
						 mapping_tree_detail::tier_block_ref_counter(tier_data_sm_)));
		}

		locate_backup_id(this);

		break;

	case CREATE:
	case CREATE_WITH_CLONE_DEVICE:
		tm_ = open_tm(open_bm(dev_path, block_manager<>::READ_WRITE));
		space_map::ptr core = tm_->get_sm();

		// FIXME: the superblock backup feature depends on metadata version
		wipe_superblock_backups(tm_->get_bm());
		for (block_address i = 0; i < nr_superblock_backups; ++i)
			core->inc(tm_->get_bm()->get_nr_blocks() - i - 1);
		init_superblock_backups(tm_->get_bm(), tm_->get_bm()->get_nr_blocks() - nr_superblock_backups, nr_superblock_backups);
		backup_id_ = 0;
		backup_offset_ = 1;

		metadata_sm_ = create_metadata_sm(*tm_, tm_->get_bm()->get_nr_blocks());
		copy_space_maps(metadata_sm_, core);
		tm_->set_sm(metadata_sm_);

		data_sm_ = create_disk_sm(*tm_, nr_data_blocks);
		details_ = device_tree_adapter::ptr(new device_tree_adapter(*tm_,
						    (ot == CREATE_WITH_CLONE_DEVICE) ? device_tree_adapter::TYPE_CLONE : device_tree_adapter::TYPE_BASIC));
		mappings_ = mapping_tree::ptr(new mapping_tree(*tm_,
							       mapping_tree_detail::block_time_ref_counter(data_sm_)));
		mappings_top_level_ = dev_tree::ptr(new dev_tree(*tm_, mappings_->get_root(),
								 mapping_tree_detail::mtree_ref_counter(tm_)));

		if (ot == CREATE_WITH_CLONE_DEVICE)
			clone_counts_ = clone_tree::ptr(new clone_tree(*tm_, uint32_traits::ref_counter()));

		::memset(&sb_, 0, sizeof(sb_));
		sb_.magic_ = SUPERBLOCK_MAGIC;
		sb_.version_ = 1;
		sb_.data_mapping_root_ = mappings_->get_root();
		sb_.device_details_root_ = details_->get_root();
		sb_.data_block_size_ = data_block_size;
		sb_.metadata_block_size_ = MD_BLOCK_SIZE >> BYTE_TO_SECTOR_SHIFT;
		sb_.metadata_nr_blocks_ = tm_->get_bm()->get_nr_blocks();
		sb_.clone_root_ = clone_counts_ ? clone_counts_->get_root() : 0;

		if (tier_data_blocks.size()) {
			sb_.tier_block_size_ = tier_block_size;
			sb_.tier_num_ = tier_data_blocks.size();
			tier_data_sm_ = create_multiple_disk_sm(*tm_, tier_data_blocks);
			tier_tree_ = tiering_tree::ptr(
				new tiering_tree(*tm_,
						 mapping_tree_detail::tier_block_ref_counter(tier_data_sm_)));
		}

		break;
	}
}

metadata::metadata(std::string const &dev_path, block_address metadata_snap)
{
	tm_ = open_tm(open_bm(dev_path, block_manager<>::READ_ONLY, !metadata_snap));
	sb_ = read_superblock(tm_->get_bm(), metadata_snap);

	// We don't open the metadata sm for a held root
	//metadata_sm_ = open_metadata_sm(tm_, &sb_.metadata_space_map_root_);
	//tm_->set_sm(metadata_sm_);

	data_sm_ = open_disk_sm(*tm_, static_cast<void *>(&sb_.data_space_map_root_));
	details_ = device_tree_adapter::ptr(new device_tree_adapter(*tm_, sb_.device_details_root_, sb_.version_));
	mappings_top_level_ = dev_tree::ptr(new dev_tree(*tm_, sb_.data_mapping_root_,
							 mapping_tree_detail::mtree_ref_counter(tm_)));
	mappings_ = mapping_tree::ptr(new mapping_tree(*tm_, sb_.data_mapping_root_,
						       mapping_tree_detail::block_time_ref_counter(data_sm_)));

	if (sb_.clone_root_)
		clone_counts_ = clone_tree::ptr(
			new clone_tree(*tm_, sb_.clone_root_, uint32_traits::ref_counter()));

	if (sb_.pool_mapping_root_ && sb_.tier_num_) {
		vector<void const*> tier_data_sm_roots;
		tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier0_data_space_map_root_));
		tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier1_data_space_map_root_));
		tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier2_data_space_map_root_));
		tier_data_sm_roots.resize(sb_.tier_num_);
		tier_data_sm_ = open_multiple_disk_sm(*tm_, tier_data_sm_roots);
		tier_tree_ = tiering_tree::ptr(
			new tiering_tree(*tm_, sb_.pool_mapping_root_,
					 mapping_tree_detail::tier_block_ref_counter(tier_data_sm_)));
	}

	locate_backup_id(this);
}

// FIXME: duplication
metadata::metadata(block_manager<>::ptr bm, open_type ot,
		   sector_t data_block_size,
		   block_address nr_data_blocks,
		   unsigned int nr_superblock_backups,
		   sector_t tier_block_size,
		   std::vector<block_address> const& tier_data_blocks)
{
	switch (ot) {
	case OPEN:
		tm_ = open_tm(bm);
		sb_ = read_superblock(tm_->get_bm());

		if (sb_.version_ > 4)
			throw runtime_error("unknown metadata version");

		metadata_sm_ = open_metadata_sm(*tm_, &sb_.metadata_space_map_root_);
		tm_->set_sm(metadata_sm_);

		data_sm_ = open_disk_sm(*tm_, static_cast<void *>(&sb_.data_space_map_root_));
		details_ = device_tree_adapter::ptr(new device_tree_adapter(*tm_, sb_.device_details_root_, sb_.version_));
		mappings_top_level_ = dev_tree::ptr(new dev_tree(*tm_, sb_.data_mapping_root_,
								 mapping_tree_detail::mtree_ref_counter(tm_)));
		mappings_ = mapping_tree::ptr(new mapping_tree(*tm_, sb_.data_mapping_root_,
							       mapping_tree_detail::block_time_ref_counter(data_sm_)));

		if (sb_.clone_root_)
			clone_counts_ = clone_tree::ptr(
				new clone_tree(*tm_, sb_.clone_root_, uint32_traits::ref_counter()));

		if (sb_.tier_num_ && sb_.pool_mapping_root_) {
			vector<void const*> tier_data_sm_roots;
			tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier0_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier1_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier2_data_space_map_root_));
			tier_data_sm_roots.resize(sb_.tier_num_);
			tier_data_sm_ = open_multiple_disk_sm(*tm_, tier_data_sm_roots);
			tier_tree_ = tiering_tree::ptr(
				new tiering_tree(*tm_, sb_.pool_mapping_root_,
						 mapping_tree_detail::tier_block_ref_counter(tier_data_sm_)));
		}

		locate_backup_id(this);

		break;

	case CREATE:
	case CREATE_WITH_CLONE_DEVICE:
		tm_ = open_tm(bm);
		space_map::ptr core = tm_->get_sm();

		// FIXME: the superblock backup feature depends on metadata version
		wipe_superblock_backups(tm_->get_bm());
		for (block_address i = 0; i < nr_superblock_backups; ++i)
			core->inc(tm_->get_bm()->get_nr_blocks() - i - 1);
		init_superblock_backups(tm_->get_bm(), tm_->get_bm()->get_nr_blocks() - nr_superblock_backups, nr_superblock_backups);
		backup_id_ = 0;
		backup_offset_ = 1;

		metadata_sm_ = create_metadata_sm(*tm_, tm_->get_bm()->get_nr_blocks());
		copy_space_maps(metadata_sm_, core);
		tm_->set_sm(metadata_sm_);

		data_sm_ = create_disk_sm(*tm_, nr_data_blocks);
		details_ = device_tree_adapter::ptr(new device_tree_adapter(*tm_,
						    (ot == CREATE_WITH_CLONE_DEVICE) ? device_tree_adapter::TYPE_CLONE : device_tree_adapter::TYPE_BASIC));
		mappings_ = mapping_tree::ptr(new mapping_tree(*tm_,
							       mapping_tree_detail::block_time_ref_counter(data_sm_)));
		mappings_top_level_ = dev_tree::ptr(new dev_tree(*tm_, mappings_->get_root(),
								 mapping_tree_detail::mtree_ref_counter(tm_)));

		if (ot == CREATE_WITH_CLONE_DEVICE)
			clone_counts_ = clone_tree::ptr(new clone_tree(*tm_, uint32_traits::ref_counter()));

		::memset(&sb_, 0, sizeof(sb_));
		sb_.magic_ = SUPERBLOCK_MAGIC;
		sb_.version_ = 1;
		sb_.data_mapping_root_ = mappings_->get_root();
		sb_.device_details_root_ = details_->get_root();
		sb_.data_block_size_ = data_block_size;
		sb_.metadata_block_size_ = MD_BLOCK_SIZE >> BYTE_TO_SECTOR_SHIFT;
		sb_.metadata_nr_blocks_ = tm_->get_bm()->get_nr_blocks();
		sb_.clone_root_ = clone_counts_ ? clone_counts_->get_root() : 0;

		if (tier_data_blocks.size()) {
			sb_.tier_block_size_ = tier_block_size;
			sb_.tier_num_ = tier_data_blocks.size();
			tier_data_sm_ = create_multiple_disk_sm(*tm_, tier_data_blocks);
			tier_tree_ = tiering_tree::ptr(
				new tiering_tree(*tm_,
						 mapping_tree_detail::tier_block_ref_counter(tier_data_sm_)));
		}

		break;
	}
}

metadata::metadata(block_manager<>::ptr bm, block_address metadata_snap)
{
	tm_ = open_tm(bm);
	sb_ = read_superblock(tm_->get_bm(), metadata_snap);

	// We don't open the metadata sm for a held root
	//metadata_sm_ = open_metadata_sm(tm_, &sb_.metadata_space_map_root_);
	//tm_->set_sm(metadata_sm_);

	data_sm_ = open_disk_sm(*tm_, static_cast<void *>(&sb_.data_space_map_root_));
	details_ = device_tree_adapter::ptr(new device_tree_adapter(*tm_, sb_.device_details_root_, sb_.version_));
	mappings_top_level_ = dev_tree::ptr(new dev_tree(*tm_, sb_.data_mapping_root_,
					    mapping_tree_detail::mtree_ref_counter(tm_)));
	mappings_ = mapping_tree::ptr(new mapping_tree(*tm_, sb_.data_mapping_root_,
				      mapping_tree_detail::block_time_ref_counter(data_sm_)));

	if (sb_.clone_root_)
		clone_counts_ = clone_tree::ptr(
					new clone_tree(*tm_, sb_.clone_root_, uint32_traits::ref_counter()));

	if (sb_.pool_mapping_root_ && sb_.tier_num_) {
		vector<void const*> tier_data_sm_roots;
		tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier0_data_space_map_root_));
		tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier1_data_space_map_root_));
		tier_data_sm_roots.push_back(static_cast<void const*>(sb_.tier2_data_space_map_root_));
		tier_data_sm_roots.resize(sb_.tier_num_);
		tier_data_sm_ = open_multiple_disk_sm(*tm_, tier_data_sm_roots);
		tier_tree_ = tiering_tree::ptr(
				     new tiering_tree(*tm_, sb_.pool_mapping_root_,
						      mapping_tree_detail::tier_block_ref_counter(tier_data_sm_)));
	}

	locate_backup_id(this);
}

// The backup_id_ is not reseted to zero for the following reasons:
// * thin_check won't be able to find the last backup,
//   if extra_blocks < MAX_SUPERBLOCK_BACKUPS
// * error tracking: for users to trace previous transactions
void
metadata::extend(block_address extra_blocks)
{
	if (!extra_blocks)
		return;

	block_address origin_size = metadata_sm_->get_nr_blocks();
	metadata_sm_->extend(extra_blocks);
	block_address new_size = metadata_sm_->get_nr_blocks();
	migrate_superblock_backups(this, origin_size, new_size);
}

void
metadata::commit()
{
	sb_.data_mapping_root_ = mappings_->get_root();
	sb_.device_details_root_ = details_->get_root();
	sb_.clone_root_ = clone_counts_ ? clone_counts_->get_root() : 0;
	sb_.pool_mapping_root_ = sb_.tier_num_ ? tier_tree_->get_root() : 0;

	data_sm_->commit();
	data_sm_->copy_root(&sb_.data_space_map_root_, sizeof(sb_.data_space_map_root_));

	void* tier_data_sm_roots[] = {
		static_cast<void *>(sb_.tier0_data_space_map_root_),
		static_cast<void *>(sb_.tier1_data_space_map_root_),
		static_cast<void *>(sb_.tier2_data_space_map_root_)
	};
	for (uint32_t i = 0; i < sb_.tier_num_; ++i) {
		tier_data_sm_[i]->commit();
		tier_data_sm_[i]->copy_root(tier_data_sm_roots[i], SPACE_MAP_ROOT_SIZE);
	}

	metadata_sm_->commit();
	metadata_sm_->copy_root(&sb_.metadata_space_map_root_, sizeof(sb_.metadata_space_map_root_));

	backup_superblock(this);

	write_ref superblock = tm_->get_bm()->superblock_zero(SUPERBLOCK_LOCATION, superblock_validator());
        superblock_disk *disk = reinterpret_cast<superblock_disk *>(superblock.data());
	superblock_traits::pack(sb_, *disk);
}

//----------------------------------------------------------------

/*
 * Returns the size of the tiering data device after conversion, in tier blocks
 */
uint64_t thin_provisioning::estimate_tier_size(uint64_t nr_data_blocks, uint32_t data_block_size, uint32_t tier_block_size) {
	uint64_t tier_blocks_for_data = nr_data_blocks * data_block_size / tier_block_size;
	uint64_t estimated_tier_size = 0;
	if (tier_blocks_for_data) {
		uint64_t max_val = std::max<uint64_t>(tier_blocks_for_data + MIN_TIER_SWAPS,
						      tier_blocks_for_data * 100 / 99);
		estimated_tier_size = std::min<uint64_t>(tier_blocks_for_data + MAX_TIER_SWAPS, max_val);
	}

	// cross validate
	uint64_t r;
	while (estimated_tier_size - tier_blocks_for_data != (r = calculate_tier_swaps(estimated_tier_size))) {
		if (estimated_tier_size - tier_blocks_for_data < r)
			++estimated_tier_size;
		else
			--estimated_tier_size;
	}

	return estimated_tier_size;
}

uint64_t thin_provisioning::calculate_tier_swaps(uint64_t nr_tier_blocks) {
	if (nr_tier_blocks) {
		uint64_t max_val = std::max<uint64_t>(MIN_TIER_SWAPS,
						      base::div_down<uint64_t>(nr_tier_blocks, 100));
		return std::min<uint64_t>(MAX_TIER_SWAPS, max_val);
	}
	return 0;
}

void thin_provisioning::find_superblock_backups(block_manager<>::ptr bm,
		superblock_backup_profile &profile) {
	vector<block_address> locations;
	locate_superblock_backups(bm, locations);
	profile.reset();
	check_superblock_backups(bm, locations, profile);
}

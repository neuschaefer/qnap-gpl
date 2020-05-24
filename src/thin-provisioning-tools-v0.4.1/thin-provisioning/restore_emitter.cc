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

#include "persistent-data/math_utils.h"
#include "thin-provisioning/restore_emitter.h"
#include "thin-provisioning/superblock.h"
#include "thin-provisioning/tiering_utils.h"

#include <limits>

using namespace std;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	using namespace superblock_detail;

	class restorer : public emitter {
	public:
		// FIXME: ugly. restorer should not depend on restorer_params to do format conversion
		// To keep the simplicy of restorer, it could be better to
		// spin-off the format conversion logic into a stand-alone visitor.
		// The visitor might look like mapping_tree_emitter.
		restorer(metadata::ptr md, boost::optional<restorer_params> params)
			: md_(md),
			  in_superblock_(false),
			  nr_data_blocks_(),
			  empty_mapping_(new_mapping_tree()),
			  data_blocks_multiplier_(1),
			  increase_metadata_time_(false) {
			if (params)
				params_ = *params;
		}

		virtual ~restorer() {
			// FIXME: replace with a call to empty_mapping_->destroy()
			md_->metadata_sm_->dec(empty_mapping_->get_root());

			// The previous step updates the metadata space map, so we should do commit again
			// FIXME: This approach introduces implicit commits
			// https://github.com/jthornber/thin-provisioning-tools/issues/73
			md_->commit();
		}

		virtual void begin_superblock(std::string const &uuid,
					      uint64_t time,
					      uint64_t trans_id,
					      boost::optional<uint32_t> flags,
					      boost::optional<uint32_t> version,
					      uint32_t data_block_size,
					      uint64_t nr_data_blocks,
					      boost::optional<uint64_t> metadata_snap,
					      boost::optional<uint64_t> reserve_block_count,
					      boost::optional<uint32_t> tier_block_size,
					      boost::optional<vector<uint64_t> > tier_data_blocks) {
			if (params_.data_block_size_) {
				if (data_block_size < params_.data_block_size_) {
					ostringstream msg;
					msg << "source block size " << data_block_size
					    << " is less than target block size " << params_.data_block_size_;
					throw runtime_error(msg.str());
				}
				if (data_block_size % params_.data_block_size_) {
					ostringstream msg;
					msg << "source block size " << data_block_size
					    << " cannot be divided by target block size " << params_.data_block_size_;
					throw runtime_error(msg.str());
				}
				data_blocks_multiplier_ = data_block_size / params_.data_block_size_;
			}

			in_superblock_ = true;
			nr_data_blocks_ = nr_data_blocks;
			superblock &sb = md_->sb_;
			memset(&sb.uuid_, 0, sizeof(sb.uuid_));
			memcpy(&sb.uuid_, uuid.c_str(), std::min(sizeof(sb.uuid_), uuid.length()));
			sb.time_ = time;
			sb.trans_id_ = trans_id;
			sb.flags_ = flags ? *flags : 0;

			if (params_.enable_superblock_backup_)
				sb.flags_ |= THIN_FEATURE_SUPERBLOCK_BACKUP;

			if (params_.enable_fast_block_clone_) {
				sb.flags_ |= THIN_FEATURE_FAST_BLOCK_CLONE;

				// For QNAP, version >= 3 means that the superblock::time_ was started from 1,
				// and there's no need to increase the time counters.
				if (!version || (version && *version < 3)) {
					increase_metadata_time_ = true;
					sb.time_ += 1;
				}
			}

			sb.version_ = params_.version_ ? params_.version_ : (version ? *version : 1);
			sb.data_block_size_ = params_.data_block_size_ ? params_.data_block_size_ : data_block_size;
			sb.metadata_snap_ = metadata_snap ? *metadata_snap : 0;
			sb.reserve_block_count_ = 0; // reserve_block_count_ will be recalculated
			md_->data_sm_->extend(nr_data_blocks * data_blocks_multiplier_);

			// case1: the source metadata is tiering enabled
			if (tier_block_size && tier_data_blocks && tier_data_blocks->size()) {
				// TODO: support conversion between different tier levels
				if (sb.tier_num_ != tier_data_blocks->size())
					throw runtime_error("invalid number of tiers");

				// TODO: support conversion between different tier block size, using params_.tier_block_size_
				sb.tier_block_size_ = *tier_block_size;

				for (size_t i = 0; i < tier_data_blocks->size(); ++i)
					md_->tier_data_sm_[i]->extend((*tier_data_blocks)[i]);

				// create at this moment to check for errors at early stage
				if (params_.disable_tiering_bypass_)
					tiering_mapping_creator_ = tiering_mapping_creator::ptr(new tiering_mapping_creator(*this, *md_));

			// case2: convert non-tiering-thin-pool to tiering-thin-pool
			} else if (params_.nr_tiers_) {
				sb.tier_block_size_ = params_.tier_block_size_ ? params_.tier_block_size_ : sb.tier_block_size_;

				if ((md_->data_sm_->get_nr_blocks() * sb.data_block_size_) % sb.tier_block_size_ != 0)
					throw runtime_error("pool size cannot be divided by tier block size");

				uint64_t total_tier_blocks = estimate_tier_size(md_->data_sm_->get_nr_blocks(),
										sb.data_block_size_,
										sb.tier_block_size_);
				// place data in the last tier
				md_->tier_data_sm_[sb.tier_num_ - 1]->extend(total_tier_blocks);
			}
		}

		virtual void end_superblock() {
			if (!in_superblock_)
				throw runtime_error("missing superblock");

			if (params_.disable_tiering_bypass_) {
				tiering_mapping_creator_->check_space_map_compatibility();
				md_->data_sm_->no_lookaside_iterate(*tiering_mapping_creator_);
			}

			md_->commit();
			in_superblock_ = false;
		}

		virtual void begin_device(uint32_t dev,
					  uint64_t mapped_blocks,
					  uint64_t trans_id,
					  uint64_t creation_time,
					  uint64_t snap_time,
					  boost::optional<uint32_t> clone_time,
					  boost::optional<uint64_t> scaned_index,
					  uint64_t snap_origin) {
			if (!in_superblock_)
				throw runtime_error("missing superblock");

			if (device_exists(dev))
				throw std::runtime_error("Device already exists");

			// Store the entry of the details tree
			device_tree_detail::device_details &details = current_device_details_;
			details.mapped_blocks_ = 0;
			details.transaction_id_ = trans_id;
			details.creation_time_ = (uint32_t)creation_time;
			details.snapshotted_time_ = (uint32_t)snap_time;
			details.cloned_time_ = boost::none;
			details.scaned_index_ = boost::none;
			details.snap_origin_ = snap_origin;

			if (clone_time)
				details.cloned_time_ = *clone_time;
			else if (params_.enable_fast_block_clone_) {
				details.cloned_time_ = 1;
				if (increase_metadata_time_) {
					details.creation_time_ += 1;
					details.snapshotted_time_ += 1;
				}
			}

			// the device's reserved blocks will be recalculate during mapping insertion
			if (scaned_index || params_.enable_fast_block_clone_)
				details.scaned_index_ = std::numeric_limits<uint64_t>::max();

			current_mapping_ = empty_mapping_->clone();
			current_device_ = boost::optional<uint32_t>(dev);
		}

		virtual void end_device() {
			uint64_t key[1] = {*current_device_};

			// Add entry to the details tree
			md_->details_->insert(key, current_device_details_);

			md_->mappings_top_level_->insert(key, current_mapping_->get_root());
			md_->mappings_->set_root(md_->mappings_top_level_->get_root()); // FIXME: ugly

			current_device_ = boost::optional<uint32_t>();
		}

		virtual void begin_named_mapping(std::string const &name) {
			throw runtime_error("not implemented");
		}

		virtual void end_named_mapping() {
			throw runtime_error("not implemented");
		}

		virtual void identifier(std::string const &name) {
			throw runtime_error("not implemented");
		}

		virtual void range_map(uint64_t origin_begin, uint64_t data_begin, uint32_t time, uint64_t len, uint32_t fastzero) {
			for (uint64_t i = 0; i < len; i++)
				single_map(origin_begin++, data_begin++, time, fastzero);
		}

		virtual void single_map(uint64_t origin_block, uint64_t data_block, uint32_t time, uint32_t fastzero) {
			if (!current_device_)
				throw runtime_error("not in device");

			if (data_block >= nr_data_blocks_) {
				std::ostringstream out;
				out << "mapping beyond end of data device (" << data_block
				    << " >= " << nr_data_blocks_ << ")";
				throw std::runtime_error(out.str());
			}

			for (uint32_t i = 0; i < data_blocks_multiplier_; ++i) {
				uint64_t key[1] = {origin_block * data_blocks_multiplier_ + i};
				mapping_tree_detail::block_time bt;
				bt.block_ = data_block * data_blocks_multiplier_ + i;
				bt.time_ = time;
				bt.zeroed_ = fastzero;
				if (increase_metadata_time_)
					bt.time_ += 1;
				current_device_details_.mapped_blocks_ +=
					static_cast<uint64_t>(current_mapping_->insert(key, bt));
				md_->data_sm_->inc(bt.block_);
			}

			if (current_device_details_.snap_origin_ == std::numeric_limits<uint64_t>::max() &&
			    current_device_details_.scaned_index_) {
				uint64_t key[1] = {data_block};
				boost::optional<uint32_t> mvalue = md_->clone_counts_->lookup(key);
				uint32_t count = mvalue ? *mvalue : 0;
				if (count > 0 && md_->data_sm_->get_nr_free() <= md_->sb_.reserve_block_count_) {
					ostringstream msg;
					msg << "insufficient free space for reserved blocks"
					    << ": device " << *current_device_
					    << ", origin_block " << origin_block
					    << ", data_block " << data_block;
					throw runtime_error(msg.str());
				}
				md_->clone_counts_->insert(key, count + 1);
				md_->sb_.reserve_block_count_ += (count > 0) ? 1 : 0;
			}
		}

		virtual void begin_tier() {
			if (!in_superblock_)
				throw runtime_error("missing superblock");
		}

		virtual void end_tier() {
		}

		virtual void range_tier(uint64_t origin_begin, uint32_t tier, uint64_t data_begin, uint64_t len) {
			for (uint64_t i = 0; i < len; i++)
				single_tier(origin_begin++, tier, data_begin++);
		}

		virtual void single_tier(uint64_t origin_block, uint32_t tier, uint64_t data_block) {
			uint64_t key[1] = {origin_block};
			mapping_tree_detail::tier_block tb;
			tb.tier_ = tier;
			tb.block_ = data_block;
			tb.reserved_ = 0;
			md_->tier_tree_->insert(key, tb);
			md_->tier_data_sm_[tier]->inc(data_block);
		}

		void begin_uint32(std::string const &name) {
			throw runtime_error("not implemented");
		}

		void end_uint32(std::string const &name) {
			throw runtime_error("not implemented");
		}

		void range_uint32(uint64_t key_begin, uint32_t value, uint64_t len) {
			throw runtime_error("not implemented");
		}

		void single_uint32(uint64_t key, uint32_t value) {
			throw runtime_error("not implemented");
		}

	private:
		single_mapping_tree::ptr new_mapping_tree() {
			return single_mapping_tree::ptr(
				new single_mapping_tree(*md_->tm_,
							mapping_tree_detail::block_time_ref_counter(md_->data_sm_)));
		}

		bool device_exists(thin_dev_t dev) const {
			uint64_t key[1] = {dev};
			device_tree::maybe_value v = md_->details_->lookup(key);
			return v;
		}

		metadata::ptr md_;
		bool in_superblock_;
		block_address nr_data_blocks_; // data block count of the input pool
		boost::optional<uint32_t> current_device_;
		device_tree_detail::device_details current_device_details_;
		single_mapping_tree::ptr current_mapping_;
		single_mapping_tree::ptr empty_mapping_;
		restorer_params params_;
		uint32_t data_blocks_multiplier_;
		bool increase_metadata_time_;
		tiering_mapping_creator::ptr tiering_mapping_creator_;
	};
}

//----------------------------------------------------------------

emitter::ptr
thin_provisioning::create_restore_emitter(metadata::ptr md, boost::optional<restorer_params> params)
{
	return emitter::ptr(new restorer(md, params));
}

//----------------------------------------------------------------

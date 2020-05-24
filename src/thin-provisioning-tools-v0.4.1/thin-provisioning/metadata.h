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

#ifndef METADATA_LL_H
#define METADATA_LL_H

#include "base/endian_utils.h"

#include "persistent-data/block.h"
#include "persistent-data/data-structures/btree.h"
#include "persistent-data/space-maps/disk.h"
#include "persistent-data/transaction_manager.h"

#include "thin-provisioning/device_tree.h"
#include "thin-provisioning/mapping_tree.h"
#include "thin-provisioning/superblock.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	// FIXME: don't use namespaces in a header
	using namespace base;
	using namespace persistent_data;

	typedef uint64_t sector_t;
	typedef uint32_t thin_dev_t;

	// Hardcode in LVM2, in 512-byte sectors
	uint64_t const DM_THIN_MAX_METADATA_SIZE[] = {
		(static_cast<uint64_t>(255) * (1 << 14) * (4096 / (1 << 9)) - 256 * 1024),
		(static_cast<uint64_t>(511) * (1 << 15) * (8192 / (1 << 9)) - 512 * 1024)
	};

	// Defined in dm-thin kernel driver
	uint32_t const THIN_FEATURE_SUPERBLOCK_BACKUP = 1U << 31;
	uint32_t const THIN_FEATURE_FAST_BLOCK_CLONE = 1U << 30;
	uint32_t const MAX_SUPERBLOCK_BACKUPS = 128;
	uint32_t const DEFAULT_TIER_BLOCK_SIZE = 8192; // in 512-byte sectors
	uint32_t const MAX_TIER_SWAPS = 400;
	uint32_t const MIN_TIER_SWAPS = 1;

	//------------------------------------------------

	// FIXME: should these be in a sub-namespace?
	typedef persistent_data::transaction_manager::ptr tm_ptr;

	enum space_map_id {
		METADATA_SPACE_MAP = 1,
		DATA_SPACE_MAP,
		TIER0_DATA_SPACE_MAP,
		TIER1_DATA_SPACE_MAP,
		TIER2_DATA_SPACE_MAP,
	};

	struct superblock_backup_profile {
		superblock_backup_profile()
			: backup_count_(0),
			  last_backup_id_(0),
			  last_blocknr_(0) {}

		void reset() {
			backup_count_ = 0;
			last_backup_id_ = 0;
			last_blocknr_ = 0;
		}

		uint32_t backup_count_;
		uint64_t last_backup_id_;
		uint64_t last_blocknr_;
	};

	// The tools require different interfaces onto the metadata than
	// the in kernel driver.  This class gives access to the low-level
	// implementation of metadata.  Implement more specific interfaces
	// on top of this.
	struct metadata {
		enum open_type {
			CREATE,
			CREATE_WITH_CLONE_DEVICE,
			OPEN
		};

		typedef block_manager<>::read_ref read_ref;
		typedef block_manager<>::write_ref write_ref;
		typedef boost::shared_ptr<metadata> ptr;


		// Deprecated: it would be better if we passed in an already
		// constructed block_manager.
		metadata(std::string const &dev_path, open_type ot,
			 sector_t data_block_size = 128, // Only used if CREATE
			 block_address nr_data_blocks = 0, // Only used if CREATE
			 unsigned int nr_superblock_backups = 0, // Only used if CREATE
			 sector_t tier_block_size = DEFAULT_TIER_BLOCK_SIZE, // Only used if CREATE
			 std::vector<block_address> const& tier_data_blocks = std::vector<block_address>()); // Only used if CREATE

		metadata(std::string const &dev_path,
			 block_address metadata_snap = 0);

		// ... use these instead ...
		metadata(block_manager<>::ptr bm, open_type ot,
			 sector_t data_block_size = 128,
			 block_address nr_data_blocks = 0, // Only used if CREATE
			 unsigned int nr_superblock_backups = 0, // Only used if CREATE
			 sector_t tier_block_size = DEFAULT_TIER_BLOCK_SIZE, // Only used if CREATE
			 std::vector<block_address> const& tier_data_blocks = std::vector<block_address>()); // Only used if CREATE
		metadata(block_manager<>::ptr bm, block_address metadata_snap);

		void extend(block_address extra_blocks);
		void commit();


		tm_ptr tm_;
		superblock_detail::superblock sb_;
		uint64_t backup_id_; // backup id for the NEW tranasaction
		block_address backup_offset_; // superblock backup location for the NEW transaction

		checked_space_map::ptr metadata_sm_;
		checked_space_map::ptr data_sm_;
		std::vector<checked_space_map::ptr> tier_data_sm_; // FIXME: use adapter?
		device_tree_adapter::ptr details_;
		dev_tree::ptr mappings_top_level_;
		mapping_tree::ptr mappings_;
		clone_tree::ptr clone_counts_;
		tiering_tree::ptr tier_tree_;
	};

	void find_superblock_backups(block_manager<>::ptr bm, superblock_backup_profile &profile);
	uint64_t estimate_tier_size(uint64_t nr_data_blocks, uint32_t data_block_size, uint32_t tier_block_size);
	uint64_t calculate_tier_swaps(uint64_t nr_tier_blocks);
}

//----------------------------------------------------------------

#endif

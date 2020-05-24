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

#ifndef RESTORE_EMITTER_H
#define RESTORE_EMITTER_H

#include "emitter.h"
#include "metadata.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	struct restorer_params {
		restorer_params()
			: version_(0),
			  enable_superblock_backup_(false),
			  enable_fast_block_clone_(false),
			  data_block_size_(0),
			  nr_tiers_(0),
			  tier_block_size_(0),
			  disable_tiering_bypass_(false) {
		}

		// Overwrite superblock::version_
		// 0: keep the source version; Non-zero: overwrite the version
		uint32_t version_;

		// Append THIN_FEATURE_SUPERBLOCK_BACKUP to superblock::flag_,
		// and preserve the last 128 metadata blocks.
		// false: keep the source setting; true: enable this feature
		bool enable_superblock_backup_;

		// 1. Append THIN_FEATURE_FAST_BLOCK_CLONE to superblock::flags_
		// 2. Increase metadata time counters by one, if necessary:
		//    superblock::time_, details::creation_time_,
		//    details::snapshotted_time_, details::cloned_time_,
		//    and the block-time reference counts.
		// 3. Create the superblock::clone_root_ btree, and expand
		//    the device details structure
		// false: keep the source setting; true: enable this feature
		bool enable_fast_block_clone_;

		// Change the data block size. The restorer will recalculate
		// data mappings and update related fields.
		// 0: keep the source value; Non-zero: change this value
		uint32_t data_block_size_; // in 512-byte sectors

		// Create the superblock::pool_mapping_root_ btree
		// and tiering space maps if they were not exist.
		// zero: keep the source setting
		// non-zero: convert to tiering-thin-pool with <N> tiers
		uint32_t nr_tiers_;

		// Change the tiering block size
		// 0: keep the source setting or use the default value
		// Non-zero: change this value
		uint32_t tier_block_size_; // in 512-byte sectors

		// Insert tiering mappings to disable tiering bypass
		// i.e., set the pool table parameter to "bypass_off"
		bool disable_tiering_bypass_;
	};

	emitter::ptr create_restore_emitter(metadata::ptr md, boost::optional<restorer_params> params);
}

//----------------------------------------------------------------

#endif

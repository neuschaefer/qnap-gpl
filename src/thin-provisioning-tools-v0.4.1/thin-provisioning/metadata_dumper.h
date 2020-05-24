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

#ifndef METADATA_DUMPER_H
#define METADATA_DUMPER_H

#include "emitter.h"
#include "metadata.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	enum repair_level {
		NO_REPAIR,
		IGNORE_MAPPINGS_AND_DETAILS_DAMAGE,
		DUMP_ORPHAN_DEVICES
	};

	enum verbosity_flags {
		DUMP_DATA_MAPPINGS = (1 << 0),
		DUMP_TIERING       = (1 << 1),
		DUMP_CLONE_COUNTS  = (1 << 2),
		DUMP_ORIGINS_ONLY  = (1 << 3)
	};

	// Set the @repair flag if your metadata is corrupt, and you'd like
	// the dumper to do it's best to recover info.  If not set, any
	// corruption encountered will cause an exception to be thrown.
	void metadata_dump(metadata::ptr md, emitter::ptr e, repair_level rl, uint32_t verbosity);
	void metadata_dump(metadata::ptr md, emitter::ptr e, repair_level rl, uint32_t verbosity,
			   std::vector<uint64_t> const& dev_id);
	void metadata_dump_subtree(metadata::ptr md, emitter::ptr e, bool repair, uint64_t subtree_root);
	void metadata_dump_clone_tree(metadata::ptr md, emitter::ptr e, bool repair, clone_tree &tree);
	void metadata_dump_tiering_tree(metadata::ptr md, emitter::ptr e, bool repair, uint64_t node);
	void metadata_dump_tiering_tree(metadata::ptr md, emitter::ptr e, bool repair, tiering_tree &tree);
	void metadata_dump_superblock_begin(metadata::ptr md, emitter::ptr e);
	void metadata_dump_superblock_end(metadata::ptr md, emitter::ptr e);
	void metadata_dump_data_space_map(metadata::ptr md, emitter::ptr e);
	void metadata_dump_space_map(metadata::ptr md, emitter::ptr e);
}

//----------------------------------------------------------------

#endif

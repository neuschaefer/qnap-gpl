#ifndef METADATA_COUNTER_H
#define METADATA_COUNTER_H

#include "thin-provisioning/metadata.h"
#include "persistent-data/data-structures/btree_counter.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	void count_metadata(transaction_manager::ptr tm,
			    superblock_detail::superblock &sb,
			    block_counter &bc,
			    space_map_detail::damage_visitor &dv,
			    bool skip_metadata_snap = false);
	void count_metadata_partial(transaction_manager::ptr tm,
				    superblock_detail::superblock const &sb,
				    block_counter &bc,
				    space_map_detail::damage_visitor &dv,
				    bool skip_metadata_snap = false);
}

//----------------------------------------------------------------

#endif

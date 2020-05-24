#ifndef MAPPING_TREE_CHECKER_H
#define MAPPING_TREE_CHECKER_H

#include "persistent-data/transaction_manager.h"
#include "thin-provisioning/device_tree.h"
#include "thin-provisioning/mapping_tree.h"
#include "thin-provisioning/superblock.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	enum mapping_tree_checker_flags {
		CHECK_MAPPED_BLOCKS = (1 << 0),
		CHECK_HIGHEST_MAPPED_BLOCK = (1 << 1),
	};

	void check_mapping_tree(transaction_manager::ptr tm,
				superblock_detail::superblock const &sb,
				std::map<uint64_t, device_tree_detail::device_details> const &details,
				mapping_tree_detail::damage_visitor &dv,
				mapping_tree_checker_flags f);
}

//----------------------------------------------------------------

#endif


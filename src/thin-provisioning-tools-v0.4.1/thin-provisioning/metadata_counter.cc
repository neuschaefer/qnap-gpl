#include "thin-provisioning/metadata_counter.h"
#include "persistent-data/space-maps/core.h"
#include "persistent-data/space-maps/disk_structures.h"

using namespace persistent_data;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	void count_trees(transaction_manager::ptr tm,
			 superblock_detail::superblock const &sb,
			 block_counter &bc) {

		// Count the device tree
		{
			noop_value_counter<device_tree_detail::device_details> vc;
			device_tree_adapter dtree(*tm, sb.device_details_root_, sb.version_);
			if (dtree.get_device_tree_cl())
				count_btree_blocks(*dtree.get_device_tree_cl(), bc, vc);
			else
				count_btree_blocks(*dtree.get_device_tree(), bc, vc);
		}

		// Count the mapping tree
		{
			noop_value_counter<mapping_tree_detail::block_time> vc;
			mapping_tree mtree(*tm, sb.data_mapping_root_,
					   mapping_tree_detail::block_traits::ref_counter(space_map::ptr()));
			count_btree_blocks(mtree, bc, vc);
		}

		// Count the clone tree
		if (sb.clone_root_) {
			noop_value_counter<uint32_t> vc;
			clone_tree ctree(*tm, sb.clone_root_, uint32_traits::ref_counter());
			count_btree_blocks(ctree, bc, vc);
		}

		// Count the tiering tree
		if (sb.tier_num_ && sb.pool_mapping_root_) {
			noop_value_counter<mapping_tree_detail::tier_block> vc;

			std::vector<void const *> tier_data_sm_roots;
			tier_data_sm_roots.push_back(static_cast<void const *>(sb.tier0_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const *>(sb.tier1_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const *>(sb.tier2_data_space_map_root_));
			tier_data_sm_roots.resize(sb.tier_num_);
			std::vector<checked_space_map::ptr> tier_data_sm_ = open_multiple_disk_sm(*tm, tier_data_sm_roots);
			tiering_tree tier_tree(*tm, sb.pool_mapping_root_,
					       mapping_tree_detail::tier_block_traits::ref_counter(tier_data_sm_));

			count_btree_blocks(tier_tree, bc, vc);
		}
	}

	void count_space_maps(transaction_manager::ptr tm,
			      superblock_detail::superblock const &sb,
			      block_counter &bc,
			      space_map_detail::damage_visitor &dv) {
		// Count the metadata space map (no-throw)
		try {
			persistent_space_map::ptr metadata_sm =
				open_metadata_sm(*tm, static_cast<void const *>(&sb.metadata_space_map_root_));
			dv.set_id(METADATA_SPACE_MAP);
			metadata_sm->count_metadata(bc, dv);
		} catch (std::exception &e) {
			dv.visit(space_map_detail::missing_counts(e.what(), METADATA_SPACE_MAP,
								  base::run<block_address>(0)));
		}

		// Count the data space map (no-throw)
		{
			persistent_space_map::ptr data_sm =
				open_disk_sm(*tm, static_cast<void const *>(&sb.data_space_map_root_));
			dv.set_id(DATA_SPACE_MAP);
			data_sm->count_metadata(bc, dv);
		}

		// Count the tiering data space maps
		if (sb.tier_num_) {
			std::vector<void const *> tier_data_sm_roots;
			tier_data_sm_roots.push_back(static_cast<void const *>(sb.tier0_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const *>(sb.tier1_data_space_map_root_));
			tier_data_sm_roots.push_back(static_cast<void const *>(sb.tier2_data_space_map_root_));

			for (uint32_t i = 0; i < sb.tier_num_; ++i) {
				persistent_space_map::ptr tier_data_sm =
					open_disk_sm(*tm, tier_data_sm_roots[i]);
				dv.set_id(TIER0_DATA_SPACE_MAP + i);
				tier_data_sm->count_metadata(bc, dv);
			}
		}
	}

	void count_tree_roots(transaction_manager::ptr tm,
			      superblock_detail::superblock const &sb,
			      block_counter &bc) {
		bc.inc(sb.device_details_root_);
		bc.inc(sb.data_mapping_root_);
		if (sb.clone_root_)
			bc.inc(sb.clone_root_);
		if (sb.tier_num_ && sb.pool_mapping_root_)
			bc.inc(sb.pool_mapping_root_);
	}

	void count_space_map_roots(transaction_manager::ptr tm,
				   superblock_detail::superblock const &sb,
				   block_counter &bc,
				   space_map_detail::damage_visitor &dv) {
		// Count the metadata space map (no-throw)
		try {
			dv.set_id(METADATA_SPACE_MAP);
			persistent_space_map::ptr metadata_sm =
				open_metadata_sm(*tm, static_cast<void const*>(&sb.metadata_space_map_root_));
			metadata_sm->count_metadata(bc, dv);
		} catch (std::exception &e) {
			dv.visit(space_map_detail::missing_counts(e.what(), METADATA_SPACE_MAP,
								  base::run<block_address>(0)));
		}

		// Count the data space map root
		{
			sm_disk_detail::sm_root_disk d;
			sm_disk_detail::sm_root v;
			::memcpy(&d, sb.data_space_map_root_, sizeof(d));
			sm_disk_detail::sm_root_traits::unpack(d, v);
			bc.inc(v.bitmap_root_);
			bc.inc(v.ref_count_root_);
		}

		// Count the tiering data space map roots
		if (sb.tier_num_) {
			sm_disk_detail::sm_root_disk d;
			sm_disk_detail::sm_root v;

			::memcpy(&d, sb.tier0_data_space_map_root_, sizeof(d));
			sm_disk_detail::sm_root_traits::unpack(d, v);
			bc.inc(v.bitmap_root_);
			bc.inc(v.ref_count_root_);

			::memcpy(&d, sb.tier1_data_space_map_root_, sizeof(d));
			sm_disk_detail::sm_root_traits::unpack(d, v);
			bc.inc(v.bitmap_root_);
			bc.inc(v.ref_count_root_);

			::memcpy(&d, sb.tier2_data_space_map_root_, sizeof(d));
			sm_disk_detail::sm_root_traits::unpack(d, v);
			bc.inc(v.bitmap_root_);
			bc.inc(v.ref_count_root_);
		}
	}

	void count_superblock_backups(superblock_detail::superblock const &sb,
				      block_counter &bc) {
		if (sb.flags_ & THIN_FEATURE_SUPERBLOCK_BACKUP) {
			sm_disk_detail::sm_root_disk const *d =
				reinterpret_cast<sm_disk_detail::sm_root_disk const *>(&sb.metadata_space_map_root_);
			sm_disk_detail::sm_root v;
			sm_disk_detail::sm_root_traits::unpack(*d, v);

			for (size_t i = 0; i < MAX_SUPERBLOCK_BACKUPS; ++i) {
				bc.inc(v.nr_blocks_ - i - 1);
			}
		}
	}
}

//----------------------------------------------------------------

void thin_provisioning::count_metadata(transaction_manager::ptr tm,
				       superblock_detail::superblock &sb,
				       block_counter &bc,
				       space_map_detail::damage_visitor &dv,
				       bool skip_metadata_snap) {
	// Count the superblock
	bc.inc(superblock_detail::SUPERBLOCK_LOCATION);
	count_trees(tm, sb, bc);

	// Count the metadata snap, if present
	if (!skip_metadata_snap && sb.metadata_snap_ != superblock_detail::SUPERBLOCK_LOCATION) {
		bc.inc(sb.metadata_snap_);

		superblock_detail::superblock snap = read_superblock(tm->get_bm(), sb.metadata_snap_);
		count_trees(tm, snap, bc);
	}

	count_space_maps(tm, sb, bc, dv);

	count_superblock_backups(sb, bc);
}

void thin_provisioning::count_metadata_partial(transaction_manager::ptr tm,
					       superblock_detail::superblock const &sb,
					       block_counter &bc,
					       space_map_detail::damage_visitor &dv,
					       bool skip_metadata_snap) {

	// Count the superblock
	bc.inc(superblock_detail::SUPERBLOCK_LOCATION);
	count_tree_roots(tm, sb, bc);

	// Count the metadata snap, if present
	if (sb.metadata_snap_ != superblock_detail::SUPERBLOCK_LOCATION) {
		bc.inc(sb.metadata_snap_);

		superblock_detail::superblock snap = read_superblock(tm->get_bm(), sb.metadata_snap_);
		count_tree_roots(tm, snap, bc);
	}

	count_space_map_roots(tm, sb, bc, dv);

	count_superblock_backups(sb, bc);
}

//----------------------------------------------------------------

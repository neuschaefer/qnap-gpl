#include "persistent-data/data-structures/btree_damage_visitor.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "thin-provisioning/tiering_utils.h"
#include <stdio.h>
#include <limits>

namespace {
	using namespace thin_provisioning;

	uint32_t find_empty_tier(block_address const *nr_blocks,
				 block_address const *nr_allocated) {
		if (nr_blocks[0]) {
			if (nr_allocated[0])
				throw runtime_error("tier0 has allocated blocks");
			if (nr_blocks[1] + nr_blocks[2])
				throw runtime_error("tier1 and tier2 size is not zero");
			return 0;
		} else if (nr_blocks[1]) {
			if (nr_allocated[1])
				throw runtime_error("tier1 has allocated blocks");
			if (nr_blocks[0] + nr_blocks[2])
				throw runtime_error("tier0 and tier2 size is not zero");
			return 1;
		} else if (nr_blocks[2]) {
			if (nr_allocated[2])
				throw runtime_error("tier2 has allocated blocks");
			if (nr_blocks[0] + nr_blocks[1])
				throw runtime_error("tier0 and tier1 size is not zero");
			return 2;
		} else
			throw runtime_error("cannot find any empty tier");
	}

	class tiering_mapping_collector: public mapping_tree_detail::tiering_visitor {
	public:
		typedef std::map<uint64_t, mapping_tree_detail::tier_block> container_type;
		typedef container_type::const_iterator const_iterator;

		tiering_mapping_collector(uint32_t tier, uint64_t nr_blocks)
			: tier_(tier), nr_blocks_(nr_blocks) {
		}

		void visit(btree_path const &path, mapping_tree_detail::tier_block const &tb) {
			if (!nr_blocks_)
				return;

			if (tb.tier_ == tier_) {
				found_.insert(std::make_pair(path[0], tb));
				--nr_blocks_;
			}
		}

		const_iterator begin() {
			return found_.begin();
		}

		const_iterator end() {
			return found_.end();
		}

	private:
		container_type found_;
		uint32_t tier_;
		uint32_t nr_blocks_;
	};

	// FIXME: duplicated to metadata_dumper.cc
	void raise_metadata_damage() {
		throw std::runtime_error("metadata contains errors (run thin_check for details).");
	}

	// FIXME: duplicated to metadata_dumper.cc
	struct fatal_mapping_damage : public mapping_tree_detail::damage_visitor {
		void visit(mapping_tree_detail::missing_devices const &d) {
			raise_metadata_damage();
		}

		void visit(mapping_tree_detail::missing_mappings const &d) {
			raise_metadata_damage();
		}

		void visit(mapping_tree_detail::missing_details const &d) {
			raise_metadata_damage();
		}

		void visit(mapping_tree_detail::unexpected_mapped_blocks const &d) {
			raise_metadata_damage();
		}

		void visit(mapping_tree_detail::unexpected_highest_mapped_block const &d) {
			raise_metadata_damage();
		}
	};

	class tiering_sm_checker: public mapping_tree_detail::tiering_visitor {
	public:
		tiering_sm_checker(std::vector<checked_space_map::ptr> const &sm,
				   space_map_detail::damage_visitor &dv)
			: sm_(sm), dv_(dv) {
		}

		virtual void visit(btree_path const &, mapping_tree_detail::tier_block const &tb) {
			switch (tb.tier_) {
			case 0:
				tier0_ref_count_[tb.block_]++;
				break;
			case 1:
				tier1_ref_count_[tb.block_]++;
				break;
			case 2:
				tier2_ref_count_[tb.block_]++;
				break;
			default:
				throw;
			}
		}

		void verify() {
			__verify(TIER0_DATA_SPACE_MAP, tier0_ref_count_, sm_[0]);
			__verify(TIER1_DATA_SPACE_MAP, tier1_ref_count_, sm_[1]);
			__verify(TIER2_DATA_SPACE_MAP, tier2_ref_count_, sm_[2]);
		}

	private:
		// FIXME: replace std::map by bitset
		typedef std::map<block_address, unsigned> map_type;

		class space_map_iterator : public persistent_data::space_map::iterator {
		public:
			space_map_iterator(space_map_id sid,
					   map_type const &ref_count,
					   space_map_detail::damage_visitor &dv)
				: sid_(sid), ref_count_(ref_count), dv_(dv) {
			}

			virtual void operator() (block_address b, ref_t c_actual) {
				unsigned c_expected = 0;
				std::map<block_address, unsigned>::const_iterator it = ref_count_.find(b);
				if (it != ref_count_.end())
					c_expected = it->second;
				if (c_expected != c_actual)
					dv_.visit(space_map_detail::unexpected_count(sid_, b, c_expected, c_actual));
			}

		private:
			space_map_id sid_;
			map_type const &ref_count_;
			space_map_detail::damage_visitor &dv_;
		};

		void __verify(space_map_id sid, map_type const &ref_count, checked_space_map::ptr sm) const {
			space_map_iterator it(sid, ref_count, dv_);
			sm->iterate(it);
		}

		std::vector<checked_space_map::ptr> const &sm_;
		space_map_detail::damage_visitor &dv_;
		map_type tier0_ref_count_;
		map_type tier1_ref_count_;
		map_type tier2_ref_count_;
	};
}

namespace thin_provisioning {
	tiering_mapping_creator::tiering_mapping_creator(emitter &e,
							 metadata &md)
		: e_(e),
		  md_(md),
		  tier_ratio_(md.sb_.tier_block_size_ / md.sb_.data_block_size_),
		  tier_block_(std::numeric_limits<uint64_t>::max() - 1),
		  dummy_mon_(base::create_quiet_progress_monitor()),
		  monitor_(*dummy_mon_),
		  current_progress_(0) {
		if (md.sb_.tier_block_size_ % md.sb_.data_block_size_)
			throw runtime_error("tier block size is not divisible by data block size");
		target_tier_ = select_target_tier();
	}

	tiering_mapping_creator::tiering_mapping_creator(emitter &e,
							 metadata &md,
							 base::progress_monitor &mon)
		: e_(e),
		  md_(md),
		  tier_ratio_(md.sb_.tier_block_size_ / md.sb_.data_block_size_),
		  tier_block_(std::numeric_limits<uint64_t>::max() - 1),
		  monitor_(mon),
		  current_progress_(0) {
		if (md.sb_.tier_block_size_ % md.sb_.data_block_size_)
			throw runtime_error("tier block size is not divisible by data block size");
		target_tier_ = select_target_tier();
	}

	tiering_mapping_creator::~tiering_mapping_creator() {
		monitor_.update_percent(100);
	}

	/*!
	 * @brief Insert a tiering mapping corresponding to a pool data block b
	 * @param[in] b  The pool data block
	 * @param[in] c  Ref-count of the pool data block
	 */
	void tiering_mapping_creator::operator() (block_address b, ref_t c) {
		block_address tmp;
		if (c && (tmp = b / tier_ratio_) != tier_block_) {
			tier_block_ = tmp;
			e_.single_tier(tier_block_, target_tier_, tier_block_);

			// Here we do integer division, since that it's nearly
			// impossible to get b overflow by multiplying 100
			uint32_t progress = (b * 100) / md_.data_sm_->get_nr_blocks();
			if (progress > current_progress_) {
				current_progress_ = progress;
				monitor_.update_percent(current_progress_);
			}
		}
	}

	void tiering_mapping_creator::check_space_map_compatibility() {
		uint32_t new_target = select_target_tier();
		if (new_target != target_tier_) {
			ostringstream msg;
			msg << "unexpected target tier for inserting mappings: "
			    << "expected " << target_tier_ << " != actual " << new_target;
			throw runtime_error(msg.str());
		}
	}

	uint32_t tiering_mapping_creator::select_target_tier() {
		block_address nr_blocks[3] = {
			md_.tier_data_sm_[0]->get_nr_blocks(),
			md_.tier_data_sm_[1]->get_nr_blocks(),
			md_.tier_data_sm_[2]->get_nr_blocks()
		};
		block_address nr_allocated[3] = {
			nr_blocks[0] - md_.tier_data_sm_[0]->get_nr_free(),
			nr_blocks[1] - md_.tier_data_sm_[1]->get_nr_free(),
			nr_blocks[2] - md_.tier_data_sm_[2]->get_nr_free(),
		};

		return ::find_empty_tier(nr_blocks, nr_allocated);
	}

	uint32_t find_empty_tier(superblock_detail::superblock const &sb) {
		unsigned char const *sm[3] = {
			sb.tier0_data_space_map_root_,
			sb.tier1_data_space_map_root_,
			sb.tier2_data_space_map_root_,
		};

		block_address nr_blocks[3] = {0};
		block_address nr_allocated[3] = {0};

		for (int i = 0; i < 3; ++i) {
			sm_disk_detail::sm_root_disk const *d;
			d = reinterpret_cast<sm_disk_detail::sm_root_disk const *>(sm[i]);
			sm_disk_detail::sm_root v;
			sm_disk_detail::sm_root_traits::unpack(*d, v);
			nr_blocks[i] = v.nr_blocks_;
			nr_allocated[i] = v.nr_allocated_;
		}

		return ::find_empty_tier(nr_blocks, nr_allocated);
	}

	tiering_sm_creator::tiering_sm_creator(std::vector<checked_space_map::ptr> &sm)
		: sm_(sm) {
	}

	void tiering_sm_creator::visit(btree_path const &, mapping_tree_detail::tier_block const &tb) {
		sm_[tb.tier_]->inc(tb.block_);
	}

	void remap_tiering_mappings(metadata::ptr md, uint32_t src_tier,
				    uint32_t dest_tier, uint64_t nr_blocks,
				    bool quiet, bool commit) {
		if (src_tier > md->tier_data_sm_.size() ||
		    dest_tier > md->tier_data_sm_.size())
			throw std::runtime_error("invalid tier index");

		if (md->tier_data_sm_[src_tier]->get_nr_blocks() -
		    md->tier_data_sm_[src_tier]->get_nr_free() < nr_blocks)
			throw std::runtime_error("insufficient number of blocks in the source tier");

		if (md->tier_data_sm_[dest_tier]->get_nr_free() < nr_blocks)
			throw std::runtime_error("insufficient free space in the destination tier");

		// traverse the tiering mapping tree to obtain the keys to migrate.
		// the process terminates if there's error.
		tiering_mapping_collector collector(src_tier, nr_blocks);
		fatal_mapping_damage dv;
		walk_tiering_tree(*md->tier_tree_, collector, dv);

		// replace the mapped values
		tiering_mapping_collector::const_iterator it = collector.begin();
		for (; it != collector.end(); ++it) {
			// find a free block in the destination tier
			space_map::maybe_block b = md->tier_data_sm_[dest_tier]->new_block();
			if (!b)
				throw std::runtime_error("failed to allocate new block in the destination tier");

			// replace the mapped value
			tiering_tree::key k = {it->first};
			mapping_tree_detail::tier_block v;
			v.tier_ = dest_tier;
			v.block_ = *b;
			md->tier_tree_->insert(k, v);

			// decrease the reference count from the source tier
			md->tier_data_sm_[src_tier]->dec(it->second.block_);

			if (!quiet)
				cout << "moved " << it->second.block_ << " to " << *b << endl;
		}

		if (commit)
			md->commit();
	}

	void auto_remap_tiering_mappings(metadata::ptr md) {
		// expected number of swap blocks
		uint64_t nr_swaps[3] = {
			calculate_tier_swaps(md->tier_data_sm_[0]->get_nr_blocks()),
			calculate_tier_swaps(md->tier_data_sm_[1]->get_nr_blocks()),
			calculate_tier_swaps(md->tier_data_sm_[2]->get_nr_blocks())
		};
		// number of free blocks
		uint64_t nr_free[3] = {
			md->tier_data_sm_[0]->get_nr_free(),
			md->tier_data_sm_[1]->get_nr_free(),
			md->tier_data_sm_[2]->get_nr_free()
		};
		// number of free blocks for migration
		uint64_t nr_usable[3] = {
			(nr_free[0] > nr_swaps[0]) ? (nr_free[0] - nr_swaps[0]) : 0,
			(nr_free[1] > nr_swaps[1]) ? (nr_free[1] - nr_swaps[1]) : 0,
			(nr_free[2] > nr_swaps[2]) ? (nr_free[2] - nr_swaps[2]) : 0
		};
		// number of blocks to be remapped
		uint64_t nr_remaps[3] = {
			(nr_swaps[0] > nr_free[0]) ? (nr_swaps[0] - nr_free[0]) : 0,
			(nr_swaps[1] > nr_free[1]) ? (nr_swaps[1] - nr_free[1]) : 0,
			(nr_swaps[2] > nr_free[2]) ? (nr_swaps[2] - nr_free[2]) : 0
		};

		for (int i = 0; i < 3; i++) {
			int j = (i + 1) % 3;
			while (nr_remaps[i] && i != j) {
				if (nr_usable[j]) {
					uint64_t nr_moved = std::min(nr_remaps[i], nr_usable[j]);
					nr_remaps[i] -= nr_moved;
					nr_usable[j] -= nr_moved;
					cout << i << "," << j << "," << nr_moved << endl;
				}
				j = (j + 1) % 3;
			}
			if (nr_remaps[i]) {
				ostringstream msg;
				msg << "insufficient space for migrating " << nr_remaps[i]
				    << " blocks of tier " << i;
				throw std::runtime_error(msg.str());
			}
		}
	}

	void check_tiering_space_map(tiering_tree const &tier_tree,
				    std::vector<checked_space_map::ptr> const &sm,
				    mapping_tree_detail::damage_visitor &dv1,
				    space_map_detail::damage_visitor &dv2) {
		tiering_sm_checker v(sm, dv2);
		walk_tiering_tree(tier_tree, v, dv1);
		v.verify();
	}
}

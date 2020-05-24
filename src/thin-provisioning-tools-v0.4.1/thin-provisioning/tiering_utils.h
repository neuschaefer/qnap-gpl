#include "base/progress_monitor.h"
#include "persistent-data/space_map.h"
#include "thin-provisioning/emitter.h"
#include "thin-provisioning/metadata.h"

namespace thin_provisioning {
	class tiering_mapping_creator : public persistent_data::space_map::iterator {
	public:
		typedef boost::shared_ptr<tiering_mapping_creator> ptr;

		tiering_mapping_creator(emitter &e,
					metadata &md);

		tiering_mapping_creator(emitter &e,
					metadata &md,
					base::progress_monitor &mon);

		virtual ~tiering_mapping_creator();

		virtual void operator() (block_address b, ref_t c);

		void check_space_map_compatibility();

	private:
		uint32_t select_target_tier();

		emitter &e_;
		metadata &md_;
		uint32_t target_tier_;
		block_address tier_ratio_;
		block_address tier_block_; // temporary iterator

		// for progress monitoring
		std::auto_ptr<base::progress_monitor> dummy_mon_;
		base::progress_monitor &monitor_;
		uint32_t current_progress_; // value=[0,100]
	};

	class tiering_sm_creator: public mapping_tree_detail::tiering_visitor {
	public:
		tiering_sm_creator(std::vector<checked_space_map::ptr> &sm);
		virtual void visit(btree_path const &, mapping_tree_detail::tier_block const &tb);

	private:
		std::vector<checked_space_map::ptr> sm_;
	};

	uint32_t find_empty_tier(superblock_detail::superblock const &sb);
	void remap_tiering_mappings(metadata::ptr md, uint32_t src_tier,
				    uint32_t dest_tier, uint64_t nr_blocks,
				    bool quiet, bool commit);
	void auto_remap_tiering_mappings(metadata::ptr md);
	void check_tiering_space_map(tiering_tree const &tier_tree,
				     std::vector<checked_space_map::ptr> const &sm,
				     mapping_tree_detail::damage_visitor &dv1,
				     space_map_detail::damage_visitor &dv2);
}

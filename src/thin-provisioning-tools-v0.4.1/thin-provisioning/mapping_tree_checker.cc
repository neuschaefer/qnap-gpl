#include "thin-provisioning/mapping_tree_checker.h"

#include <map>

using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	class mapped_block_counter : public mapping_tree_detail::mapping_visitor {
	public:
		mapped_block_counter(): mapped_blocks_(0) {}
		virtual ~mapped_block_counter() {}

		void visit(btree_path const &path, mapping_tree_detail::block_time const &m) {
			++mapped_blocks_;

			if (!highest_mapped_block_ ||
			    *highest_mapped_block_ < path.back())
				highest_mapped_block_ = path.back();
		}

		uint64_t get_mapped_blocks() {
			return mapped_blocks_;
		}

		boost::optional<uint64_t> get_highest_mapped_block() {
			return highest_mapped_block_;
		}
	private:
		uint64_t mapped_blocks_;
		boost::optional<uint64_t> highest_mapped_block_;
	};

	typedef std::map<uint64_t, device_tree_detail::device_details> dd_map;

	class mapping_tree_checker : public mapping_tree_detail::device_visitor {
	public:
		mapping_tree_checker(transaction_manager::ptr tm,
				     dd_map const &details,
				     mapping_tree_detail::damage_visitor &dv,
				     mapping_tree_checker_flags flags)
			: tm_(tm),
			  details_(details),
			  dv_(dv),
			  flags_(flags) {
		}

		virtual ~mapping_tree_checker() {
		}

		void visit(btree_path const &path, block_address tree_root) {
			uint64_t dev_id = path[0];

			dd_map::const_iterator it = details_.find(dev_id);
			bool dd_found = (it != details_.end());
			if (!dd_found) {
				ostringstream msg;
				msg << "mappings present for device " << dev_id
				    << ", but it isn't present in devices tree.";
				dv_.visit(mapping_tree_detail::missing_details(msg.str(), dev_id));
			}

			devices_.insert(dev_id);

			single_mapping_tree tree(*tm_, tree_root,
						 mapping_tree_detail::block_time_ref_counter(space_map::ptr()));

			if (dd_found)
				check_mapping_tree_detailed(tree, dev_id, it->second);
			else
				check_mapping_tree_simple(tree, dev_id);
		}

		void check_device_symmetry() {
			dd_map::const_iterator it = details_.begin();
			for (; it != details_.end(); ++it) {
				if (!devices_.count(it->first)) {
					ostringstream msg;
					msg << "device " << it->first << " is present in devices tree,"
					    << " but it isn't present in top-level mapping tree.";
					dv_.visit(mapping_tree_detail::missing_devices(
						  msg.str(),
						  run<uint64_t>(it->first, it->first + 1)));
				}
			}
		}

	private:
		void check_mapping_tree_detailed(single_mapping_tree const &tree,
						 uint64_t dev_id,
						 device_tree_detail::device_details const &details) {
			mapped_block_counter counter;

			if (flags_ & CHECK_MAPPED_BLOCKS) {
				walk_mapping_tree(tree, dev_id, counter, dv_);

				if (counter.get_mapped_blocks() != details.mapped_blocks_)
					dv_.visit(mapping_tree_detail::unexpected_mapped_blocks(
							dev_id,
							details.mapped_blocks_,
							counter.get_mapped_blocks()));

			} else if (flags_ & CHECK_HIGHEST_MAPPED_BLOCK) {
				walk_last_leaf(tree, dev_id, counter, dv_);

				boost::optional<uint64_t> expected = boost::none;
				boost::optional<uint64_t> actual = counter.get_highest_mapped_block();

				if (details.mapped_blocks_)
					expected = details.mapped_blocks_ - 1;

				if ((!expected != !actual) ||
				    (expected && actual && *actual < *expected)) {
					dv_.visit(mapping_tree_detail::unexpected_highest_mapped_block(
							dev_id,
							expected,
							actual));
				}
			}
		}

		void check_mapping_tree_simple(single_mapping_tree const &tree, uint64_t dev_id) {
			if (flags_ & CHECK_MAPPED_BLOCKS)
				check_mapping_tree(tree, dev_id, dv_);
			else if (flags_ & CHECK_HIGHEST_MAPPED_BLOCK)
				check_last_leaf(tree, dev_id, dv_);
		}

		transaction_manager::ptr tm_;
		dd_map const &details_;

		// FIXME: it might not be good to couple a damage_visitor with a ValueVisitor
		mapping_tree_detail::damage_visitor &dv_;

		std::set<uint64_t> devices_;
		mapping_tree_checker_flags flags_;
	};
}

//----------------------------------------------------------------

namespace thin_provisioning {
	void check_mapping_tree(transaction_manager::ptr tm,
				superblock_detail::superblock const &sb,
				std::map<uint64_t, device_tree_detail::device_details> const &details,
				mapping_tree_detail::damage_visitor &dv,
				mapping_tree_checker_flags f) {
		mapping_tree_checker checker(tm, details, dv, f);
		dev_tree dtree(*tm, sb.data_mapping_root_,
			       mapping_tree_detail::mtree_traits::ref_counter(tm));
		walk_mapping_tree(dtree, checker, dv);
		checker.check_device_symmetry();
	}
}

//----------------------------------------------------------------


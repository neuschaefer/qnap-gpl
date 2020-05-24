#include "thin-provisioning/mapping_tree.h"

#include "persistent-data/data-structures/btree_damage_visitor.h"
#include "persistent-data/space_map.h"

using namespace persistent_data;

//----------------------------------------------------------------

namespace thin_provisioning {
	namespace mapping_tree_detail {
		space_map_ref_counter::space_map_ref_counter(space_map::ptr sm)
			: sm_(sm)
		{
		}

		void
		space_map_ref_counter::inc(block_address b)
		{
			sm_->inc(b);
		}

		void
		space_map_ref_counter::dec(block_address b)
		{
			sm_->dec(b);
		}

		//--------------------------------

		block_time_ref_counter::block_time_ref_counter(space_map::ptr sm)
			: sm_(sm)
		{
		}

		void
		block_time_ref_counter::inc(block_time bt)
		{
			sm_->inc(bt.block_);
		}

		void
		block_time_ref_counter::dec(block_time bt)
		{
			sm_->dec(bt.block_);
		}

		//--------------------------------

		void
		block_traits::unpack(disk_type const &disk, value_type &value)
		{
			uint64_t v = to_cpu<uint64_t>(disk);
			value.block_ = v >> 24;
			value.zeroed_ = (v >> 22) & 3;
			value.time_ = v & ((1 << 22) - 1);
		}

		// block_time::time_ could be 24-bit if block_time::zeroed_ was omitted
		void
		block_traits::pack(value_type const &value, disk_type &disk)
		{
			uint64_t v = (value.block_ << 24) | ((value.zeroed_ & 3) << 22) | value.time_;
			disk = base::to_disk<base::le64>(v);
		}

		//--------------------------------

		mtree_ref_counter::mtree_ref_counter(transaction_manager::ptr tm)
			: tm_(tm)
		{
		}

		void
		mtree_ref_counter::inc(block_address b)
		{
		}

		void
		mtree_ref_counter::dec(block_address b)
		{
		}

		//--------------------------------

		void
		mtree_traits::unpack(disk_type const &disk, value_type &value)
		{
			value = base::to_cpu<uint64_t>(disk);
		}

		void
		mtree_traits::pack(value_type const &value, disk_type &disk)
		{
			disk = base::to_disk<base::le64>(value);
		}

		//--------------------------------

		tier_block_ref_counter::tier_block_ref_counter(std::vector<checked_space_map::ptr> const& sm)
			: sm_(sm)
		{
		}

		void
		tier_block_ref_counter::inc(tier_block tb)
		{
			if (!sm_[tb.tier_].get()) {
				ostringstream msg;
				msg << "space map for tier " << tb.tier_ << " not found";
				throw std::runtime_error(msg.str());
			}
			sm_[tb.tier_]->inc(tb.block_);
		}

		void
		tier_block_ref_counter::dec(tier_block tb)
		{
			if (!sm_[tb.tier_].get()) {
				ostringstream msg;
				msg << "space map for tier " << tb.tier_ << " not found";
				throw std::runtime_error(msg.str());
			}
			sm_[tb.tier_]->dec(tb.block_);
		}

		//--------------------------------

		void tier_block_traits::unpack(disk_type const &disk, value_type &value)
		{
			uint64_t v = to_cpu<uint64_t>(disk);
			value.tier_ = v >> 61;
			value.block_ = (v >> 24) & ((static_cast<uint64_t>(1) << 37) - 1);
			value.reserved_ = v & ((1 << 24) - 1);
		}

		void tier_block_traits::pack(value_type const &value, disk_type &disk)
		{
			uint64_t v = (static_cast<uint64_t>(value.tier_) << 61) |
				     ((value.block_ & ((static_cast<uint64_t>(1) << 37) - 1)) << 24) |
				     (value.reserved_ & ((static_cast<uint64_t>(1) << 24) - 1));
			disk = base::to_disk<base::le64>(v);
		}

		//--------------------------------

		missing_devices::missing_devices(std::string const &desc, run<uint64_t> const &keys)
		: desc_(desc),
		  keys_(keys)
		{
		}

		void
		missing_devices::visit(damage_visitor &v) const
		{
			v.visit(*this);
		}

		//--------------------------------

		missing_mappings::missing_mappings(std::string const &desc,
						   uint64_t thin_dev,
						   run<uint64_t> const &keys)
			: desc_(desc),
			thin_dev_(thin_dev),
			keys_(keys)
		{
		}

		missing_mappings::missing_mappings(std::string const &desc,
						   run<uint64_t> const &keys)
			: desc_(desc),
			  keys_(keys)
		{
		}

		void
		missing_mappings::visit(damage_visitor &v) const
		{
			v.visit(*this);
		}

		//--------------------------------

		missing_details::missing_details(std::string const &desc,
						 uint64_t thin_dev)
			: desc_(desc),
			  thin_dev_(thin_dev)
		{
		}

		void
		missing_details::visit(damage_visitor &v) const
		{
			v.visit(*this);
		}

		//--------------------------------

		unexpected_mapped_blocks::unexpected_mapped_blocks(uint64_t thin_dev,
								   uint64_t expected,
								   uint64_t actual)
			: thin_dev_(thin_dev),
			  expected_(expected),
			  actual_(actual)
		{
		}

		void
		unexpected_mapped_blocks::visit(damage_visitor &v) const
		{
			v.visit(*this);
		}

		//--------------------------------

		unexpected_highest_mapped_block::unexpected_highest_mapped_block(uint64_t thin_dev,
										 boost::optional<uint64_t> expected,
										 boost::optional<uint64_t> actual)
			: thin_dev_(thin_dev),
			  expected_(expected),
			  actual_(actual)
		{
		}

		void
		unexpected_highest_mapped_block::visit(damage_visitor &v) const
		{
			v.visit(*this);
		}
	}
}

//----------------------------------------------------------------

namespace {
	using namespace thin_provisioning;
	using namespace mapping_tree_detail;

	struct noop_block_time_visitor : public mapping_tree_detail::mapping_visitor {
		virtual void visit(btree_path const &, block_time const &) {
		}
	};

	struct noop_block_visitor : public mapping_tree_detail::device_visitor {
		virtual void visit(btree_path const &, uint64_t) {
		}
	};

	struct noop_tier_block_visitor : public mapping_tree_detail::tiering_visitor {
		virtual void visit(btree_path const &, tier_block const &) {
		}
	};

	struct noop_uint32_visitor : public mapping_tree_detail::uint32_visitor {
		virtual void visit(btree_path const &, uint32_t) {
		}
	};

	class dev_tree_damage_visitor {
	public:
		dev_tree_damage_visitor(damage_visitor &v)
		: v_(v) {
		}

		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			switch (path.size()) {
			case 0:
				v_.visit(missing_devices(d.desc_, d.lost_keys_));
				break;

			case 1:
				v_.visit(missing_mappings(d.desc_, path[0], d.lost_keys_));
				break;

			default:
				throw std::runtime_error("dev_tree_damage_visitor: path too long");
			}
		}

	private:
		damage_visitor &v_;
	};

	class mapping_tree_damage_visitor {
	public:
		mapping_tree_damage_visitor(damage_visitor &v)
		: v_(v) {
		}

		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			switch (path.size()) {
			case 0:
				v_.visit(missing_devices(d.desc_, d.lost_keys_));
				break;

			case 1:
				v_.visit(missing_mappings(d.desc_, path[0], d.lost_keys_));
				break;

			default:
				throw std::runtime_error("mapping_tree_damage_visitor: path too long");
			}
		}

	private:
		damage_visitor &v_;
	};

	class single_mapping_tree_damage_visitor {
	public:
		single_mapping_tree_damage_visitor(damage_visitor &v,
						   boost::optional<uint64_t> dev_id)
		: v_(v),
		  dev_id_(dev_id) {
		}

		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			switch (path.size()) {
			case 0:
				if (dev_id_)
					v_.visit(missing_mappings(d.desc_, *dev_id_, d.lost_keys_));
				else
					v_.visit(missing_mappings(d.desc_, d.lost_keys_));
				break;

			default:
				throw std::runtime_error("single_mapping_tree_damage_visitor: path too long");
			}
		}

	private:
		damage_visitor &v_;
		boost::optional<uint64_t> dev_id_;
	};

	class simple_damage_visitor {
	public:
		simple_damage_visitor(damage_visitor &v)
		: v_(v) {
		}

		// FIXME: do not use missing_mappings, use another damage class, e.g., missing_tier
		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			v_.visit(missing_mappings(d.desc_, d.lost_keys_));
		}

	private:
		damage_visitor &v_;
	};

}

void
thin_provisioning::walk_mapping_tree(dev_tree const &tree,
				     mapping_tree_detail::device_visitor &dev_v,
				     mapping_tree_detail::damage_visitor &dv)
{
	dev_tree_damage_visitor ll_dv(dv);
	btree_visit_values(tree, dev_v, ll_dv);
}

void
thin_provisioning::check_mapping_tree(dev_tree const &tree,
				      mapping_tree_detail::damage_visitor &visitor)
{
	noop_block_visitor dev_v;
	walk_mapping_tree(tree, dev_v, visitor);
}

void
thin_provisioning::walk_mapping_tree(mapping_tree const &tree,
				     mapping_tree_detail::mapping_visitor &mv,
				     mapping_tree_detail::damage_visitor &dv)
{
	mapping_tree_damage_visitor ll_dv(dv);
	btree_visit_values(tree, mv, ll_dv);
}

void
thin_provisioning::check_mapping_tree(mapping_tree const &tree,
				      mapping_tree_detail::damage_visitor &visitor)
{
	noop_block_time_visitor mv;
	walk_mapping_tree(tree, mv, visitor);
}

void
thin_provisioning::walk_mapping_tree(single_mapping_tree const &tree,
				     boost::optional<uint64_t> dev_id,
				     mapping_tree_detail::mapping_visitor &mv,
				     mapping_tree_detail::damage_visitor &dv)
{
	single_mapping_tree_damage_visitor ll_dv(dv, dev_id);
	btree_visit_values(tree, mv, ll_dv);
}

void
thin_provisioning::check_mapping_tree(single_mapping_tree const &tree,
				      boost::optional<uint64_t> dev_id,
				      mapping_tree_detail::damage_visitor &visitor)
{
	noop_block_time_visitor mv;
	walk_mapping_tree(tree, dev_id, mv, visitor);
}

void
thin_provisioning::walk_last_leaf(single_mapping_tree const &tree,
				  boost::optional<uint64_t> dev_id,
				  mapping_tree_detail::mapping_visitor &mv,
				  mapping_tree_detail::damage_visitor &dv)
{
	single_mapping_tree_damage_visitor sdv(dv, dev_id);
	btree_visit_last_values(tree, mv, sdv);
}

void
thin_provisioning::check_last_leaf(single_mapping_tree const &tree,
				   boost::optional<uint64_t> dev_id,
				   mapping_tree_detail::damage_visitor &visitor)
{
	noop_block_time_visitor mv;
	walk_last_leaf(tree, dev_id, mv, visitor);
}

void
thin_provisioning::walk_tiering_tree(tiering_tree const &tree,
				     mapping_tree_detail::tiering_visitor &tier_v,
				     mapping_tree_detail::damage_visitor &dv)
{
	simple_damage_visitor sdv(dv);
	btree_visit_values(tree, tier_v, sdv);
}

void
thin_provisioning::check_tiering_tree(tiering_tree const &tree,
				      mapping_tree_detail::damage_visitor &visitor)
{
	noop_tier_block_visitor tier_v;
	walk_tiering_tree(tree, tier_v, visitor);
}

void
thin_provisioning::walk_clone_tree(clone_tree const &tree,
				   mapping_tree_detail::uint32_visitor &clone_v,
				   mapping_tree_detail::damage_visitor &dv)
{
	simple_damage_visitor sdv(dv);
	btree_visit_values(tree, clone_v, sdv);
}

void
thin_provisioning::check_clone_tree(clone_tree const &tree,
				    mapping_tree_detail::damage_visitor &visitor)
{
	noop_uint32_visitor clone_v;
	walk_clone_tree(tree, clone_v, visitor);
}

//----------------------------------------------------------------

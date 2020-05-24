#include "thin-provisioning/device_tree.h"
#include "thin-provisioning/superblock.h"

#include "persistent-data/data-structures/btree_damage_visitor.h"

#include <limits>

using namespace persistent_data;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	using namespace device_tree_detail;

	struct visitor_adapter {
		visitor_adapter(device_visitor &dv)
		: dv_(dv) {
		}

		void visit(btree_path const &path, device_details const &dd) {
			dv_.visit(path[0], dd);
		}

	private:
		device_visitor &dv_;
	};

	// No op for now, should add sanity checks in here however.
	struct noop_visitor : public device_visitor {
		virtual void visit(block_address dev_id, device_details const &dd) {
		}
	};

	class ll_damage_visitor {
	public:
		// FIXME: is the namespace needed on here?
		ll_damage_visitor(device_tree_detail::damage_visitor &v)
		: v_(v) {
		}

		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			v_.visit(missing_devices(d.desc_, d.lost_keys_));
		}

	private:
		device_tree_detail::damage_visitor &v_;
	};
}

namespace thin_provisioning {
	namespace device_tree_detail {
		device_details::device_details()
			: mapped_blocks_(0),
			  transaction_id_(0),
			  creation_time_(0),
			  snapshotted_time_(0),
			  snap_origin_(std::numeric_limits<uint64_t>::max()) {
		}

		device_details::device_details(superblock_detail::superblock const &sb)
			: mapped_blocks_(0),
			  transaction_id_(0),
			  creation_time_((sb.version_ >= 3) ? 1 :0),
			  snapshotted_time_((sb.version_ >= 3) ? 1 :0),
			  snap_origin_(std::numeric_limits<uint64_t>::max()) {
			if (sb.version_ >= 4) {
				cloned_time_ = 1;
				scaned_index_ = std::numeric_limits<uint64_t>::max();
			}
		}

		void
		device_details_traits::unpack(device_details_disk const &disk, device_details &value)
		{
			value.mapped_blocks_ = to_cpu<uint64_t>(disk.mapped_blocks_);
			value.transaction_id_ = to_cpu<uint64_t>(disk.transaction_id_);
			value.creation_time_ = to_cpu<uint32_t>(disk.creation_time_);
			value.snapshotted_time_ = to_cpu<uint32_t>(disk.snapshotted_time_);
			value.cloned_time_ = boost::none;
			value.scaned_index_ = boost::none;
			value.snap_origin_ = to_cpu<uint64_t>(disk.snap_origin_);
		}

		void
		device_details_traits::pack(device_details const &value, device_details_disk &disk)
		{
			disk.mapped_blocks_ = to_disk<le64>(value.mapped_blocks_);
			disk.transaction_id_ = to_disk<le64>(value.transaction_id_);
			disk.creation_time_ = to_disk<le32>(value.creation_time_);
			disk.snapshotted_time_ = to_disk<le32>(value.snapshotted_time_);
			disk.snap_origin_ = to_disk<le64>(value.snap_origin_);
		}

		void
		device_details_traits_cl::unpack(device_details_disk_cl const &disk, device_details &value)
		{
			value.mapped_blocks_ = to_cpu<uint64_t>(disk.mapped_blocks_);
			value.transaction_id_ = to_cpu<uint64_t>(disk.transaction_id_);
			value.creation_time_ = to_cpu<uint32_t>(disk.creation_time_);
			value.snapshotted_time_ = to_cpu<uint32_t>(disk.snapshotted_time_);
			value.cloned_time_ = to_cpu<uint32_t>(disk.cloned_time_);
			value.scaned_index_ = to_cpu<uint64_t>(disk.scaned_index_);
			value.snap_origin_ = to_cpu<uint64_t>(disk.snap_origin_);
		}

		void
		device_details_traits_cl::pack(device_details const &value, device_details_disk_cl &disk)
		{
			disk.mapped_blocks_ = to_disk<le64>(value.mapped_blocks_);
			disk.transaction_id_ = to_disk<le64>(value.transaction_id_);
			disk.creation_time_ = to_disk<le32>(value.creation_time_);
			disk.snapshotted_time_ = to_disk<le32>(value.snapshotted_time_);
			disk.cloned_time_ = value.cloned_time_ ? to_disk<le32>(*value.cloned_time_) : to_disk<le32>(0U);
			disk.scaned_index_ = value.scaned_index_ ? to_disk<le64>(*value.scaned_index_) :
					     to_disk<le64>(std::numeric_limits<uint64_t>::max());
			disk.snap_origin_ = to_disk<le64>(value.snap_origin_);
		}

		missing_devices::missing_devices(std::string const &desc,
						 run<uint64_t> const &keys)
			: desc_(desc),
			  keys_(keys) {
		}

		void missing_devices::visit(damage_visitor &v) const {
			v.visit(*this);
		}
	}

	device_tree_adapter::device_tree_adapter(transaction_manager &tm, device_tree_type type) {
		if (type == TYPE_CLONE)
			details_cl_ = device_tree_cl::ptr(new device_tree_cl(tm,
							  device_tree_detail::device_details_traits_cl::ref_counter()));
		else
			details_ = device_tree::ptr(new device_tree(tm,
						    device_tree_detail::device_details_traits::ref_counter()));
	}

	device_tree_adapter::device_tree_adapter(transaction_manager &tm,
						 block_address root,
						 uint32_t version) {
		if (version >= 4)
			details_cl_ = device_tree_cl::ptr(new device_tree_cl(tm, root,
							  device_tree_detail::device_details_traits_cl::ref_counter()));
		else
			details_ = device_tree::ptr(new device_tree(tm, root,
						    device_tree_detail::device_details_traits::ref_counter()));
	}

	device_tree_adapter::~device_tree_adapter() {}

	boost::optional<device_details> device_tree_adapter::lookup(key const &key) const {
		return details_cl_.get() ? details_cl_->lookup(key) : details_->lookup(key);
	}

	block_address device_tree_adapter::get_root() const {
		return details_cl_.get() ? details_cl_->get_root() : details_->get_root();
	}

	void device_tree_adapter::insert(key const &key, device_details const &value) {
		if (details_cl_.get())
			details_cl_->insert(key, value);
		else
			details_->insert(key, value);
	}

	device_tree::ptr device_tree_adapter::get_device_tree() const {
		return details_;
	}

	device_tree_cl::ptr device_tree_adapter::get_device_tree_cl() const {
		return details_cl_;
	}
}

//----------------------------------------------------------------

void
thin_provisioning::walk_device_tree(device_tree_adapter const &tree,
				    device_tree_detail::device_visitor &vv,
				    device_tree_detail::damage_visitor &dv)
{
	visitor_adapter av(vv);
	ll_damage_visitor ll_dv(dv);
	if (tree.get_device_tree_cl())
		btree_visit_values(*tree.get_device_tree_cl(), av, ll_dv);
	else
		btree_visit_values(*tree.get_device_tree(), av, ll_dv);
}

void
thin_provisioning::check_device_tree(device_tree_adapter const &tree, damage_visitor &visitor)
{
	noop_visitor vv;
	walk_device_tree(tree, vv, visitor);
}

//----------------------------------------------------------------

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

#include "thin-provisioning/patch_emitter.h"

#include "persistent-data/space-maps/disk_structures.h"
#include "thin-provisioning/superblock.h"

//----------------------------------------------------------------

namespace {
	using namespace thin_provisioning;
	using namespace superblock_detail;

	class patch_emitter : public emitter {
	public:
		// TODO: allow to set the patch mode (skip or overwrite if there's exist mappings)
		patch_emitter(metadata::ptr md)
			: md_(md),
			  nr_data_blocks_(0) {
			sm_disk_detail::sm_root_disk const *d =
				reinterpret_cast<sm_disk_detail::sm_root_disk const *>(md_->sb_.data_space_map_root_);
			sm_disk_detail::sm_root v;
			sm_disk_detail::sm_root_traits::unpack(*d, v);
			nr_data_blocks_ = v.nr_blocks_;
		}

		virtual void begin_superblock(std::string const &uuid,
					      uint64_t time,
					      uint64_t trans_id,
					      boost::optional<uint32_t> flags,
					      boost::optional<uint32_t> version,
					      uint32_t data_block_size,
					      uint64_t nr_data_blocks,
					      boost::optional<uint64_t> metadata_snap,
					      boost::optional<uint64_t> reserve_block_count,
					      boost::optional<uint32_t> tier_block_size,
					      boost::optional<vector<uint64_t> > tier_data_blocks) {
			// DO nothing
		}

		virtual void end_superblock() {
			md_->commit();
		}

		virtual void begin_device(uint32_t dev,
					  uint64_t mapped_blocks,
					  uint64_t trans_id,
					  uint64_t creation_time,
					  uint64_t snap_time,
					  boost::optional<uint32_t> clone_time,
					  boost::optional<uint64_t> scaned_index,
					  uint64_t snap_origin) {
			uint64_t key[1] = {dev};
			dev_tree::maybe_value subtree_root = md_->mappings_top_level_->lookup(key);
			device_tree::maybe_value v = md_->details_->lookup(key);

			if (subtree_root.is_initialized() && v.is_initialized()) {
				current_device_ = dev;
				current_device_details_ = *v;
				current_mapping_ = open_mapping_tree(*subtree_root);

			// insert a new device if it is not present
			} else if ((subtree_root.is_initialized() || v.is_initialized()) == 0) {
				current_device_ = dev;
				current_device_details_.mapped_blocks_ = 0;
				current_device_details_.transaction_id_ = trans_id;
				current_device_details_.creation_time_ = (uint32_t)creation_time;
				current_device_details_.snapshotted_time_ = (uint32_t)snap_time;
				current_device_details_.cloned_time_ = boost::none;
				current_device_details_.scaned_index_ = boost::none;
				current_device_details_.snap_origin_ = snap_origin;

				if (clone_time)
					current_device_details_.cloned_time_ = *clone_time;

				current_mapping_ = new_mapping_tree();
			} else
				throw std::runtime_error("Device not found");

		}

		virtual void end_device() {
			uint64_t key[1] = {*current_device_};

			md_->details_->insert(key, current_device_details_);

			// remember to update the root of mapping tree after modifying the top-level tree
			md_->mappings_top_level_->insert(key, current_mapping_->get_root());
			md_->mappings_->set_root(md_->mappings_top_level_->get_root()); // FIXME: ugly

			current_device_ = boost::optional<uint32_t>();
		}

		virtual void begin_named_mapping(std::string const &name) {
			throw runtime_error("not implemented");
		}

		virtual void end_named_mapping() {
			throw runtime_error("not implemented");
		}

		virtual void identifier(std::string const &name) {
			throw runtime_error("not implemented");
		}

		virtual void range_map(uint64_t origin_begin, uint64_t data_begin, uint32_t time, uint64_t len, uint32_t fastzero) {
			for (uint64_t i = 0; i < len; i++)
				single_map(origin_begin++, data_begin++, time, fastzero);
		}

		virtual void single_map(uint64_t origin_block, uint64_t data_block, uint32_t time, uint32_t fastzero) {
			if (!current_device_)
				throw runtime_error("not in device");

			if (data_block >= nr_data_blocks_) {
				std::ostringstream out;
				out << "mapping beyond end of data device (" << data_block
				    << " >= " << nr_data_blocks_ << ")";
				throw std::runtime_error(out.str());
			}
			uint64_t key[1] = {origin_block};
			mapping_tree_detail::block_time bt;
			bt.block_ = data_block;
			bt.time_ = time;
			bt.zeroed_ = fastzero;
			current_device_details_.mapped_blocks_ +=
				static_cast<uint64_t>(current_mapping_->insert(key, bt));
			md_->data_sm_->inc(bt.block_);

			// FIXME: btree::insert() could return the original mapped value,
			//        then we should decrease the data_sm ref-count of the original mapped value
		}

		virtual void begin_tier() {
			// do nothing
		}

		virtual void end_tier() {
			// do nothing
		}

		virtual void range_tier(uint64_t origin_begin, uint32_t tier, uint64_t data_begin, uint64_t len) {
			for (uint64_t i = 0; i < len; i++)
				single_tier(origin_begin++, tier, data_begin++);
		}

		virtual void single_tier(uint64_t origin_block, uint32_t tier, uint64_t data_block) {
			uint64_t key[1] = {origin_block};
			mapping_tree_detail::tier_block tb;
			tb.tier_ = tier;
			tb.block_ = data_block;
			tb.reserved_ = 0;
			md_->tier_tree_->insert(key, tb);
			md_->tier_data_sm_[tier]->inc(data_block);

			// FIXME: after overwrite, decrease the tier_data_sm ref-count of the original mapped value
		}

		virtual void begin_uint32(std::string const &name) {
			throw runtime_error("not implemented");
		}

		virtual void end_uint32(std::string const &name) {
			throw runtime_error("not implemented");
		}

		virtual void range_uint32(uint64_t key_begin, uint32_t value, uint64_t len) {
			throw runtime_error("not implemented");
		}

		virtual void single_uint32(uint64_t key, uint32_t value) {
			throw runtime_error("not implemented");
		}

	private:
		single_mapping_tree::ptr new_mapping_tree() {
			return single_mapping_tree::ptr(
				new single_mapping_tree(*md_->tm_,
							mapping_tree_detail::block_time_ref_counter(md_->data_sm_)));
		}

		single_mapping_tree::ptr open_mapping_tree(block_address root) {
			return single_mapping_tree::ptr(
				new single_mapping_tree(*md_->tm_, root,
							mapping_tree_detail::block_time_ref_counter(md_->data_sm_)));
		}

		metadata::ptr md_;
		block_address nr_data_blocks_; // data block count of the output pool
		boost::optional<uint32_t> current_device_;
		device_tree_detail::device_details current_device_details_;
		single_mapping_tree::ptr current_mapping_;
	};
}

//----------------------------------------------------------------

emitter::ptr
thin_provisioning::create_patch_emitter(metadata::ptr md)
{
	return emitter::ptr(new patch_emitter(md));
}

//----------------------------------------------------------------

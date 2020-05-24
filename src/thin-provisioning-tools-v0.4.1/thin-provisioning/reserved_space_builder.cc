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

#include "thin-provisioning/reserved_space_builder.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "thin-provisioning/superblock.h"

#include <limits>

//----------------------------------------------------------------

namespace {
	using namespace thin_provisioning;
	using namespace superblock_detail;

	class reserved_space_builder : public emitter {
	public:
		reserved_space_builder(metadata::ptr md)
			: md_(md),
			  in_superblock_(false),
			  reserved_block_count_(0) {
			if (!(md_->sb_.flags_ & THIN_FEATURE_FAST_BLOCK_CLONE) ||
			    md_->sb_.reserve_block_count_ ||
			    md_->sb_.clone_root_)
				throw runtime_error("reserved space is in-use");
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
			in_superblock_ = true;
			clone_counts_ = clone_tree::ptr(new clone_tree(*md_->tm_, uint32_traits::ref_counter()));
		}

		virtual void end_superblock() {
			if (!in_superblock_)
				throw runtime_error("missing superblock");

			// Note that unlike the use case of restore_emitter,
			// here we have two copy of metadata: original and the shadowed.
			// Leaving the shadowed metadata uncommitted does not
			// affect the original metadata. Therefore, we don't
			// need to decrease the reference count of the
			// temporary node while exception happened.
			md_->clone_counts_ = clone_counts_->clone();
			md_->metadata_sm_->dec(clone_counts_->get_root());
			md_->sb_.reserve_block_count_ = reserved_block_count_;
			md_->commit();
			reserved_block_count_ = 0;
			in_superblock_ = false;
		}

		virtual void begin_device(uint32_t dev,
					  uint64_t mapped_blocks,
					  uint64_t trans_id,
					  uint64_t creation_time,
					  uint64_t snap_time,
					  boost::optional<uint32_t> clone_time,
					  boost::optional<uint64_t> scaned_index,
					  uint64_t snap_origin) {
			if (!in_superblock_)
				throw runtime_error("missing superblock");

			device_tree_detail::device_details &details = current_device_details_;
			details.mapped_blocks_ = mapped_blocks;
			details.transaction_id_ = trans_id;
			details.creation_time_ = creation_time;
			details.snapshotted_time_ = snap_time;
			details.cloned_time_ = boost::none;
			if (clone_time)
				details.cloned_time_ = *clone_time;

			// reset the scanned_index since that we are going
			// to rebuild the clone reference count of this device
			details.scaned_index_ = std::numeric_limits<uint64_t>::max();

			current_device_ = boost::optional<uint32_t>(dev);
		}

		virtual void end_device() {
			uint64_t key[1] = {*current_device_};
			md_->details_->insert(key, current_device_details_);
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
			// insert clone reference count
			if (current_device_details_.snap_origin_ == std::numeric_limits<uint64_t>::max() &&
			    current_device_details_.scaned_index_) {
				uint64_t key[1] = {data_block};
				boost::optional<uint32_t> mvalue = clone_counts_->lookup(key);
				uint32_t count = mvalue ? *mvalue : 0;
				if (count > 0 && md_->data_sm_->get_nr_free() <= reserved_block_count_) {
					ostringstream msg;
					msg << "insufficient free space for reserved blocks"
					    << ": device " << *current_device_
					    << ", origin_block " << origin_block
					    << ", data_block " << data_block;
					throw runtime_error(msg.str());
				}
				clone_counts_->insert(key, count + 1);
				reserved_block_count_ += (count > 0) ? 1 : 0;
			}
		}

		virtual void begin_tier() {
			throw runtime_error("not implemented");
		}

		virtual void end_tier() {
			throw runtime_error("not implemented");
		}

		virtual void range_tier(uint64_t origin_begin, uint32_t tier, uint64_t data_begin, uint64_t len) {
			throw runtime_error("not implemented");
		}

		virtual void single_tier(uint64_t origin_block, uint32_t tier, uint64_t data_block) {
			throw runtime_error("not implemented");
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
		metadata::ptr md_;
		bool in_superblock_;
		boost::optional<uint32_t> current_device_;
		device_tree_detail::device_details current_device_details_;
		clone_tree::ptr clone_counts_;
		uint64_t reserved_block_count_;
	};
}

//----------------------------------------------------------------

emitter::ptr
thin_provisioning::create_reserved_space_builder(metadata::ptr md)
{
	return emitter::ptr(new reserved_space_builder(md));
}

//----------------------------------------------------------------

// Copyright (C) 2011 Red Hat, Inc. All rights reserved.
//
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

#include "thin-provisioning/emitter.h"
#include "thin-provisioning/metadata_dumper.h"
#include "thin-provisioning/mapping_tree.h"

#include <limits>

using namespace persistent_data;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	void raise_metadata_damage() {
		throw std::runtime_error("metadata contains errors (run thin_check for details).\n"
					 "perhaps you wanted to run with --repair");
	}

	//--------------------------------

	struct ignore_details_damage : public device_tree_detail::damage_visitor {
		void visit(device_tree_detail::missing_devices const &d) {
		}
	};

	struct fatal_details_damage : public device_tree_detail::damage_visitor {
		void visit(device_tree_detail::missing_devices const &d) {
			raise_metadata_damage();
		}
	};

	device_tree_detail::damage_visitor::ptr details_damage_policy(bool repair) {
		typedef device_tree_detail::damage_visitor::ptr dvp;

		if (repair)
			return dvp(new ignore_details_damage());
		else
			return dvp(new fatal_details_damage());
	}

	//--------------------------------

	struct ignore_mapping_damage : public mapping_tree_detail::damage_visitor {
		void visit(mapping_tree_detail::missing_devices const &d) {
		}

		void visit(mapping_tree_detail::missing_mappings const &d) {
		}

		void visit(mapping_tree_detail::missing_details const &d) {
		}

		void visit(mapping_tree_detail::unexpected_mapped_blocks const &d) {
		}

		void visit(mapping_tree_detail::unexpected_highest_mapped_block const &d) {
		}
	};

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

	mapping_tree_detail::damage_visitor::ptr mapping_damage_policy(bool repair) {
		typedef mapping_tree_detail::damage_visitor::ptr mvp;

		if (repair)
			return mvp(new ignore_mapping_damage());
		else
			return mvp(new fatal_mapping_damage());
	}

	//--------------------------------

	typedef map<block_address, device_tree_detail::device_details> dd_map;

	class details_extractor : public device_tree_detail::device_visitor {
	public:
		void visit(block_address dev_id, device_tree_detail::device_details const &dd) {
			dd_.insert(make_pair(dev_id, dd));
		}

		dd_map const &get_details() const {
			return dd_;
		}

	private:
		dd_map dd_;
	};

	class mapping_emitter : public mapping_tree_detail::mapping_visitor {
	public:
		mapping_emitter(emitter::ptr e)
			: e_(e),
			  in_range_(false) {
		}

		~mapping_emitter() {
			end_mapping();
		}

		typedef mapping_tree_detail::block_time block_time;
		void visit(btree_path const &path, block_time const &bt) {
			add_mapping(path[0], bt);
		}

	private:
		void start_mapping(uint64_t origin_block, block_time const &bt) {
			origin_start_ = origin_block;
			dest_start_ = bt.block_;
			time_ = bt.time_;
			fastzero_ = bt.zeroed_;
			len_ = 1;
			in_range_ = true;
		}

		void end_mapping() {
			if (in_range_) {
				in_range_ = false;

				if (len_ == 1)
					e_->single_map(origin_start_, dest_start_, time_, fastzero_);
				else
					e_->range_map(origin_start_, dest_start_, time_, len_, fastzero_);
			}
		}

		void add_mapping(uint64_t origin_block, block_time const &bt) {
			if (!in_range_)
				start_mapping(origin_block, bt);

			else if (origin_block == origin_start_ + len_ &&
				 bt.block_ == dest_start_ + len_ &&
				 time_ == bt.time_ && fastzero_ == bt.zeroed_)
				len_++;

			else {
				end_mapping();
				start_mapping(origin_block, bt);
			}
		}

		emitter::ptr e_;
		block_address origin_start_;
		block_address dest_start_;
		uint32_t time_, fastzero_;
		block_address len_;
		bool in_range_;
	};

	class mapping_tree_emitter : public mapping_tree_detail::device_visitor {
	public:
		mapping_tree_emitter(metadata::ptr md,
				     emitter::ptr e,
				     dd_map const &dd,
				     repair_level rl,
				     uint32_t verbosity,
				     mapping_tree_detail::damage_visitor::ptr damage_policy)
			: md_(md),
			  e_(e),
			  dd_(dd),
			  repair_level_(rl),
			  output_verbosity_(verbosity),
			  damage_policy_(damage_policy) {
		}

		void visit(btree_path const &path, block_address tree_root) {
			block_address dev_id = path[0];

			dd_map::const_iterator it = dd_.find(path[0]);

			if (it != dd_.end()) {
				device_tree_detail::device_details const &d = it->second;

				if ((output_verbosity_ & DUMP_ORIGINS_ONLY) &&
				    d.snap_origin_ != std::numeric_limits<uint64_t>::max())
					return;

				e_->begin_device(dev_id,
						 d.mapped_blocks_,
						 d.transaction_id_,
						 d.creation_time_,
						 d.snapshotted_time_,
						 d.cloned_time_,
						 d.scaned_index_,
						 d.snap_origin_);

				if (output_verbosity_ & DUMP_DATA_MAPPINGS) {
					try {
						emit_mappings(dev_id, tree_root);
					} catch (std::exception &e) {
						std::terminate();
					}
				}

				e_->end_device();
			} else if (repair_level_ == DUMP_ORPHAN_DEVICES) {
				uint32_t default_pool_time = 0;
				boost::optional<uint32_t> default_cloned_time;
				boost::optional<uint64_t> default_scaned_index;
				if (md_->sb_.version_ >= 4) {
					default_pool_time = 1;
					default_cloned_time = default_pool_time;
					default_scaned_index = std::numeric_limits<uint64_t>::max();
				}
				e_->begin_device(dev_id,
						 0,
						 0,
						 default_pool_time,
						 default_pool_time,
						 default_cloned_time,
						 default_scaned_index,
						 std::numeric_limits<uint64_t>::max());

				if (output_verbosity_ & DUMP_DATA_MAPPINGS) {
					try {
						emit_mappings(dev_id, tree_root);
					} catch (std::exception &e) {
						std::terminate();
					}
				}

				e_->end_device();
			} else {
				ostringstream msg;
				msg << "mappings present for device " << dev_id
				    << ", but it isn't present in device tree";
				throw runtime_error(msg.str());
			}
		}

	private:
		void emit_mappings(uint64_t dev_id, block_address subtree_root) {
			mapping_emitter me(e_);
			single_mapping_tree tree(*md_->tm_, subtree_root,
						 mapping_tree_detail::block_time_ref_counter(md_->data_sm_));
			walk_mapping_tree(tree, dev_id, static_cast<mapping_tree_detail::mapping_visitor &>(me), *damage_policy_);
		}

		metadata::ptr md_;
		emitter::ptr e_;
		dd_map const &dd_;
		repair_level repair_level_;
		uint32_t output_verbosity_;
		mapping_tree_detail::damage_visitor::ptr damage_policy_;
	};

	class tiering_emitter : public mapping_tree_detail::tiering_visitor {
	public:
		tiering_emitter(emitter::ptr e)
			: e_(e),
			  mapping_in_range_(false) {
		}

		~tiering_emitter() {
			end_mapping();
		}

		typedef mapping_tree_detail::tier_block tier_block;
		void visit(btree_path const &path, tier_block const &tb) {
			add_mapping(path[0], tb);
		}

	private:
		void start_mapping(uint64_t origin_block, tier_block const &tb) {
			mapping_tier_ = tb.tier_;
			mapping_origin_start_ = origin_block;
			mapping_dest_start_ = tb.block_;
			mapping_len_ = 1;
			mapping_in_range_ = true;
		}

		void end_mapping() {
			if (mapping_in_range_) {
				if (mapping_len_ == 1)
					e_->single_tier(mapping_origin_start_, mapping_tier_, mapping_dest_start_);
				else
					e_->range_tier(mapping_origin_start_, mapping_tier_, mapping_dest_start_, mapping_len_);

				mapping_in_range_ = false;
			}
		}

		void add_mapping(uint64_t origin_block, tier_block const &tb) {
			if (!mapping_in_range_)
				start_mapping(origin_block, tb);

			else if (origin_block == mapping_origin_start_ + mapping_len_ &&
				 tb.block_ == mapping_dest_start_ + mapping_len_ &&
				 mapping_tier_ == tb.tier_)
				mapping_len_++;

			else {
				end_mapping();
				start_mapping(origin_block, tb);
			}
		}

		emitter::ptr e_;
		uint32_t mapping_tier_;
		block_address mapping_origin_start_;
		block_address mapping_dest_start_;
		block_address mapping_len_;
		bool mapping_in_range_;
	};

	// TODO: add class tiering_mapping_emitter : public mapping_tree_detail::mapping_visitor
	// to dump tiering mappings of specific bottom-level data mapping tree


	class uint32_emitter : public mapping_tree_detail::uint32_visitor {
	public:
		uint32_emitter(emitter::ptr e)
			: e_(e),
			  in_range_(false) {
		}

		~uint32_emitter() {
			end_mapping();
		}

		void visit(btree_path const &path, uint32_t value) {
			add_mapping(path[0], value);
		}

	private:
		void start_mapping(uint64_t key, uint32_t value) {
			key_start_ = key;
			value_ = value;
			len_ = 1;
			in_range_ = true;
		}

		void end_mapping() {
			if (in_range_) {
				if (len_ == 1)
					e_->single_uint32(key_start_, value_);
				else
					e_->range_uint32(key_start_, value_, len_);

				in_range_ = false;
			}
		}

		void add_mapping(uint64_t key, uint32_t value) {
			if (!in_range_)
				start_mapping(key, value);

			else if (key == key_start_ + len_ &&
					value == value_)
				len_++;
			else {
				end_mapping();
				start_mapping(key, value);
			}
		}

		emitter::ptr e_;
		block_address key_start_;
		uint32_t value_;
		block_address len_;
		bool in_range_;
	};

	class space_map_dumper : public persistent_data::space_map::iterator {
	public:
		typedef boost::shared_ptr<space_map_dumper> ptr;

		space_map_dumper(emitter::ptr e): e_(e) {
		}

		virtual void operator() (block_address b, ref_t c) {
			if (!c)
				return;

			btree_path p(1, b);
			e_.visit(p, c);
		}
	private:
		uint32_emitter e_;
	};
}

//----------------------------------------------------------------

void
thin_provisioning::metadata_dump(metadata::ptr md, emitter::ptr e, repair_level rl, uint32_t verbosity)
{
	details_extractor de;
	bool repair = (rl > NO_REPAIR) ? true : false;
	device_tree_detail::damage_visitor::ptr dd_policy(details_damage_policy(repair));
	walk_device_tree(*md->details_, de, *dd_policy);

	boost::optional<uint64_t> reserve_block_count;
	if (md->sb_.clone_root_)
		reserve_block_count = md->sb_.reserve_block_count_;

	boost::optional<vector<uint64_t> > tier_data_blocks;
	if (md->sb_.tier_num_) {
		tier_data_blocks = vector<uint64_t>(md->sb_.tier_num_);
		for (size_t i = 0; i < md->sb_.tier_num_; ++i) {
			if (md->tier_data_sm_[i].get()) {
				(*tier_data_blocks)[i] = md->tier_data_sm_[i]->get_nr_blocks();
			}
		}
	}

	e->begin_superblock("", md->sb_.time_,
			    md->sb_.trans_id_,
			    md->sb_.flags_,
			    md->sb_.version_,
			    md->sb_.data_block_size_,
			    md->data_sm_->get_nr_blocks(),
			    boost::optional<block_address>(),
			    reserve_block_count,
			    md->sb_.tier_block_size_,
			    tier_data_blocks);

	{
		mapping_tree_detail::damage_visitor::ptr md_policy(mapping_damage_policy(repair));
		mapping_tree_emitter mte(md, e, de.get_details(), rl, verbosity, mapping_damage_policy(repair));
		walk_mapping_tree(*md->mappings_top_level_, mte, *md_policy);

		if ((verbosity & DUMP_CLONE_COUNTS) && md->sb_.clone_root_) {
			e->begin_uint32("clone");
			metadata_dump_clone_tree(md, e, repair, *md->clone_counts_);
			e->end_uint32("clone");
		}

		if ((verbosity & DUMP_TIERING) && md->sb_.tier_num_) {
			e->begin_tier();
			metadata_dump_tiering_tree(md, e, repair, *md->tier_tree_);
			e->end_tier();
		}
	}

	e->end_superblock();
}

//----------------------------------------------------------------

static void
metadata_dump(metadata::ptr md, emitter::ptr e, repair_level rl, uint32_t verbosity, uint64_t dev_id)
{
	details_extractor de;
	uint64_t key[1] = {dev_id};

	dev_tree::maybe_value single_mapping_tree_root = md->mappings_top_level_->lookup(key);
	if (!single_mapping_tree_root)
		throw runtime_error("specified device id not found in top-level mapping tree");

	device_tree::maybe_value details;
	try {
		details = md->details_->lookup(key);
	} catch (std::exception &e) {

	}
	if (!details) {
		// allow dev_id to be absent in device details tree in repair mode
		if (rl < DUMP_ORPHAN_DEVICES)
			throw runtime_error("specified device id not found in device details tree.\n"
					    "perhaps you wanted to run with --repair=2");
	} else
		de.visit(dev_id, *details);

	{
		bool repair = (rl > NO_REPAIR) ? true : false;
		mapping_tree_emitter mte(md, e, de.get_details(), rl, verbosity, mapping_damage_policy(repair));
		std::vector<uint64_t> path(1, dev_id); // btree_detail::btree_path is std::vector<uint64_t>
		mte.visit(path, *single_mapping_tree_root);

		// TODO: dump tiering mappings of specific device
	}
}

void
thin_provisioning::metadata_dump(metadata::ptr md, emitter::ptr e, repair_level rl, uint32_t verbosity,
				 std::vector<uint64_t> const& dev_id)
{
	boost::optional<uint64_t> reserve_block_count;
	if (md->sb_.clone_root_)
		reserve_block_count = md->sb_.reserve_block_count_;

	boost::optional<vector<uint64_t> > tier_data_blocks;
	if (md->sb_.tier_num_) {
		tier_data_blocks = vector<uint64_t>(md->sb_.tier_num_);
		for (size_t i = 0; i < md->sb_.tier_num_; ++i) {
			if (md->tier_data_sm_[i].get()) {
				(*tier_data_blocks)[i] = md->tier_data_sm_[i]->get_nr_blocks();
			}
		}
	}

	e->begin_superblock("", md->sb_.time_,
			    md->sb_.trans_id_,
			    md->sb_.flags_,
			    md->sb_.version_,
			    md->sb_.data_block_size_,
			    md->data_sm_->get_nr_blocks(),
			    boost::optional<block_address>(),
			    reserve_block_count,
			    md->sb_.tier_block_size_,
			    tier_data_blocks);
	// FIXME: ugly
	// Do not explicitly iterate through the list of devices
	for (size_t i = 0; i < dev_id.size(); i++ )
		::metadata_dump(md, e, rl, verbosity, dev_id[i]);

	e->end_superblock();
}
//----------------------------------------------------------------

void
thin_provisioning::metadata_dump_subtree(metadata::ptr md, emitter::ptr e, bool repair, uint64_t subtree_root) {
	mapping_emitter me(e);
	single_mapping_tree tree(*md->tm_, subtree_root,
				 mapping_tree_detail::block_time_ref_counter(md->data_sm_));
	walk_mapping_tree(tree, boost::none, static_cast<mapping_tree_detail::mapping_visitor &>(me),
			  *mapping_damage_policy(repair));
}

//----------------------------------------------------------------

void
thin_provisioning::metadata_dump_clone_tree(metadata::ptr md, emitter::ptr e, bool repair, clone_tree &tree)
{
	uint32_emitter u32e(e);
	walk_clone_tree(*md->clone_counts_, u32e, *mapping_damage_policy(repair));
}

//----------------------------------------------------------------

void
thin_provisioning::metadata_dump_tiering_tree(metadata::ptr md, emitter::ptr e, bool repair, uint64_t node)
{
	vector<void const*> tier_data_sm_roots;
	tier_data_sm_roots.push_back(static_cast<void const*>(md->sb_.tier0_data_space_map_root_));
	tier_data_sm_roots.push_back(static_cast<void const*>(md->sb_.tier1_data_space_map_root_));
	tier_data_sm_roots.push_back(static_cast<void const*>(md->sb_.tier2_data_space_map_root_));
	tier_data_sm_roots.resize(md->sb_.tier_num_);
	std::vector<checked_space_map::ptr> tier_data_sm = open_multiple_disk_sm(*md->tm_, tier_data_sm_roots);

	tiering_tree tree(*md->tm_, node,
			  mapping_tree_detail::tier_block_ref_counter(tier_data_sm));

	metadata_dump_tiering_tree(md, e, repair, tree);
}

void
thin_provisioning::metadata_dump_tiering_tree(metadata::ptr md, emitter::ptr e, bool repair, tiering_tree &tree)
{
	tiering_emitter te(e);
	walk_tiering_tree(tree, te, *mapping_damage_policy(repair));
}

//----------------------------------------------------------------

void
thin_provisioning::metadata_dump_superblock_begin(metadata::ptr md, emitter::ptr e) {
	boost::optional<uint64_t> reserve_block_count;
	if (md->sb_.clone_root_)
		reserve_block_count = md->sb_.reserve_block_count_;

	boost::optional<vector<uint64_t> > tier_data_blocks;
	if (md->sb_.tier_num_) {
		tier_data_blocks = vector<uint64_t>(md->sb_.tier_num_);
		for (size_t i = 0; i < md->sb_.tier_num_; ++i) {
			if (md->tier_data_sm_[i].get()) {
				(*tier_data_blocks)[i] = md->tier_data_sm_[i]->get_nr_blocks();
			}
		}
	}

	e->begin_superblock("", md->sb_.time_,
			    md->sb_.trans_id_,
			    md->sb_.flags_,
			    md->sb_.version_,
			    md->sb_.data_block_size_,
			    md->data_sm_->get_nr_blocks(),
			    boost::optional<block_address>(),
			    reserve_block_count,
			    md->sb_.tier_block_size_,
			    tier_data_blocks);
}

//----------------------------------------------------------------

void
thin_provisioning::metadata_dump_superblock_end(metadata::ptr md, emitter::ptr e) {
	e->end_superblock();
}

//----------------------------------------------------------------

void
thin_provisioning::metadata_dump_data_space_map(metadata::ptr md, emitter::ptr e) {
	metadata_dump_superblock_begin(md, e);
	{
		checked_space_map::ptr data_sm = open_disk_sm(*md->tm_, static_cast<void *>(md->sb_.data_space_map_root_));
		space_map_dumper::ptr d = space_map_dumper::ptr(new space_map_dumper(e));
		data_sm->iterate(*d);
	}
	metadata_dump_superblock_end(md, e);
}

//----------------------------------------------------------------

void
thin_provisioning::metadata_dump_space_map(metadata::ptr md, emitter::ptr e) {
	metadata_dump_superblock_begin(md, e);
	{
		checked_space_map::ptr meta_sm = open_metadata_sm(*md->tm_, static_cast<void *>(md->sb_.metadata_space_map_root_));
		space_map_dumper::ptr d = space_map_dumper::ptr(new space_map_dumper(e));
		meta_sm->iterate(*d);
	}
	metadata_dump_superblock_end(md, e);
}

//----------------------------------------------------------------

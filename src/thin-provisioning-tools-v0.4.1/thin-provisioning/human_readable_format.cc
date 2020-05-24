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

#include "human_readable_format.h"

#include <iostream>

using namespace std;
using namespace thin_provisioning;

//----------------------------------------------------------------

namespace {
	template <typename T>
	std::ostream &operator << (ostream &out, boost::optional<T> const &maybe) {
		if (maybe)
			out << *maybe;

		return out;
	}

	class compact_emitter : public emitter {
	public:
		compact_emitter(ostream &out)
			: out_(out) {
		}

		void begin_superblock(string const &uuid,
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
			out_ << "begin superblock=>uuid:\"" << uuid << "\""
			     << ",time:" << time
			     << ",transaction:" << trans_id
			     << ",flags:" << (flags ? *flags : 0)
			     << ",version:" << (version ? *version : 1)
			     << ",data_block_size:" << data_block_size
			     << ",nr_data_blocks:" << nr_data_blocks;
			if (metadata_snap)
				out_ << ",metadata_snap:" << *metadata_snap;
			if (reserve_block_count)
				out_ << ",reserve_block_count:" << *reserve_block_count;
			if (tier_block_size && tier_data_blocks && tier_data_blocks->size()) {
				out_ << ",tier_block_size:" << *tier_block_size
				     << ",nr_tiers:" << tier_data_blocks->size();
				for (size_t i = 0; i < tier_data_blocks->size(); ++i)
					out_ << ",tier" << i << "_data_blocks:" << (*tier_data_blocks)[i];
			}
			out_ << endl;
		}

		void end_superblock() {
			out_ << "end superblock" << endl;
		}

		void begin_device(uint32_t dev_id,
				  uint64_t mapped_blocks,
				  uint64_t trans_id,
				  uint64_t creation_time,
				  uint64_t snap_time,
				  boost::optional<uint32_t> clone_time,
				  boost::optional<uint64_t> scaned_index,
				  uint64_t snap_origin) {
			out_ << "device=>dev_id:" << dev_id << ","
			     << "mapped_blocks:" << mapped_blocks << ","
			     << "transaction:" << trans_id << ","
			     << "creation_time:" << creation_time << ","
			     << "snap_time:" << snap_time << ",";
			if (clone_time)
				out_ << "clone_time:" << *clone_time << ",";
			if (scaned_index)
				out_ << "scaned_index:" << *scaned_index << ",";
			out_ << "snap_origin:" << snap_origin << endl;
		}

		void end_device() {
			out_ << endl;
		}

		void begin_named_mapping(string const &name) {
			out_ << "begin named mapping"
			     << endl;
		}

		void end_named_mapping() {
			out_ << "end named mapping"
			     << endl;
		}

		void identifier(string const &name) {
			out_ << "identifier:" << name << endl;
		}

		void range_map(uint64_t origin_begin, uint64_t data_begin, uint32_t time, uint64_t len, uint32_t fastzero) {
			out_ << origin_begin
			     << "," << data_begin
			     << "," << len
			     << "," << time
			     << "," << fastzero
			     << endl;
		}

		void single_map(uint64_t origin_block, uint64_t data_block, uint32_t time, uint32_t fastzero) {
			out_ << origin_block
			     << "," << data_block
			     << ",1"	/* len = 1*/
			     << "," << time
			     << "," << fastzero
			     << endl;
		}

		void begin_tier() {
			out_ << "begin tiering" << endl;
		}

		void end_tier() {
			out_ << "end tiering" << endl;
		}

		void range_tier(uint64_t origin_begin, uint32_t tier, uint64_t data_begin, uint64_t len) {
			out_ << origin_begin
			     << "," << tier
			     << "," << data_begin
			     << "," << len
			     << endl;
		}

		void single_tier(uint64_t origin_block, uint32_t tier, uint64_t data_block) {
			out_ << origin_block
			     << "," << tier
			     << "," << data_block
			     << ",1"	/* len = 1*/
			     << endl;
		}

		void begin_uint32(std::string const &name) {
			out_ << "begin " << name << endl;
		}

		void end_uint32(std::string const &name) {
			out_ << "end " << name << endl;
		}

		void range_uint32(uint64_t key_begin, uint32_t value, uint64_t len) {
			out_ << key_begin
			     << "," << value
			     << "," << len
			     << endl;
		}

		void single_uint32(uint64_t key, uint32_t value) {
			out_ << key
			     << "," << value
			     << ",1"    /* len = 1*/
			     << endl;
		}

	private:
		ostream &out_;
	};

	class hr_emitter : public emitter {
	public:
		hr_emitter(ostream &out)
			: out_(out) {
		}

		void begin_superblock(string const &uuid,
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
			out_ << "begin superblock: \"" << uuid << "\""
			     << ", " << time
			     << ", " << trans_id
			     << ", " << (flags ? *flags : 0)
			     << ", " << (version ? *version : 1)
			     << ", " << data_block_size
			     << ", " << nr_data_blocks;
			if (metadata_snap)
				out_ << ", " << *metadata_snap;
			if (reserve_block_count)
				out_ << ", " << *reserve_block_count;
			if (tier_block_size && tier_data_blocks && tier_data_blocks->size()) {
				out_ << ", " << *tier_block_size
				     << ", " << tier_data_blocks->size();
				for (size_t i = 0; i < tier_data_blocks->size(); ++i)
					out_ << ", " << i << ":" << (*tier_data_blocks)[i];
			}
			out_ << endl;
		}

		void end_superblock() {
			out_ << "end superblock" << endl;
		}

		void begin_device(uint32_t dev_id,
				  uint64_t mapped_blocks,
				  uint64_t trans_id,
				  uint64_t creation_time,
				  uint64_t snap_time,
				  boost::optional<uint32_t> clone_time,
				  boost::optional<uint64_t> scaned_index,
				  uint64_t snap_origin) {
			out_ << "device: " << dev_id << endl
			     << "mapped_blocks: " << mapped_blocks << endl
			     << "transaction: " << trans_id << endl
			     << "creation time: " << creation_time << endl
			     << "snap time: " << snap_time << endl;
			if (clone_time)
				out_ << "clone_time: " << *clone_time << endl;
			if (scaned_index)
				out_ << "scaned_index: " << *scaned_index << endl;
			out_ << "snap_origin: " << snap_origin << endl;
		}

		void end_device() {
			out_ << endl;
		}

		void begin_named_mapping(string const &name) {
			out_ << "begin named mapping"
			     << endl;
		}

		void end_named_mapping() {
			out_ << "end named mapping"
			     << endl;
		}

		void identifier(string const &name) {
			out_ << "identifier: " << name << endl;
		}

		void range_map(uint64_t origin_begin, uint64_t data_begin, uint32_t time, uint64_t len, uint32_t fastzero) {
			out_ << "    (" << origin_begin
			     << ".." << origin_begin + len - 1
			     << ") -> (" << data_begin
			     << ".." << data_begin + len - 1
			     << "), "
			     << time
			     << ", " << fastzero
			     << endl;
		}

		void single_map(uint64_t origin_block, uint64_t data_block, uint32_t time, uint32_t fastzero) {
			out_ << "    " << origin_block
			     << " -> " << data_block
			     << ", " << time
			     << ", " << fastzero
			     << endl;
		}

		void begin_tier() {
			out_ << "begin tiering" << endl;
		}

		void end_tier() {
			out_ << "end tiering" << endl;
		}

		void range_tier(uint64_t origin_begin, uint32_t tier, uint64_t data_begin, uint64_t len) {
			out_ << "    (" << origin_begin
			     << ".." << origin_begin + len - 1
			     << ") -> " << tier
			     << ", (" << data_begin
			     << ".." << data_begin + len - 1
			     << ")"
			     << endl;
		}

		void single_tier(uint64_t origin_block, uint32_t tier, uint64_t data_block) {
			out_ << "    " << origin_block
			     << " -> " << tier
			     << ", " << data_block
			     << endl;
		}

		void begin_uint32(std::string const &name) {
			out_ << "begin " << name << endl;
		}

		void end_uint32(std::string const &name) {
			out_ << "end " << name << endl;
		}

		void range_uint32(uint64_t key_begin, uint32_t value, uint64_t len) {
			out_ << "    (" << key_begin
			     << ".." << key_begin + len - 1
			     << ") -> " << value
			     << endl;
		}

		void single_uint32(uint64_t key, uint32_t value) {
			out_ << "    " << key
			     << " -> " << value
			     << endl;
		}

	private:
		ostream &out_;
	};
}

//----------------------------------------------------------------

thin_provisioning::emitter::ptr
thin_provisioning::create_human_readable_emitter(ostream &out)
{
	return emitter::ptr(new hr_emitter(out));
}
thin_provisioning::emitter::ptr
thin_provisioning::create_compact_emitter(ostream &out)
{
	return emitter::ptr(new compact_emitter(out));
}

//----------------------------------------------------------------

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

#ifndef EMITTER_H
#define EMITTER_H

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <string>
#include <stdint.h>
#include <vector>

//----------------------------------------------------------------

namespace thin_provisioning {

	//------------------------------------------------
	// Here's a little grammar for how this all hangs together:
	//
	// superblock := <uuid> <time> <trans_id> <data_block_size> device*
	// device := <dev id> <transaction id> <creation time> <snap time> <binding>
        // binding := (<identifier> | <mapping>)*
	// mapping := range_map | single_map
	// range_map := <origin_begin> <origin_end> <data_begin>
	// single_map := <origin> <data>
	// named_mapping := <identifier> <mapping>
	//------------------------------------------------

	class emitter {
	public:
		typedef boost::shared_ptr<emitter> ptr;

		virtual ~emitter() {}

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
					      boost::optional<std::vector<uint64_t> > tier_data_blocks) = 0;
		virtual void end_superblock() = 0;

		virtual void begin_device(uint32_t dev_id,
					  uint64_t mapped_blocks,
					  uint64_t trans_id,
					  uint64_t creation_time,
					  uint64_t snap_time,
					  boost::optional<uint32_t> clone_time,
					  boost::optional<uint64_t> scaned_index,
					  uint64_t snap_origin) = 0;
		virtual void end_device() = 0;

		virtual void begin_named_mapping(std::string const &name) = 0;
		virtual void end_named_mapping() = 0;

		virtual void identifier(std::string const &name) = 0;
		virtual void range_map(uint64_t origin_begin, uint64_t data_begin, uint32_t time, uint64_t len, uint32_t fastzero) = 0;
		virtual void single_map(uint64_t origin_block, uint64_t data_block, uint32_t time, uint32_t fastzero) = 0;

		virtual void begin_tier() = 0;
		virtual void end_tier() = 0;

		virtual void range_tier(uint64_t origin_begin, uint32_t tier, uint64_t data_begin, uint64_t len) = 0;
		virtual void single_tier(uint64_t origin_block, uint32_t tier, uint64_t data_block) = 0;

		virtual void begin_uint32(std::string const &name) = 0;
		virtual void end_uint32(std::string const &name) = 0;
		virtual void range_uint32(uint64_t key_begin, uint32_t value, uint64_t len) = 0;
		virtual void single_uint32(uint64_t key, uint32_t value) = 0;
	};
}

//----------------------------------------------------------------

#endif

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

#include "space_map.h"

using namespace persistent_data;

//----------------------------------------------------------------

sm_decrementer::sm_decrementer(space_map::ptr sm, block_address b)
	: sm_(sm),
	  b_(b),
	  released_(false) {
}

sm_decrementer::~sm_decrementer() {
	if (!released_)
		sm_->dec(b_);
}

void
sm_decrementer::dont_bother() {
	released_ = true;
}

//----------------------------------------------------------------

space_map_detail::missing_counts::missing_counts(std::string const &desc,
						 uint32_t space_map_id,
						 base::run<block_address> const &lost)
	: desc_(desc),
	  space_map_id_(space_map_id),
	  lost_(lost)
{
}

space_map_detail::unexpected_count::unexpected_count(uint32_t space_map_id,
						     block_address b,
						     ref_t expected,
						     boost::optional<ref_t> actual)
	: space_map_id_(space_map_id),
	  b_(b),
	  expected_(expected),
	  actual_(actual)
{
}

//----------------------------------------------------------------

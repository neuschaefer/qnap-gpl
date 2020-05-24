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

#ifndef SPACE_MAP_H
#define SPACE_MAP_H

#include "persistent-data/block.h"
#include "persistent-data/block_counter.h"
#include "persistent-data/run.h"

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <functional>

//----------------------------------------------------------------

namespace persistent_data {
	typedef uint32_t ref_t;

	// FIXME: document these methods

	class space_map {
	public:
		typedef boost::shared_ptr<space_map> ptr;

		virtual ~space_map() {};

		virtual block_address get_nr_blocks() const = 0;
		virtual block_address get_nr_free() const = 0;
		virtual ref_t get_count(block_address b) const = 0;
		virtual void set_count(block_address b, ref_t c) = 0;
		virtual void commit() = 0;

		virtual void inc(block_address b) = 0;
		virtual void dec(block_address b) = 0;

		// FIXME: change these to return an optional, failure is
		// not that rare if we're restricting the area that's
		// searched.
		typedef boost::optional<block_address> maybe_block;

		typedef std::pair<block_address, block_address> span;
		typedef boost::optional<span> maybe_span;

		struct span_iterator {
			typedef boost::optional<span> maybe_span;

			virtual maybe_span first() = 0;
			virtual maybe_span next() = 0;
		};

		struct single_span_iterator : public span_iterator {
			single_span_iterator(span const &s)
			  : s_(s) {
			}

			virtual maybe_span first() {
				return maybe_span(s_);
			}

			virtual maybe_span next() {
				return maybe_span();
			}

		private:
			span s_;
		};

		// deliberately not virtual
		maybe_block new_block() {
			single_span_iterator it(span(0, get_nr_blocks()));
			return new_block(it);
		}

		virtual maybe_block find_free(span_iterator &it) = 0;

		virtual bool count_possibly_greater_than_one(block_address b) const = 0;

		virtual void extend(block_address extra_blocks) = 0;

		struct iterator {
			virtual ~iterator() {}

			virtual void operator() (block_address b, ref_t c) = 0;
		};

		virtual void iterate(iterator &it) const {
			throw std::runtime_error("iterate() not implemented");
		}

		virtual void no_lookaside_iterate(iterator &it) const {
			throw std::runtime_error("no_lookaside_iterate() not implemented");
		}

		// This is a concrete method
		maybe_block new_block(span_iterator &it) {
			maybe_block mb = find_free(it);

			if (mb)
				inc(*mb);

			return mb;
		}
	};

	//--------------------------------

	namespace space_map_detail {
		class damage {
		public:
			virtual ~damage() {}
		};

		class missing_counts : public damage {
		public:
			missing_counts(std::string const &desc,
				       uint32_t space_map_id,
				       base::run<block_address> const &lost);

			std::string desc_;
			uint32_t space_map_id_;
			base::run<block_address> lost_;
		};

		class unexpected_count : public damage {
		public:
			unexpected_count(uint32_t space_map_id,
					 block_address b,
					 ref_t expected,
					 boost::optional<ref_t> actual);

			uint32_t space_map_id_;
			block_address b_;
			ref_t expected_;
			boost::optional<ref_t> actual_;
		};

		class visitor {
		public:
			typedef boost::shared_ptr<visitor> ptr;

			virtual ~visitor() {}
			virtual void visit(block_address b, uint32_t count) = 0;
		};

		class damage_visitor {
		public:
			typedef boost::shared_ptr<damage_visitor> ptr;

			virtual ~damage_visitor() {}
			virtual void visit(missing_counts const &mc) = 0;
			virtual void visit(unexpected_count const &uc) = 0;

			// the identifier will be propagated to the emitted damages,
			// for applications to identify the belonging space map.
			// FIXME: any better approaches?
			virtual void set_id(uint32_t id) {
				id_ = id;
			}
			virtual int get_id() {
				return id_;
			}
		private:
			uint32_t id_;
		};

		class noop_damage_visitor : public damage_visitor {
		public:
			virtual void visit(missing_counts const &mc) {}
			virtual void visit(unexpected_count const &uc) {}
		};
	}

	//--------------------------------

	class persistent_space_map : public space_map {
	public:
		typedef boost::shared_ptr<persistent_space_map> ptr;

		virtual void count_metadata(block_counter &bc, space_map_detail::damage_visitor &dv) const = 0;
		virtual size_t root_size() const = 0;
		virtual void copy_root(void *dest, size_t len) const = 0;
	};

	class checked_space_map : public persistent_space_map {
	public:
		typedef boost::shared_ptr<checked_space_map> ptr;

		virtual void visit(space_map_detail::visitor &v,
				   space_map_detail::damage_visitor &dv) const {
			throw std::runtime_error("space_map.visit not implemented");
		}

		// FIXME: should this be in the base space_map class?
		virtual ptr clone() const = 0;
		virtual void destroy() = 0;
	};

	class sm_adjust {
	public:
		sm_adjust(space_map::ptr sm, block_address b, int delta)
			: sm_(sm),
			  b_(b),
			  delta_(delta) {

			adjust_count(delta_);
		}

		~sm_adjust() {
			adjust_count(-delta_);
		}

		void release() {
			delta_ = 0;
		}

	private:
		void adjust_count(int delta) {
			if (delta == 1)
				sm_->inc(b_);

			else if (delta == -1)
				sm_->dec(b_);

			else
				sm_->set_count(b_, sm_->get_count(b_) + delta);
		}

		space_map::ptr sm_;
		block_address b_;
		int delta_;
	};

	class sm_decrementer {
	public:
		sm_decrementer(space_map::ptr sm, block_address b);
		~sm_decrementer();
		void dont_bother();

	private:
		space_map::ptr sm_;
		block_address b_;
		bool released_;
	};
}

//----------------------------------------------------------------

#endif

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

#include "base/endian_utils.h"

#include "persistent-data/space-maps/disk.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "persistent-data/space-maps/recursive.h"
#include "persistent-data/space-maps/careful_alloc.h"

#include "persistent-data/data-structures/btree_damage_visitor.h"
#include "persistent-data/data-structures/btree_counter.h"
#include "persistent-data/checksum.h"
#include "persistent-data/math_utils.h"
#include "persistent-data/transaction_manager.h"

using namespace persistent_data;
using namespace std;
using namespace sm_disk_detail;

//----------------------------------------------------------------

namespace {
	uint64_t const BITMAP_CSUM_XOR = 240779;

	struct bitmap_block_validator : public bcache::validator {
		virtual void check(void const *raw, block_address location) const {
			bitmap_header const *data = reinterpret_cast<bitmap_header const *>(raw);
			crc32c sum(BITMAP_CSUM_XOR);
			sum.append(&data->not_used, MD_BLOCK_SIZE - sizeof(uint32_t));
			if (sum.get_sum() != to_cpu<uint32_t>(data->csum)) {
				std::ostringstream out;
				out << "bad checksum in space map bitmap (block "
				    << location << ")";
				throw checksum_error(out.str());
			}

			if (to_cpu<uint64_t>(data->blocknr) != location) {
				std::ostringstream out;
				out << "bad block nr in space map bitmap (block "
				    << location << ")";
				throw checksum_error(out.str());
			}
		}

		virtual bool check_raw(void const *raw) const {
			bitmap_header const *data = reinterpret_cast<bitmap_header const *>(raw);
			crc32c sum(BITMAP_CSUM_XOR);
			sum.append(&data->not_used, MD_BLOCK_SIZE - sizeof(uint32_t));
			if (sum.get_sum() != to_cpu<uint32_t>(data->csum))
				return false;
			return true;
		}

		virtual void prepare(void *raw, block_address location) const {
			bitmap_header *data = reinterpret_cast<bitmap_header *>(raw);
			data->blocknr = to_disk<base::le64, uint64_t>(location);

			crc32c sum(BITMAP_CSUM_XOR);
			sum.append(&data->not_used, MD_BLOCK_SIZE - sizeof(uint32_t));
			data->csum = to_disk<base::le32>(sum.get_sum());
		}
	};

	//--------------------------------

	uint64_t const INDEX_CSUM_XOR = 160478;

	// FIXME: factor out the common code in these validators
	struct index_block_validator : public bcache::validator {
		virtual void check(void const *raw, block_address location) const {
			metadata_index const *mi = reinterpret_cast<metadata_index const *>(raw);
			crc32c sum(INDEX_CSUM_XOR);
			sum.append(&mi->padding_, MD_BLOCK_SIZE - sizeof(uint32_t));
			if (sum.get_sum() != to_cpu<uint32_t>(mi->csum_)) {
				std::ostringstream out;
				out << "bad checksum in metadata index block (block "
				    << location << ")";
				throw checksum_error(out.str());
			}

			if (to_cpu<uint64_t>(mi->blocknr_) != location) {
				std::ostringstream out;
				out << "bad block nr in metadata index block (block "
				    << location << ")";
				throw checksum_error(out.str());
			}
		}

		virtual bool check_raw(void const *raw) const {
			metadata_index const *mi = reinterpret_cast<metadata_index const *>(raw);
			crc32c sum(INDEX_CSUM_XOR);
			sum.append(&mi->padding_, MD_BLOCK_SIZE - sizeof(uint32_t));
			if (sum.get_sum() != to_cpu<uint32_t>(mi->csum_))
				return false;
			return true;
		}

		virtual void prepare(void *raw, block_address location) const {
			metadata_index *mi = reinterpret_cast<metadata_index *>(raw);
			mi->blocknr_ = to_disk<base::le64, uint64_t>(location);

			crc32c sum(INDEX_CSUM_XOR);
			sum.append(&mi->padding_, MD_BLOCK_SIZE - sizeof(uint32_t));
			mi->csum_ = to_disk<base::le32>(sum.get_sum());
		}
	};

	//--------------------------------

	class bitmap {
	public:
		typedef transaction_manager::read_ref read_ref;
		typedef transaction_manager::write_ref write_ref;

		bitmap(transaction_manager &tm,
		       index_entry const &ie,
		       bcache::validator::ptr v)
			: tm_(tm),
			  validator_(v),
			  ie_(ie) {
		}

		ref_t lookup(unsigned b) const {
			read_ref rr = tm_.read_lock(ie_.blocknr_, validator_);
			return __lookup_raw(bitmap_data(rr), b);
		}

		void insert(unsigned b, ref_t n) {
			write_ref wr = tm_.shadow(ie_.blocknr_, validator_).first;
			void *bits = bitmap_data(wr);
			bool was_free = !test_bit_le(bits, b * 2) && !test_bit_le(bits, b * 2 + 1);
			if (n == 1 || n == 3)
				set_bit_le(bits, b * 2 + 1);
			else
				clear_bit_le(bits, b * 2 + 1);

			if (n == 2 || n == 3)
				set_bit_le(bits, b * 2);
			else
				clear_bit_le(bits, b * 2);

			ie_.blocknr_ = wr.get_location();

			if (was_free && n > 0) {
				ie_.nr_free_--;
				if (b == ie_.none_free_before_)
					ie_.none_free_before_++;
			}

			if (!was_free && n == 0) {
				ie_.nr_free_++;
				if (b < ie_.none_free_before_)
					ie_.none_free_before_ = b;
			}
		}

		boost::optional<unsigned> find_free(unsigned begin, unsigned end) {
			begin = max(begin, ie_.none_free_before_);
			if (begin >= end)
				return boost::optional<unsigned>();

			read_ref rr = tm_.read_lock(ie_.blocknr_, validator_);
			void const *bits = bitmap_data(rr);

			le64 const *w = reinterpret_cast<le64 const *>(bits);
			le64 const *le64_begin = w + (begin >> 5);    // w + div_down(begin, 32)
			le64 const *le64_end = w + ((end + 31) >> 5); // w + div_up(end, 32)

			for (le64 const *ptr = le64_begin; ptr < le64_end; ptr++) {
				unsigned bit_begin = (ptr == le64_begin) ? (begin & 0x1F) : 0;
				unsigned bit_end = ((ptr == le64_end - 1) && (end & 0x1F)) ? (end & 0x1F) : 32;
				int i;
				if ((i = find_free_bits(ptr, bit_begin, bit_end)) >= 0)
					return ((ptr - w) << 5) + i;
			}

			return boost::optional<unsigned>();
		}

		index_entry const &get_ie() const {
			return ie_;
		}

		// FIXME: duplicates to bitmap::visit()
		// The function name "iterate" is improper. Actually it's the visitor pattern.
		void iterate(block_address offset, block_address hi, space_map::iterator &it) const {
			read_ref rr = tm_.read_lock(ie_.blocknr_, validator_);
			void const *bits = bitmap_data(rr);

			for (unsigned b = 0; b < hi; b++) {
				ref_t b1 = test_bit_le(bits, b * 2);
				ref_t b2 = test_bit_le(bits, b * 2 + 1);
				ref_t result = b2 ? 1 : 0;
				result |= b1 ? 0x2 : 0;
				it(offset + b, result);
			}
		}

		void visit(block_address offset, block_address hi, space_map_detail::visitor &v) const {
			read_ref rr = tm_.read_lock(ie_.blocknr_, validator_);
			void const *bits = bitmap_data(rr);

			for (unsigned b = 0; b < hi; b++) {
				ref_t b1 = test_bit_le(bits, b * 2);
				ref_t b2 = test_bit_le(bits, b * 2 + 1);
				ref_t result = b2 ? 1 : 0;
				result |= b1 ? 0x2 : 0;
				v.visit(offset + b, result);
			}
		}

	private:
		void *bitmap_data(transaction_manager::write_ref &wr) {
			bitmap_header *h = reinterpret_cast<bitmap_header *>(wr.data());
			return h + 1;
		}

		void const *bitmap_data(transaction_manager::read_ref &rr) const {
			bitmap_header const *h = reinterpret_cast<bitmap_header const *>(rr.data());
			return h + 1;
		}

		ref_t __lookup_raw(void const *bits, unsigned b) const {
			ref_t b1 = test_bit_le(bits, b * 2);
			ref_t b2 = test_bit_le(bits, b * 2 + 1);
			ref_t result = b2 ? 1 : 0;
			result |= b1 ? 0x2 : 0;
			return result;
		}

		int find_free_bits(le64 const* bits, unsigned bit_begin, unsigned bit_end) {
			uint64_t v = to_cpu<uint64_t>(*bits);
			v >>= (bit_begin * 2);
			for (; bit_begin < bit_end; bit_begin++) {
				if (!(v & 0x3)) {
					return bit_begin;
				}
				v = v >> 2;
			}
			return -1;
		}

		transaction_manager &tm_;
		bcache::validator::ptr validator_;

		index_entry ie_;
	};

	struct ref_count_traits {
		typedef le32 disk_type;
		typedef uint32_t value_type;
		typedef no_op_ref_counter<uint32_t> ref_counter;

		static void unpack(disk_type const &d, value_type &v) {
			v = to_cpu<value_type>(d);
		}

		static void pack(value_type const &v, disk_type &d) {
			d = to_disk<disk_type>(v);
		}
	};

#if 0
	class ref_count_checker : public btree_checker<1, ref_count_traits> {
	public:
		typedef boost::shared_ptr<ref_count_checker> ptr;

		ref_count_checker(block_counter &counter)
			: btree_checker<1, ref_count_traits>(counter) {
		}
	};
#endif

	class index_entry_visitor {
	public:
		virtual ~index_entry_visitor() {}
		virtual void visit(block_address ie_index, index_entry const &ie) = 0;
		virtual void visit(run<block_address> const &missing) = 0;
	};

	// FIXME: tidy up the classes: index_entry_visitor, index_entry_counter, ...
	class ref_count_visitor: public index_entry_visitor {
	public:
		ref_count_visitor(transaction_manager &tm, space_map_detail::visitor &v, block_address nr_blocks)
			: tm_(tm),
			  bitmap_validator_(new bitmap_block_validator),
			  v_(v),
			  nr_blocks_(nr_blocks),
			  nr_indexes_(div_up<block_address>(nr_blocks_, ENTRIES_PER_BLOCK)) {}

		virtual void visit(block_address ie_index, index_entry const &ie) {
			unsigned hi = (ie_index == nr_indexes_ - 1) ? (nr_blocks_ - ENTRIES_PER_BLOCK * ie_index) : ENTRIES_PER_BLOCK;
			bitmap bm(tm_, ie, bitmap_validator_);
			bm.visit(ie_index * ENTRIES_PER_BLOCK, hi, v_);
		}

		// TODO: allow to visit a damaged index_store
		virtual void visit(run<block_address> const &missing) {
			run<block_address> r(missing.begin_, missing.end_);
			if (r.begin_)
				*r.begin_ *= ENTRIES_PER_BLOCK;
			if (r.end_)
				*r.end_ *= ENTRIES_PER_BLOCK;
			std::ostringstream out;
			out << "missing reference counts for blocks " << r;
			throw std::runtime_error(out.str());
		}

	private:
		transaction_manager &tm_;
		bcache::validator::ptr bitmap_validator_;
		space_map_detail::visitor &v_;
		block_address nr_blocks_;
		block_address nr_indexes_;
	};

	class index_store {
	public:
		typedef boost::shared_ptr<index_store> ptr;

		virtual void count_metadata(block_counter &bc, space_map_detail::damage_visitor &dv) const = 0;
		virtual void resize(block_address nr_indexes) = 0;
		virtual index_entry find_ie(block_address b) const = 0;
		virtual void save_ie(block_address b, struct index_entry ie) = 0;
		virtual void commit_ies() = 0;
		virtual ptr clone() const = 0;
		virtual block_address get_root() const = 0;
		virtual void visit(index_entry_visitor &v) const = 0;
		virtual void destroy() = 0;
	};

	class sm_disk : public checked_space_map {
	public:
		typedef boost::shared_ptr<sm_disk> ptr;
		typedef transaction_manager::read_ref read_ref;
		typedef transaction_manager::write_ref write_ref;

		sm_disk(index_store::ptr indexes,
			transaction_manager &tm)
			: tm_(tm),
			  bitmap_validator_(new bitmap_block_validator),
			  indexes_(indexes),
			  nr_blocks_(0),
			  nr_allocated_(0),
			  ref_counts_(tm_, ref_count_traits::ref_counter()) {
		}

		sm_disk(index_store::ptr indexes,
			transaction_manager &tm,
			sm_root const &root)
			: tm_(tm),
			  bitmap_validator_(new bitmap_block_validator),
			  indexes_(indexes),
			  nr_blocks_(root.nr_blocks_),
			  nr_allocated_(root.nr_allocated_),
			  ref_counts_(tm_, root.ref_count_root_, ref_count_traits::ref_counter()) {
		}

		block_address get_nr_blocks() const {
			return nr_blocks_;
		}

		block_address get_nr_free() const {
			return nr_blocks_ - nr_allocated_;
		}

		ref_t get_count(block_address b) const {
			ref_t count = lookup_bitmap(b);
			if (count == 3)
				return lookup_ref_count(b);

			return count;
		}

		void set_count(block_address b, ref_t c) {
			ref_t old = get_count(b);

			if (c == old)
				return;

			if (c > 2) {
				if (old < 3)
					insert_bitmap(b, 3);
				insert_ref_count(b, c);
			} else {
				if (old > 2)
					remove_ref_count(b);
				insert_bitmap(b, c);
			}

			if (old == 0)
				nr_allocated_++;
			else if (c == 0)
				nr_allocated_--;
		}

		void commit() {
			indexes_->commit_ies();
		}

		void inc(block_address b) {
			// FIXME: 2 get_counts
			ref_t old = get_count(b);
			set_count(b, old + 1);
		}

		void dec(block_address b) {
			ref_t old = get_count(b);
			set_count(b, old - 1);
		}

		// FIXME: keep track of the lowest free block so we
		// can start searching from a suitable place.
		maybe_block find_free(span_iterator &it) {
			for (maybe_span ms = it.first(); ms; ms = it.next()) {
				block_address begin = ms->first;
				block_address end = ms->second;

				block_address begin_index = begin / ENTRIES_PER_BLOCK;
				block_address end_index = div_up<block_address>(end, ENTRIES_PER_BLOCK);

				for (block_address index = begin_index; index < end_index; index++) {
					index_entry ie = indexes_->find_ie(index);

					bitmap bm(tm_, ie, bitmap_validator_);
					unsigned bit_begin = (index == begin_index) ? (begin % ENTRIES_PER_BLOCK) : 0;
					unsigned bit_end = (index == end_index - 1) ?
							   (end - ENTRIES_PER_BLOCK * index) : ENTRIES_PER_BLOCK;

					boost::optional<unsigned> maybe_b = bm.find_free(bit_begin, bit_end);
					if (maybe_b) {
						block_address b = (index * ENTRIES_PER_BLOCK) + *maybe_b;
						return b;
					}
				}
			}

			return maybe_block();
		}

		bool count_possibly_greater_than_one(block_address b) const {
			return get_count(b) > 1;
		}

		virtual void extend(block_address extra_blocks) {
			block_address nr_blocks = nr_blocks_ + extra_blocks;

			block_address bitmap_count = div_up<block_address>(nr_blocks, ENTRIES_PER_BLOCK);
			block_address old_bitmap_count = div_up<block_address>(nr_blocks_, ENTRIES_PER_BLOCK);

			indexes_->resize(bitmap_count);
			for (block_address i = old_bitmap_count; i < bitmap_count; i++) {
				write_ref wr = tm_.new_block(bitmap_validator_);

				index_entry ie;
				ie.blocknr_ = wr.get_location();
				ie.nr_free_ = i == (bitmap_count - 1) ?
					(nr_blocks - ENTRIES_PER_BLOCK * i) : ENTRIES_PER_BLOCK;
				ie.none_free_before_ = 0;

				indexes_->save_ie(i, ie);
			}

			nr_blocks_ = nr_blocks;
		}

		struct look_aside_visitor : public space_map_detail::visitor {
			look_aside_visitor(space_map_detail::visitor &v)
				: v_(v) {
			}

			void visit(block_address b, uint32_t count) {
				if (count < 3)
					v_.visit(b, count);
				// FIXME: implement btree::iterator to obtain
				// ref-counts in the ref_counts_ tree sequentially
			}

			space_map_detail::visitor &v_;
		};

		class ref_count_value_visitor {
		public:
			ref_count_value_visitor(space_map_detail::visitor &v)
				: v_(v) {
			}

			virtual void visit(btree_path const &path, ref_count_traits::value_type c) {
				v_.visit(path.back(), c);
			}

		private:
			space_map_detail::visitor &v_;
		};

		class ref_count_damage_visitor {
		public:
			ref_count_damage_visitor(space_map_detail::damage_visitor &v)
				: v_(v) {
			}

			virtual void visit(btree_path const &path, btree_detail::damage const &d) {
				v_.visit(space_map_detail::missing_counts(d.desc_, 0, d.lost_keys_));
			}

		private:
			space_map_detail::damage_visitor &v_;
		};

		virtual void visit(space_map_detail::visitor &v,
				   space_map_detail::damage_visitor &dv) const {
			look_aside_visitor wrapper(v);
			ref_count_visitor rv(tm_, wrapper, nr_blocks_);
			indexes_->visit(rv);

			// FIXME: repalce by btree::iterator
			ref_count_value_visitor vv(v);
			ref_count_damage_visitor dv_(dv);
			btree_visit_values(ref_counts_, vv, dv_);
		}

		struct look_aside_iterator : public iterator {
			look_aside_iterator(sm_disk const &smd, iterator &it)
				: smd_(smd),
				  it_(it) {
			}

			virtual void operator () (block_address b, ref_t c) {
				it_(b, c == 3 ? smd_.lookup_ref_count(b) : c);
			}

			sm_disk const &smd_;
			iterator &it_;
		};

		friend struct look_aside_iterator;

		virtual void iterate(iterator &it) const {
			look_aside_iterator wrapper(*this, it);
			unsigned nr_indexes = div_up<block_address>(nr_blocks_, ENTRIES_PER_BLOCK);

			for (unsigned i = 0; i < nr_indexes; i++) {
				unsigned hi = (i == nr_indexes - 1) ? (nr_blocks_ - ENTRIES_PER_BLOCK * i) : ENTRIES_PER_BLOCK;
				index_entry ie = indexes_->find_ie(i);
				bitmap bm(tm_, ie, bitmap_validator_);
				bm.iterate(i * ENTRIES_PER_BLOCK, hi, wrapper);
			}
		}

		virtual void no_lookaside_iterate(iterator &it) const {
			unsigned nr_indexes = div_up<block_address>(nr_blocks_, ENTRIES_PER_BLOCK);

			for (unsigned i = 0; i < nr_indexes; i++) {
				index_entry ie = indexes_->find_ie(i);
				/* skip unused index entries */
				if (ie.nr_free_ != ENTRIES_PER_BLOCK) {
					unsigned hi = (i == nr_indexes - 1) ? (nr_blocks_ - ENTRIES_PER_BLOCK * i) : ENTRIES_PER_BLOCK;
					bitmap bm(tm_, ie, bitmap_validator_);
					bm.iterate(i * ENTRIES_PER_BLOCK, hi, it);
				}
			}
		}

		virtual void count_metadata(block_counter &bc, space_map_detail::damage_visitor &dv) const {
			indexes_->count_metadata(bc, dv);

			noop_value_counter<uint32_t> vc;
			count_btree_blocks(ref_counts_, bc, vc);
		}

		virtual size_t root_size() const {
			return sizeof(sm_root_disk);
		}

		virtual void copy_root(void *dest, size_t len) const {
			sm_root_disk d;
			sm_root v;

			if (len < sizeof(d))
				throw runtime_error("root too small");

			v.nr_blocks_ = sm_disk::get_nr_blocks();
			v.nr_allocated_ = sm_disk::get_nr_allocated();
			v.bitmap_root_ = get_index_store()->get_root();
			v.ref_count_root_ = sm_disk::get_ref_count_root();

			sm_root_traits::pack(v, d);
			::memcpy(dest, &d, sizeof(d));
		}

		virtual checked_space_map::ptr clone() const {
			sm_root root;
			root.nr_blocks_ = nr_blocks_;
			root.nr_allocated_ = nr_allocated_;
			root.bitmap_root_ = indexes_->get_root();
			root.ref_count_root_ = ref_counts_.get_root();
			return checked_space_map::ptr(
				new sm_disk(indexes_->clone(), tm_, root));
		}

		virtual void destroy() {
			indexes_->destroy();
			ref_counts_.destroy();
		}

	protected:
		transaction_manager &get_tm() const {
			return tm_;
		}

		block_address get_nr_allocated() const {
			return nr_allocated_;
		}

		block_address get_ref_count_root() const {
			return ref_counts_.get_root();
		}

		index_store::ptr get_index_store() const {
			return indexes_;
		}

	private:
		void check_block(block_address b) const {
			if (b >= nr_blocks_) {
				std::ostringstream out;
				out << "space map disk: block out of bounds ("
				    << b << " >= " << nr_blocks_ << ")";
				throw std::runtime_error(out.str());
			}
		}

		ref_t lookup_bitmap(block_address b) const {
			check_block(b);

			index_entry ie = indexes_->find_ie(b / ENTRIES_PER_BLOCK);
			bitmap bm(tm_, ie, bitmap_validator_);
			return bm.lookup(b % ENTRIES_PER_BLOCK);
		}

		void insert_bitmap(block_address b, unsigned n) {
			check_block(b);

			if (n > 3)
				throw runtime_error("bitmap can only hold 2 bit values");

			index_entry ie = indexes_->find_ie(b / ENTRIES_PER_BLOCK);
			bitmap bm(tm_, ie, bitmap_validator_);
			bm.insert(b % ENTRIES_PER_BLOCK, n);
			indexes_->save_ie(b / ENTRIES_PER_BLOCK, bm.get_ie());
		}

		ref_t lookup_ref_count(block_address b) const {
			uint64_t key[1] = {b};
			boost::optional<ref_t> mvalue = ref_counts_.lookup(key);
			if (!mvalue)
				throw runtime_error("ref count not in tree");
			return *mvalue;
		}

		void insert_ref_count(block_address b, ref_t count) {
			uint64_t key[1] = {b};
			ref_counts_.insert(key, count);
		}

		void remove_ref_count(block_address b) {
			uint64_t key[1] = {b};
			ref_counts_.remove(key);
		}

		transaction_manager &tm_;
		bcache::validator::ptr bitmap_validator_;
		index_store::ptr indexes_;
		block_address nr_blocks_;
		block_address nr_allocated_;

		btree<1, ref_count_traits> ref_counts_;
	};

	//--------------------------------

	class ie_value_visitor {
	public:
		ie_value_visitor(index_entry_visitor &v)
			: v_(v) {
		}

		virtual void visit(btree_path const &path, sm_disk_detail::index_entry const &ie) {
			v_.visit(path.back(), ie);
		}

	private:
		index_entry_visitor &v_;
	};

	class ie_damage_visitor {
	public:
		ie_damage_visitor(index_entry_visitor &v)
			: v_(v) {
		}

		virtual void visit(btree_path const &path, btree_detail::damage const &d) {
			v_.visit(d.lost_keys_);
		}

	private:
		index_entry_visitor &v_;
	};

	class btree_index_store : public index_store {
	public:
		typedef boost::shared_ptr<btree_index_store> ptr;

		btree_index_store(transaction_manager &tm)
			: tm_(tm),
			  bitmaps_(tm, index_entry_traits::ref_counter(tm.get_sm())) {
		}

		btree_index_store(transaction_manager &tm,
				  block_address root)
			: tm_(tm),
			  bitmaps_(tm, root, index_entry_traits::ref_counter(tm.get_sm())) {
		}

		//--------------------------------

		struct index_entry_counter {
			index_entry_counter(transaction_manager &tm,
					    block_counter &bc,
					    space_map_detail::damage_visitor &dv)
			: tm_(tm), bc_(bc), dv_(dv),
			  bitmap_validator_(new bitmap_block_validator) {
			}

			void visit(btree_detail::node_location const &loc, index_entry const &ie) {
				if (ie.blocknr_ != 0) {
					try {
						block_manager<>::read_ref rr = tm_.read_lock(ie.blocknr_, bitmap_validator_);
						if (rr.data())
							bc_.inc(ie.blocknr_);
					} catch (std::exception &e) {
						dv_.visit(space_map_detail::missing_counts(e.what(), dv_.get_id(),
							base::run<block_address>(loc.path.back() * ENTRIES_PER_BLOCK,
										 (loc.path.back() + 1) * ENTRIES_PER_BLOCK)));
					}
				}
			}

		private:
			transaction_manager &tm_;
			block_counter &bc_;
			space_map_detail::damage_visitor &dv_;
			bcache::validator::ptr bitmap_validator_;
		};

		virtual void count_metadata(block_counter &bc, space_map_detail::damage_visitor &dv) const {
			index_entry_counter vc(tm_, bc, dv);
			count_btree_blocks(bitmaps_, bc, vc);
		}

		//--------------------------------

		virtual void resize(block_address nr_entries) {
			// No op
		}

		virtual index_entry find_ie(block_address ie_index) const {
			uint64_t key[1] = {ie_index};
			boost::optional<index_entry> mindex = bitmaps_.lookup(key);
			if (!mindex)
				throw runtime_error("Couldn't lookup bitmap");

			return *mindex;
		}

		virtual void save_ie(block_address ie_index, struct index_entry ie) {
			uint64_t key[1] = {ie_index};
			bitmaps_.insert(key, ie);
		}

		virtual void commit_ies() {
			// No op
		}

		virtual index_store::ptr clone() const {
			return index_store::ptr(new btree_index_store(tm_, bitmaps_.get_root()));
		}

		virtual block_address get_root() const {
			return bitmaps_.get_root();
		}

		virtual void visit(index_entry_visitor &v) const {
			ie_value_visitor vv(v);
			ie_damage_visitor dv(v);
			btree_visit_values(bitmaps_, vv, dv);
		}

		virtual void destroy() {
			bitmaps_.destroy();
		}

	private:
		transaction_manager &tm_;
		btree<1, index_entry_traits> bitmaps_;
	};

	class metadata_index_store : public index_store {
	public:
		typedef boost::shared_ptr<metadata_index_store> ptr;

		metadata_index_store(transaction_manager &tm)
			: tm_(tm),
			  bitmap_validator_(new bitmap_block_validator) {
			block_manager<>::write_ref wr = tm_.new_block(index_validator());
			bitmap_root_ = wr.get_location();
		}

		metadata_index_store(transaction_manager &tm, block_address root, block_address nr_indexes)
			: tm_(tm),
			  bitmap_root_(root),
			  bitmap_validator_(new bitmap_block_validator) {
			resize(nr_indexes);
			load_ies();
		}

		virtual void count_metadata(block_counter &bc, space_map_detail::damage_visitor &dv) const {
			bc.inc(bitmap_root_);

			for (unsigned i = 0; i < entries_.size(); i++) {
				block_address b = entries_[i].blocknr_;

				if (b != 0) {
					try {
						block_manager<>::read_ref rr = tm_.read_lock(b, bitmap_validator_);
						if (rr.data())
							bc.inc(b);
					} catch (std::exception &e) {
						dv.visit(space_map_detail::missing_counts(e.what(), dv.get_id(),
							base::run<block_address>(i * ENTRIES_PER_BLOCK,
										 (i + 1) * ENTRIES_PER_BLOCK)));
					}
				}
			}
		}

		virtual void resize(block_address nr_indexes) {
			if (nr_indexes > MAX_METADATA_BITMAPS)
				throw runtime_error("metadata bitmap count exceeds limit");

			entries_.resize(nr_indexes);
		}

		virtual index_entry find_ie(block_address ie_index) const {
			return entries_[ie_index];
		}

		virtual void save_ie(block_address ie_index, struct index_entry ie) {
			entries_[ie_index] = ie;
		}

		virtual void commit_ies() {
			std::pair<block_manager<>::write_ref, bool> p =
				tm_.shadow(bitmap_root_, index_validator());

			bitmap_root_ = p.first.get_location();
			metadata_index *mdi = reinterpret_cast<metadata_index *>(p.first.data());

			for (unsigned i = 0; i < entries_.size(); i++)
				index_entry_traits::pack(entries_[i], mdi->index[i]);
		}

		virtual index_store::ptr clone() const {
			return index_store::ptr(new metadata_index_store(tm_, bitmap_root_, entries_.size()));
		}

		virtual block_address get_root() const {
			return bitmap_root_;
		}

		virtual void visit(index_entry_visitor &v) const {
			for (unsigned i = 0; i < entries_.size(); i++)
				v.visit(i, entries_[i]);
		}

		virtual void destroy() {
			throw runtime_error("not implemented");
		}

	private:
		void load_ies() {
			block_manager<>::read_ref rr =
				tm_.read_lock(bitmap_root_, index_validator());

			metadata_index const *mdi = reinterpret_cast<metadata_index const *>(rr.data());
			for (unsigned i = 0; i < entries_.size(); i++)
				index_entry_traits::unpack(*(mdi->index + i), entries_[i]);
		}

		transaction_manager &tm_;
		block_address bitmap_root_;
		std::vector<index_entry> entries_;
		bcache::validator::ptr bitmap_validator_;
	};
}

//----------------------------------------------------------------

checked_space_map::ptr
persistent_data::create_disk_sm(transaction_manager &tm,
				block_address nr_blocks)
{
	index_store::ptr store(new btree_index_store(tm));
	checked_space_map::ptr sm(new sm_disk(store, tm));
	sm->extend(nr_blocks);
	sm->commit();
	return sm;
}

checked_space_map::ptr
persistent_data::open_disk_sm(transaction_manager &tm, void const *root)
{
	sm_root_disk d;
	sm_root v;

	::memcpy(&d, root, sizeof(d));
	sm_root_traits::unpack(d, v);
	index_store::ptr store(new btree_index_store(tm, v.bitmap_root_));
	return checked_space_map::ptr(new sm_disk(store, tm, v));
}

// allow creating zero-sized space maps
std::vector<checked_space_map::ptr>
persistent_data::create_multiple_disk_sm(transaction_manager &tm,
					 vector<block_address> const& nr_blocks)
{
	std::vector<checked_space_map::ptr> data_sm(nr_blocks.size());
	for (size_t i = 0; i < nr_blocks.size(); ++i)
		data_sm[i] = create_disk_sm(tm, nr_blocks[i]);
	return data_sm;
}

// allow opening zero-sized space maps
std::vector<checked_space_map::ptr>
persistent_data::open_multiple_disk_sm(transaction_manager &tm,
				       vector<void const*> const& roots)
{
	std::vector<checked_space_map::ptr> data_sm(roots.size());
	for (size_t i = 0; i < roots.size(); ++i) {
		sm_root v;
		sm_root_traits::unpack(*reinterpret_cast<sm_root_disk const*>(roots[i]), v);
		checked_space_map::ptr sm = open_disk_sm(tm, roots[i]);
		data_sm[i] = sm;
	}
	return data_sm;
}

checked_space_map::ptr
persistent_data::create_metadata_sm(transaction_manager &tm, block_address nr_blocks)
{
	index_store::ptr store(new metadata_index_store(tm));
	checked_space_map::ptr sm(new sm_disk(store, tm));
	sm->extend(nr_blocks);
	sm->commit();
	return create_careful_alloc_sm(
		create_recursive_sm(sm));
}

checked_space_map::ptr
persistent_data::open_metadata_sm(transaction_manager &tm, void const *root)
{
	sm_root_disk d;
	sm_root v;

	::memcpy(&d, root, sizeof(d));
	sm_root_traits::unpack(d, v);
	block_address nr_indexes = div_up<block_address>(v.nr_blocks_, ENTRIES_PER_BLOCK);
	index_store::ptr store(new metadata_index_store(tm, v.bitmap_root_, nr_indexes));
	return create_careful_alloc_sm(
		create_recursive_sm(
			checked_space_map::ptr(new sm_disk(store, tm, v))));
}

bcache::validator::ptr
persistent_data::bitmap_validator() {
	return bcache::validator::ptr(new bitmap_block_validator());
}

bcache::validator::ptr
persistent_data::index_validator() {
	return bcache::validator::ptr(new index_block_validator());
}

//----------------------------------------------------------------

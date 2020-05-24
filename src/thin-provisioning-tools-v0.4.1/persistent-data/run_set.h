#ifndef PERSISTENT_DATA_H
#define PERSISTENT_DATA_H

#include "persistent-data/run.h"

#include <algorithm>
#include <set>

//----------------------------------------------------------------

namespace base {
	template <typename T>
	class run_set {
	public:
		void clear() {
			runs_.clear();
		}

		void add(T const &b) {
			add(run<T>(b, b + 1));
		}

		void add(T const &b, T const &e) {
			add(run<T>(b, e));
		}

		void add(run<T> const &r_) {
			run<T> r(r_);

			if (runs_.size()) {
				// Skip all blocks that end before r
				const_iterator it = runs_.lower_bound(r);
				if (it != runs_.begin())
					--it;

				while (it != runs_.end() && it->end_ < r.begin_)
					++it;

				// work out which runs overlap
				if (it != runs_.end()) {
					r.begin_ = min_maybe(it->begin_, r.begin_);
					const_iterator first = it;
					while (it != runs_.end() && it->begin_ <= r.end_) {
						r.end_ = max_maybe(it->end_, r.end_);
						++it;
					}

					// remove overlapping runs
					runs_.erase(first, it);
				}
			}

			runs_.insert(r);
		}

		void merge(run_set<T> const &rhs) {
			for (const_iterator it = rhs.begin(); it != rhs.end(); ++it)
				add(*it);
		}

		bool member(T const &v) const {
			if (!runs_.size())
				return false;

			typename rset::const_iterator it = runs_.lower_bound(run<T>(v));

			if (it != runs_.end() && it->begin_ == v)
				return true;

			if (it != runs_.begin()) {
				it--;
				return it->contains(v);
			}

			return false;
		}

		struct compare_begin {
			bool operator ()(run<T> const &r1, run<T> const &r2) const {
				return r1.begin_ < r2.begin_;
			}
		};

		typedef std::set<run<T>, compare_begin> rset;
		typedef typename rset::const_iterator const_iterator;

		const_iterator begin() const {
			return runs_.begin();
		}

		const_iterator end() const {
			return runs_.end();
		}

		void negate() {
			rset replacement;

			if (runs_.begin() == runs_.end())
				replacement.insert(run<T>());
			else {
				typename rset::const_iterator b = runs_.begin();

				// Some versions of gcc give a spurious warning here.
				maybe last = b->end_;

				if (b->begin_)
					replacement.insert(run<T>(maybe(), *(b->begin_)));

				++b;
				while (b != runs_.end()) {
					replacement.insert(run<T>(last, b->begin_));
					last = b->end_;
					++b;
				}

				if (last)
					replacement.insert(run<T>(last, maybe()));

			}

			runs_ = replacement;
		}

		void truncate(T const &e) {
			truncate_end(e);
		}

		void truncate(T const &b, T const &e) {
			truncate_begin(b);
			truncate_end(e);
		}

	private:
		typedef typename run<T>::maybe maybe;

		static maybe min_maybe(maybe const &m1, maybe const &m2) {
			if (!m1 || !m2)
				return maybe();

			return maybe(std::min<T>(*m1, *m2));
		}

		static maybe max_maybe(maybe const &m1, maybe const &m2) {
			if (!m1 || !m2)
				return maybe();

			return maybe(std::max<T>(*m1, *m2));
		}

		// the truncated run_set contains b
		void truncate_begin(T const &b) {
			typename rset::iterator first;
			while (runs_.size()) {
				first = runs_.begin();
				// a range ends with boost::none should not be erased
				if (!first->end_ || first->end_ > b)
					break;
				runs_.erase(first);
			}

			if (!runs_.size())
				return;

			first = runs_.begin();
			if (!first->begin_ || first->begin_ < b) {
				run<T> r(b, first->end_);
				runs_.erase(first);
				runs_.insert(r);
			}
		}

		// the truncated run_set does not contain e
		void truncate_end(T const &e) {
			typename rset::iterator last;
			while (runs_.size()) {
				last = runs_.end();
				--last;
				if (last->begin_ < e)
					break;
				runs_.erase(last);
			}

			if (!runs_.size())
				return;

			last = runs_.end();
			--last;
			if (!last->end_ || last->end_ > e) {
				run<T> r(last->begin_, e);
				runs_.erase(last);
				runs_.insert(r);
			}
		}

		rset runs_;
	};
}

//----------------------------------------------------------------

#endif

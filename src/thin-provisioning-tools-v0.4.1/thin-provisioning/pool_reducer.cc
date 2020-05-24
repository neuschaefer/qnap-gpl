#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/tokenizer.hpp>

#include "thin-provisioning/pool_reducer.h"
#include "persistent-data/math_utils.h"

//----------------------------------------------------------------

namespace {
	using namespace thin_provisioning;
	using namespace mapping_tree_detail;

	void verify_arguments(metadata::ptr md, tier_span_list const &pba_reduction) {
		uint32_t nr_tiers = md->sb_.tier_num_;
		if (md->sb_.version_ < 4 || !nr_tiers)
			throw std::runtime_error("unsupported metadata version");

		uint32_t &data_block_size = md->sb_.data_block_size_;
		uint32_t &tier_block_size = md->sb_.tier_block_size_;
		if (!tier_block_size ||
		    (tier_block_size & (data_block_size - 1)))
			throw std::runtime_error("tier block size is not a multiple of data block size");

		uint32_t multiplier = tier_block_size / data_block_size;
		if (multiplier & (multiplier - 1))
			throw std::runtime_error("tier block multipler is not a power of two");

		boost::optional<uint32_t> last_tier = pba_reduction.last_tier();
		if (!last_tier || last_tier >= md->sb_.tier_num_) {
			std::stringstream ss;
			ss << "invalid tier index " << last_tier;
			throw std::runtime_error(ss.str());
		}

		for (uint32_t i = 0; i <= last_tier; i++) {
			block_address nr_blocks = md->tier_data_sm_[i]->get_nr_blocks();
			tier_span_list::const_iterator it = pba_reduction.lower_bound(i);
			tier_span_list::const_iterator list_end = pba_reduction.upper_bound(i);
			for (; it != list_end; ++it) {
				if (it->begin_ >= nr_blocks || it->end_ > nr_blocks) {
					std::stringstream ss;
					ss << "invalid block range [" << *it->begin_ << "," << *it->end_
					   << ") for tier " << i;
					throw std::runtime_error(ss.str());
				}
			}
		}
	}

	class metadata_progress_monitor {
	public:
		// use div_up() for conservative accounting
		metadata_progress_monitor(block_address nr_blocks,
					  base::progress_monitor &mon)
			: nr_blocks_(nr_blocks),
			  step_(base::div_up<block_address>(nr_blocks_, 100)),
			  progress_(0),
			  monitor_(mon) {
			monitor_.update_percent(0);
		}

		void advance() {
			if (progress_ < 100 && !--step_) {
				step_ = base::div_up<block_address>(nr_blocks_, 100);
				monitor_.update_percent(++progress_);
			}
		}

		void end_monitoring() {
			progress_ = 100;
			step_ = base::div_up<block_address>(nr_blocks_, 100);
			monitor_.update_percent(100);
			monitor_.stop_monitoring();
		}

	private:
		block_address nr_blocks_;
		block_address step_;
		unsigned progress_;
		base::progress_monitor &monitor_;
	};

	// TODO:
	// 1. disallow set_new_pool_size() if we had added any used_lba
	// 2. stop building remappings if there's no enough free block
	class tier_remappings {
		typedef std::map<block_address, std::pair<block_address, block_address> > map_type;

	public:
		tier_remappings(metadata::ptr md, tier_span_list const &pba_reduction)
			: md_(md),
			  range_initialized_(false), range_begin_(0), range_end_(0),
			  origin_initialized_(false), origin_begin_(0), origin_end_(0),
			  new_pool_size_(0) {
			new_tier_size_ = calculate_new_tier_size(md, pba_reduction);
			for (unsigned i = 0; i < new_tier_size_.size(); i++)
				new_pool_size_ += new_tier_size_[i] - calculate_tier_swaps(new_tier_size_[i]);

			uint32_t multiplier = md->sb_.tier_block_size_ / md->sb_.data_block_size_;
			tier_block_shift_ = ffs(multiplier) - 1;
		}

		void create_remappings() {
			space_map_iterator iter(new_pool_size_, tier_block_shift_, *this);
			md_->data_sm_->no_lookaside_iterate(iter);
			end_iteration();

			tiering_tree_visitor visitor(new_pool_size_, tier_block_shift_, *this);
			md_->tier_tree_->visit_depth_first(visitor);
			end_iteration();

			free_lba_.negate();
			free_lba_.truncate(0, new_pool_size_);

			__create_remappings();
		}

		void dump_remappings() {
			cout << "Origin LBA" << endl;
			for (run_set<block_address>::const_iterator it = origin_lba_.begin(); it != origin_lba_.end(); ++it)
				cout << *it << endl;
			cout << endl;

			cout << "Free LBA" << endl;
			for (run_set<block_address>::const_iterator it = free_lba_.begin(); it != free_lba_.end(); ++it)
				cout << *it << endl;
			cout << endl;

			cout << "Remapped LBA" << endl;
			for (map_type::const_iterator it = remapped_lba_.begin(); it != remapped_lba_.end(); ++it)
				cout << it->first << " (" << it->second.first << "," << it->second.second << ")" << endl;
			cout << endl;
		}

		// Here we borrow the free_lba_ to store used LBA
		// The free_lba_ then will be negated in the end
		void add_used_lba(block_address tier_block) {
			if (!range_initialized_) {
				range_initialized_ = true;
				range_begin_ = tier_block;
				range_end_ = tier_block + 1;
			} else if (tier_block > range_end_) {
				free_lba_.add(range_begin_, range_end_);
				range_begin_ = tier_block;
				range_end_ = tier_block + 1;
			} else if (tier_block == range_end_)
				++range_end_;
		}

		void add_origin_lba(block_address tier_block) {
			if (!origin_initialized_) {
				origin_initialized_ = true;
				origin_begin_ = tier_block;
				origin_end_ = tier_block + 1;
			} else if (tier_block > origin_end_) {
				origin_lba_.add(origin_begin_, origin_end_);
				origin_begin_ = tier_block;
				origin_end_ = tier_block + 1;
			} else if (tier_block == origin_end_)
				++origin_end_;
		}

		void end_iteration() {
			if (range_initialized_) {
				free_lba_.add(range_begin_, range_end_);
				range_initialized_ = false;
			}
			if (origin_initialized_) {
				origin_lba_.add(origin_begin_, origin_end_);
				origin_initialized_ = false;
			}
		}

		bool contains(block_address origin_lba) const {
			return origin_lba >= origin_begin_ && origin_lba < origin_end_;
		}

		block_address get_remapped_lba(block_address origin_lba) const {
			map_type::const_iterator iter = remapped_lba_.lower_bound(origin_lba);
			if (iter == remapped_lba_.end())
				--iter; // for a non-empty ordered set, --iter must be less than the given key
			else if (iter->first > origin_lba) {
				if (iter == remapped_lba_.begin())
					throw std::runtime_error("remappings not found");
				--iter;
			}

			block_address offset = origin_lba - iter->first;
			if (offset > iter->second.second) // key not in range
				throw std::runtime_error("remappings not found");

			return iter->second.first + offset;
		}

		map_type::const_iterator begin() const {
			return remapped_lba_.begin();
		}

		map_type::const_iterator end() const {
			return remapped_lba_.end();
		}

		block_address get_new_pool_size() const {
			return new_pool_size_;
		}

		std::vector<block_address> const &get_new_tier_size() const {
			return new_tier_size_;
		}

	private:
		class space_map_iterator: public space_map::iterator {
		public:
			space_map_iterator(block_address new_pool_size, uint64_t tier_block_shift, tier_remappings &rmap)
				: new_pool_size_(new_pool_size),
				  tier_block_shift_(tier_block_shift),
				  remappings_(rmap) {
			}

			void operator()(block_address b, uint32_t count) {
				// Skip unused block
				if (!count)
					return;

				block_address tier_block = b >> tier_block_shift_;
				if (tier_block < new_pool_size_)
					remappings_.add_used_lba(tier_block);
				else
					remappings_.add_origin_lba(tier_block);
			}

		private:
			block_address new_pool_size_;
			uint64_t tier_block_shift_;
			tier_remappings &remappings_;
		};

		class tiering_tree_visitor: public thin_provisioning::tiering_tree::visitor {
		public:
			tiering_tree_visitor(block_address new_pool_size, uint64_t tier_block_shift, tier_remappings &rmap)
				: new_pool_size_(new_pool_size),
				  tier_block_shift_(tier_block_shift),
				  remappings_(rmap) {
			}

			bool visit_internal(node_location const &loc,
					    tiering_tree::internal_node const &n) {
				return true;
			}

			bool visit_internal_leaf(node_location const &loc,
						 tiering_tree::internal_node const &n) {
				return true;
			}

			bool visit_leaf(node_location const &l,
					tiering_tree::leaf_node const &n) {
				unsigned nr_entries = n.get_nr_entries();

				for (unsigned i = 0; i < nr_entries; i++) {
					uint64_t key = n.key_at(i);
					if (key < new_pool_size_)
						remappings_.add_used_lba(key);
					else
						remappings_.add_origin_lba(key);
				}

				return true;
			}

			void visit_complete() {
			}

		private:
			block_address new_pool_size_;
			uint64_t tier_block_shift_;
			tier_remappings &remappings_; // in tiering blocks
		};

		void __create_remappings() {
			run_set<block_address>::const_iterator iter1 = origin_lba_.begin();
			run_set<block_address>::const_iterator iter2 = free_lba_.begin();

			block_address origin_begin;
			block_address origin_len = 0;
			block_address free_begin;
			block_address free_len = 0;

			if (iter1 != origin_lba_.end()) {
				origin_begin = *iter1->begin_;
				origin_len = *iter1->end_ - origin_begin;
			}
			if (iter2 != free_lba_.end()) {
				free_begin = *iter2->begin_;
				free_len = *iter2->end_ - free_begin;
			}

			while (origin_len && free_len) {
				block_address remapped_len = std::min(origin_len, free_len);
				remapped_lba_.insert(std::make_pair(origin_begin, std::make_pair(free_begin, remapped_len)));

				origin_begin += remapped_len;
				origin_len -= remapped_len;
				free_begin += remapped_len;
				free_len -= remapped_len;

				if (!origin_len && ++iter1 != origin_lba_.end()) {
					origin_begin = *iter1->begin_;
					origin_len = *iter1->end_ - origin_begin;
				}

				if (!free_len && ++iter2 != free_lba_.end()) {
					free_begin = *iter2->begin_;
					free_len = *iter2->end_ - free_begin;
				}
			}

			if (iter1 != origin_lba_.end())
				throw std::runtime_error("insufficient number of free blocks");

			if (remapped_lba_.size()) {
				origin_begin_ = remapped_lba_.begin()->first;
				origin_end_ = remapped_lba_.rbegin()->first + remapped_lba_.rbegin()->second.second;
			} else
				origin_begin_ = origin_end_ = 0;
		}

		std::vector<block_address> calculate_new_tier_size(metadata::ptr md, tier_span_list const &pba_reduction) {
			uint32_t nr_tiers = md->sb_.tier_num_;
			vector<block_address> s(nr_tiers);
			for (uint32_t i = 0; i < nr_tiers; i++) {
				s[i] = md->tier_data_sm_[i]->get_nr_blocks();

				tier_span_list::const_iterator it = pba_reduction.lower_bound(i);
				tier_span_list::const_iterator list_end = pba_reduction.upper_bound(i);
				for (; it != list_end; ++it)
					s[i] -= (*it->end_ - *it->begin_);
			}

			return s;
		}

		metadata::ptr md_;
		map_type remapped_lba_;

		bool range_initialized_;
		block_address range_begin_;
		block_address range_end_;
		bool origin_initialized_;
		block_address origin_begin_;
		block_address origin_end_;

		run_set<block_address> origin_lba_;
		run_set<block_address> free_lba_;

		block_address new_pool_size_;
		std::vector<block_address> new_tier_size_;
		block_address tier_block_shift_;
	};

	// Remap tiering mappings, and also resize tiering space maps
	//
	// Expected result:
	// 1. the key range of the new tiering tree is reduced
	// 2. the tiering space maps are reconstructed and resized accordinately
	class tiering_remapper: public thin_provisioning::tiering_tree::visitor {
	public:
		tiering_remapper(metadata::ptr md,
				 tier_remappings const &remappings,
				 tier_span_list const &pba_reduction,
				 metadata_progress_monitor &mon)
			: md_(md),
			  remappings_(remappings),
			  pba_reduction_(pba_reduction),
			  monitor_(mon) {

			// create the new tiering tree and the corresponding space maps
			// FIXME: implement btree.remove() to avoid migrating all the key-values
			// to the new tiering tree
			// FIXME: implement btree_index_store::resize() to avoid rebuilding the
			// tiering space maps
			std::vector<block_address> const &new_tier_size = remappings.get_new_tier_size();
			tier_data_sm_ = create_multiple_disk_sm(*md->tm_, new_tier_size);
			tier_mappings_ = tiering_tree::ptr(new tiering_tree(*md->tm_,
				mapping_tree_detail::tier_block_ref_counter(tier_data_sm_)));
		}

		bool visit_internal(node_location const &loc,
				    tiering_tree::internal_node const &n) {
			monitor_.advance();
			return true;
		}

		bool visit_internal_leaf(node_location const &loc,
					 tiering_tree::internal_node const &n) {
			monitor_.advance();
			return true;
		}

		bool visit_leaf(node_location const &l,
				tiering_tree::leaf_node const &n) {
			unsigned nr_entries = n.get_nr_entries();

			for (unsigned i = 0; i < nr_entries; i++) {
				uint64_t key = n.key_at(i);
				tier_block tb = n.value_at(i);

				// remap the LBA located beyond the new pool size to a free LBA
				if (remappings_.contains(key))
					key = remappings_.get_remapped_lba(key);

				// shift the PBA
				tb.block_ -= get_pba_shift(tb);

				// FIXME: implement btree.remove() to avoid migrating
				// all the key-values to the new tiering tree
				uint64_t k[1] = {key};
				tier_mappings_->insert(k, tb);
				tier_data_sm_[tb.tier_]->inc(tb.block_);
			}

			monitor_.advance();

			return true;
		}

		void visit_complete() {
			// TODO: evaluate whether metadata fragmentation caused by
			//       block releasing impacts further free block searching
			//       performance or not.

			// destroy the original tiering tree and the corresponding space maps
			md_->tier_tree_->destroy();
			std::vector<checked_space_map::ptr> &old_sm = md_->tier_data_sm_;
			old_sm[0]->destroy();
			old_sm[1]->destroy();
			old_sm[2]->destroy();

			// switch to the new one
			md_->tier_tree_ = tier_mappings_->clone();
			md_->tm_->get_sm()->dec(tier_mappings_->get_root());
			md_->tier_data_sm_ = tier_data_sm_;
		}

	private:


		// Returns the number of blocks to reduce before the given PBA,
		// i.e., the offset to the remapped PBA.
		block_address get_pba_shift(mapping_tree_detail::tier_block const &tb) {
			block_address shift = 0;
			tier_span_list::const_iterator it = pba_reduction_.lower_bound(tb.tier_);
			tier_span_list::const_iterator list_end = pba_reduction_.upper_bound(tb.tier_);
			for (; it != list_end; ++it) {
				if (tb.block_ >= it->end_)
					shift += *it->end_ - *it->begin_;
				// error checking
				else if (tb.block_ >= it->begin_) {
					std::stringstream ss;
					ss << "block " << tb.block_ << " of tier " << tb.tier_ << " has data";
					throw std::runtime_error(ss.str());
				} else
					break;
			}

			return shift;
		}

		metadata::ptr md_;
		tier_remappings const &remappings_; // in tiering blocks
		tier_span_list const &pba_reduction_;

		// internal use only
		metadata_progress_monitor &monitor_;

		// rebuilt mappings
		tiering_tree::ptr tier_mappings_;
		std::vector<checked_space_map::ptr> tier_data_sm_;
	};

	// Adjust the mapped values of the data mapping tree according to tiering remappings
	class thin_remapper: public thin_provisioning::mapping_tree::visitor {
	public:
		thin_remapper(metadata::ptr md,
			      tier_remappings const &rmap,
			      metadata_progress_monitor &mon)
			: bm_(md->tm_->get_bm()),
			  remappings_(rmap),
			  monitor_(mon),
			  validator_(new btree_node_validator()) {
			uint32_t multiplier = md->sb_.tier_block_size_ / md->sb_.data_block_size_;
			tier_block_shift_ = ffs(multiplier) - 1;
			tier_block_mask_ = multiplier - 1;
		}

		bool visit_internal(node_location const &loc,
				    mapping_tree::internal_node const &n) {
			if (already_visited(n))
				return false;
			monitor_.advance();
			return true;
		}

		bool visit_internal_leaf(node_location const &loc,
					 mapping_tree::internal_node const &n) {
			if (already_visited(n))
				return false;
			monitor_.advance();
			return true;
		}

		bool visit_leaf(node_location const &l,
				mapping_tree::leaf_node const &n) {
			if (already_visited(n))
				return false;

			if (previous_leaf_) {
				block_manager<>::write_ref blk = bm_->write_lock(*previous_leaf_, validator_);
				mapping_tree::leaf_node n2 = to_node<mapping_tree_detail::block_traits>(blk);
				remap(n2);
				previous_leaf_ = boost::none;
			}

			if (need_remapping(n))
				previous_leaf_ = n.get_block_nr();

			monitor_.advance();

			return true;
		}

		void visit_complete() {
			if (previous_leaf_) {
				block_manager<>::write_ref blk = bm_->write_lock(*previous_leaf_, validator_);
				mapping_tree::leaf_node n2 = to_node<mapping_tree_detail::block_traits>(blk);
				remap(n2);
				previous_leaf_ = boost::none;
			}
		}

	private:
		template <typename node>
		bool already_visited(node const &n) {
			block_address b = n.get_location();

			if (visited_.member(b))
				return true;

			visited_.add(b);
			return false;
		}

		// Remap the thin data mappings according to tiering mappings
		// NOTE: Do NOT modify the data space map at this moment.
		//       The ref-counts of remapped data blocks will be modified
		//       sequentially in the later space map traversal process,
		//       to avoid slow btree::lookup() operations.
		void remap(mapping_tree::leaf_node &n) {
			unsigned nr_entries = n.get_nr_entries();
			for (unsigned i = 0; i < nr_entries; i++) {
				mapping_tree::value_type v = n.value_at(i);

				block_address tier_block = v.block_ >> tier_block_shift_;

				if (!remappings_.contains(tier_block))
					continue;

				v.block_ = (remappings_.get_remapped_lba(tier_block) << tier_block_shift_) +
					   (v.block_ & tier_block_mask_);
				n.set_value(i, v);
			}
		}

		// Returns true if the given node contains a block that needs remapping
		bool need_remapping(mapping_tree::leaf_node const &n) {
			unsigned nr_entries = n.get_nr_entries();
			for (unsigned i = 0; i < nr_entries; i++) {
				mapping_tree::value_type v = n.value_at(i);
				if (remappings_.contains(v.block_ >> tier_block_shift_))
					return true;
			}
			return false;
		}

		block_manager<>::ptr bm_;
		tier_remappings const &remappings_; // in tiering blocks

		// internal use only
		base::run_set<block_address> visited_;
		boost::optional<block_address> previous_leaf_;
		uint64_t tier_block_shift_;
		uint64_t tier_block_mask_;
		metadata_progress_monitor &monitor_;
		bcache::validator::ptr validator_;
	};

	// Rebuild the thin data space map according to tiering remappings
	class space_map_copier: public space_map_detail::visitor {
	public:
		space_map_copier(metadata::ptr md,
				 tier_remappings const &rmap)
			: md_(md),
			  remappings_(rmap) {
			uint32_t multiplier = md_->sb_.tier_block_size_ / md->sb_.data_block_size_;
			tier_block_shift_ = ffs(multiplier) - 1;
			tier_block_mask_ = multiplier - 1;

			data_sm_ = create_disk_sm(*md->tm_, rmap.get_new_pool_size() << tier_block_shift_);
		}

		void visit(block_address b, uint32_t count) {
			if (!count)
				return;

			std::map<block_address, block_address>::const_iterator it;
			block_address tier_block = b >> tier_block_shift_;
			if (remappings_.contains(tier_block)) {
				b = (remappings_.get_remapped_lba(tier_block) << tier_block_shift_) +
				    (b & tier_block_mask_);
			}
			data_sm_->set_count(b, count);
		}

		checked_space_map::ptr get_data_sm() {
			return data_sm_;
		}

	private:
		metadata::ptr md_;
		tier_remappings const &remappings_; // in tiering blocks

		// rebuilt space map
		checked_space_map::ptr data_sm_;

		// internal use only
		uint64_t tier_block_shift_;
		uint64_t tier_block_mask_;
	};

	class space_map_damage_visitor: public space_map_detail::damage_visitor {
	public:
		virtual void visit(space_map_detail::missing_counts const &mc) {
			std::stringstream ss;
			ss << "space map has missing counts " << mc.lost_;
			throw std::runtime_error(ss.str());
		}

		virtual void visit(space_map_detail::unexpected_count const &uc) {
			std::stringstream ss;
			ss << "reference counts differ for block " << uc.b_
			   << ", expected " << uc.expected_ << ", but got ";
			if (uc.actual_)
				ss << *uc.actual_;
			else
				ss << "--";
			throw std::runtime_error(ss.str());
		}
	};

	// Check whether the given ranges are used or not
	class space_map_usage_checker: public space_map_detail::visitor {
	public:
		space_map_usage_checker(base::run_set<block_address>::const_iterator first,
					base::run_set<block_address>::const_iterator last)
			: it_(first), last_(last) {
		}

		void visit(block_address b, uint32_t count) {
			while (it_ != last_ && b >= it_->end_)
				++it_;

			if (it_ == last_)
				return;

			if (count && b >= it_->begin_) {
				std::stringstream ss;
				ss << "block " << b << " is being used";
				throw std::runtime_error(ss.str());
			}
		}

	private:
		base::run_set<block_address>::const_iterator it_;
		base::run_set<block_address>::const_iterator last_;
	};

	// FIXME: implement btree::iterator to avoid visiting all the key-value pairs
	void check_space_map_empty(metadata::ptr md, tier_span_list const &pba_reduction) {
		boost::optional<uint32_t> last_tier = pba_reduction.last_tier();
		for (uint32_t i = 0; i <= last_tier; i++) {
			tier_span_list::const_iterator begin = pba_reduction.lower_bound(i);
			tier_span_list::const_iterator end = pba_reduction.upper_bound(i);
			if (begin == end)
				continue;

			space_map_usage_checker checker(begin, end);
			space_map_damage_visitor dv;
			md->tier_data_sm_[i]->visit(checker, dv);
		}
	}
}

//----------------------------------------------------------------

namespace thin_provisioning {
	tier_block_span::tier_block_span(tier_block_span const &rhs)
		: tier_(rhs.tier_), begin_(rhs.begin_), end_(rhs.end_) {
	}

	// The input string could be either "tier:begin-end" or "tier:begin+length"
	tier_block_span::tier_block_span(const char *str) {
		std::string s(str);
		boost::char_separator<char> sep("", ":+-", boost::keep_empty_tokens);
		boost::tokenizer<boost::char_separator<char> > tok(s, sep);

		boost::tokenizer<boost::char_separator<char> >::iterator it = tok.begin();
		unsigned count = 0;
		for (; it != tok.end(); ++it)
			++count;
		if (count != 5) {
			std::ostringstream ss;
			ss << "invalid string: " << str;
			throw std::runtime_error(ss.str());
		}

		it = tok.begin();
		tier_ = boost::lexical_cast<uint64_t>(*it++);
		char delim1 = (*it)[0];
		++it;
		begin_ = boost::lexical_cast<uint64_t>(*it++);
		char delim2 = (*it)[0];
		++it;
		end_ = boost::lexical_cast<uint64_t>(*it);
		if (delim2 == '+')
			end_ += begin_;

		if (delim1 != ':' || (delim2 != '-' && delim2 != '+')) {
			std::ostringstream ss;
			ss << "invalid string: " << str;
			throw std::runtime_error(ss.str());
		}
	}

	//-------------------------------------------------------------------

	void tier_span_list::add(tier_block_span const &s) {
		if (s.tier_ >= list_.size())
			list_.resize(s.tier_ + 1);
		list_[s.tier_].add(s.begin_, s.end_);
	}

	tier_span_list::const_iterator tier_span_list::lower_bound(uint32_t tier) const {
		if (tier >= list_.size())
			return list_.back().end();
		return list_[tier].begin();
	}

	tier_span_list::const_iterator tier_span_list::upper_bound(uint32_t tier) const {
		if (tier >= list_.size())
			return list_.back().end();
		return list_[tier].end();
	}

	boost::optional<uint32_t> tier_span_list::last_tier() const {
		uint32_t size = static_cast<uint32_t>(list_.size());
		return size ? size - 1 : boost::optional<uint32_t>();
	}

	//-------------------------------------------------------------------

	void reduce_pool_size(metadata::ptr md, tier_span_list const &pba_reduction,
			      base::progress_monitor &mon) {
		verify_arguments(md, pba_reduction);

		block_address nr_allocated = md->metadata_sm_->get_nr_blocks() - md->metadata_sm_->get_nr_free();
		metadata_progress_monitor monitor(nr_allocated, mon);

		// scan through the data space map to create remappings
		tier_remappings remappings(md, pba_reduction);
		remappings.create_remappings();

		// remap tiering mappings
		tiering_remapper rmap1(md, remappings, pba_reduction, monitor);
		md->tier_tree_->visit_depth_first(rmap1);

		// remap thin data mappings
		thin_remapper rmap2(md, remappings, monitor);
		md->mappings_->visit_depth_first(rmap2);

		// rebuild thin data space map
		space_map_copier copier(md, remappings);
		space_map_damage_visitor dv;
		md->data_sm_->visit(copier, dv);
		md->data_sm_->destroy();
		md->data_sm_ = copier.get_data_sm();

		monitor.end_monitoring();
	}

	void check_pool_reducible(metadata::ptr md, tier_span_list const &pba_reduction) {
		verify_arguments(md, pba_reduction);
		check_space_map_empty(md, pba_reduction);
	}
}

//----------------------------------------------------------------

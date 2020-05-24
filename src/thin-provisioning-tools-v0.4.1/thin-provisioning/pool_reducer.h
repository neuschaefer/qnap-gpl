#ifndef POOL_REDUCER_H
#define POOL_REDUCER_H

#include "base/progress_monitor.h"
#include "metadata.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	struct tier_block_span {
		tier_block_span(tier_block_span const &rhs);
		tier_block_span(char const *str);

		uint32_t tier_;
		block_address begin_;
		block_address end_;
	};

	class tier_span_list {
	public:
		typedef base::run_set<block_address> list_type;
		typedef list_type::const_iterator const_iterator;

		void add(tier_block_span const &s);
		const_iterator lower_bound(uint32_t tier) const;
		const_iterator upper_bound(uint32_t tier) const;
		boost::optional<uint32_t> last_tier() const;

	private:
		std::vector<list_type> list_; // map tier id to ranges
	};

	void reduce_pool_size(metadata::ptr md, tier_span_list const &pba_reduction,
			      base::progress_monitor &mon);
	void check_pool_reducible(metadata::ptr md, tier_span_list const &pba_reduction);
}

//----------------------------------------------------------------

#endif

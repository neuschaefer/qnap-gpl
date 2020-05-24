#include "thin-provisioning/metadata_copier.h"

#include "base/file_copier.h"
#include "base/file_utils.h"
#include "persistent-data/file_utils.h"
#include "persistent-data/math_utils.h"
#include "persistent-data/space-maps/disk.h"
#include "persistent-data/space-maps/disk_structures.h"
#include "thin-provisioning/superblock.h"

#include <errno.h>

using namespace persistent_data;
using namespace thin_provisioning;
using namespace file_utils;

//----------------------------------------------------------------

namespace {
	class metadata_copier {
	public:
		metadata_copier(string const &src, string const &dest, uint32_t io_block_size, uint32_t iodepth) {
			if (sm_disk_detail::ENTRIES_PER_BLOCK * MD_BLOCK_SIZE % io_block_size)
				throw std::runtime_error("index entry size cannot be divided by block size");

			file_copier_ = base::create_async_file_copier(src, dest, io_block_size, iodepth);

			blocks_per_entry_ = sm_disk_detail::ENTRIES_PER_BLOCK * MD_BLOCK_SIZE / io_block_size;
			nr_io_blocks_ = div_up<uint64_t>(get_file_length(src), io_block_size);
		}

		// Copy all the blocks of an index entry.
		// Here we don't do extra checking on index entries
		// (e.g., check index_entry::nr_free_ <= ENTRIES_PER_BLOCK,
		//  or index_entry::none_free_before_ <= ENTRIES_PER_BLOCK),
		// to preserve the capability of "copy a broken metadata".
		void visit(uint32_t index, sm_disk_detail::index_entry const &ie) {
			if (ie.nr_free_ == sm_disk_detail::ENTRIES_PER_BLOCK)
				return;

			uint64_t begin = static_cast<uint64_t>(blocks_per_entry_) * index;
			uint64_t end = std::min(begin + blocks_per_entry_, nr_io_blocks_);

			file_copier_->copy(begin, end);
		}

	private:
		std::auto_ptr<file_copier> file_copier_;
		uint32_t blocks_per_entry_;
		uint64_t nr_io_blocks_; // includes the leftover metadata blocks
	};

	// reentrant version of metadata_index_store::load_ies()
	void load_indexes(block_manager<>::ptr bm,
			  superblock_detail::superblock const &sb,
			  std::vector<sm_disk_detail::index_entry> &entries) {

		sm_disk_detail::sm_root_disk d;
		sm_disk_detail::sm_root v;
		::memcpy(&d, sb.metadata_space_map_root_, sizeof(d));
		sm_disk_detail::sm_root_traits::unpack(d, v);

		block_address nr_indexes = div_up<block_address>(v.nr_blocks_, sm_disk_detail::ENTRIES_PER_BLOCK);
		entries.resize(nr_indexes);
		block_manager<>::read_ref rr =
				bm->read_lock(v.bitmap_root_, index_validator());
		sm_disk_detail::metadata_index const *mdi = reinterpret_cast<sm_disk_detail::metadata_index const *>(rr.data());
		for (block_address i = 0; i < nr_indexes; i++)
			sm_disk_detail::index_entry_traits::unpack(*(mdi->index + i), entries[i]);
	}
}

// TODO: Implement a more elegant approach to visit the index entries,
//       e.g., an index_entry visitor.
int thin_provisioning::metadata_copy(string const &src, string const &dest,
				     uint64_t io_block_size, uint64_t iodepth,
				     base::progress_monitor &mon, bool excl) {
	try {
		std::vector<sm_disk_detail::index_entry> entries;
		{
			block_manager<>::ptr bm;
			bm = open_bm(src, block_manager<>::READ_ONLY, excl);
			superblock_detail::superblock sb;
			sb = read_superblock(bm);
			load_indexes(bm, sb, entries);
		}

		metadata_copier copier(src, dest, io_block_size, iodepth);
		for (size_t i = 0; i < entries.size(); ++i) {
			copier.visit(i, entries[i]);
			mon.update_percent((i + 1) * 100 / entries.size());
		}

	} catch (std::exception &e) {
		mon.stop_monitoring();
		cerr << e.what() << endl;
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------

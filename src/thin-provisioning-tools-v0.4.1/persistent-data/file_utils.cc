#include "persistent-data/math_utils.h"
#include "persistent-data/file_utils.h"
#include "persistent-data/space-maps/disk_structures.h"

#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace base;
using namespace std;

//----------------------------------------------------------------

persistent_data::block_address
persistent_data::get_nr_blocks(std::string const &path)
{
	return div_down<block_address>(file_utils::get_file_length(path),
				       MD_BLOCK_SIZE);
}

persistent_data::block_address
persistent_data::get_nr_metadata_blocks(std::string const &path)
{
	return std::min(get_nr_blocks(path), static_cast<uint64_t>(sm_disk_detail::MAX_METADATA_BITMAPS * sm_disk_detail::ENTRIES_PER_BLOCK));
}

persistent_data::block_manager<>::ptr
persistent_data::open_bm(std::string const &dev_path, block_manager<>::mode m, bool excl)
{
	block_address nr_blocks = get_nr_metadata_blocks(dev_path);
	return block_manager<>::ptr(new block_manager<>(dev_path, nr_blocks, 1, m, excl));
}

//----------------------------------------------------------------

#ifndef METADATA_COPIER_H
#define METADATA_COPIER_H

#include <stdint.h>
#include <string>

#include "base/progress_monitor.h"

//----------------------------------------------------------------

namespace thin_provisioning {
	int metadata_copy(std::string const &src, std::string const &dest,
			  uint64_t block_size, uint64_t iodepth,
			  base::progress_monitor &mon, bool excl);
}

//----------------------------------------------------------------

#endif

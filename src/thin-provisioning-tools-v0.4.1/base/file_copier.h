#ifndef BASE_FILE_COPIER_H
#define BASE_FILE_COPIER_H

#include <stdint.h>

#include <string>
#include <memory>

namespace base {
	class file_copier {
	public:
		virtual ~file_copier() {};
		virtual void copy(uint64_t begin, uint64_t end) = 0;
		virtual void flush() = 0;
	};

	std::auto_ptr<file_copier> create_async_file_copier(
		std::string const &src,
		std::string const &dest,
		uint32_t io_block_size,
		uint32_t iodepth);
}

#endif

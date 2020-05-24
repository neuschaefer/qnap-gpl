#include "base/file_copier.h"

#include "base/file_utils.h"
#include "persistent-data/math_utils.h"

#include <errno.h>
#include <libaio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>

// FIXME: detect page size at runtime
#define PAGE_SIZE 4096

using namespace base;
using namespace file_utils;
using namespace std;

//----------------------------------------------------------------

namespace {
	void *alloc_aligned(size_t len, size_t alignment) {
		void *result = NULL;
		int r = posix_memalign(&result, alignment, len);
		if (r)
			return NULL;

		return result;
	}

	/**
	 * @brief An aio-based file copier
	 * @note  If there were errors, the client should run flush() before
	 *        the next copy(). In this situation, the flush() routine
	 *        completes the in-flight IOs only, without submitting any more
	 *        IO.
	 */
	class async_file_copier : public base::file_copier {
	public:
		enum op {
			FREE = 0,
			READ,
			WRITE
		};

		enum state {
			NORMAL = 0,
			ERROR
		};

		async_file_copier(string const &src, string const &dest, uint32_t io_block_size, uint32_t iodepth)
			: src_(src),
			  dest_(dest),
			  io_block_size_(io_block_size),
			  iodepth_(iodepth),
			  nr_free_(0),
			  nr_write_(0),
			  state_(NORMAL) {
			if (io_block_size & (PAGE_SIZE - 1))
				throw std::runtime_error("block size is not a multiple of page size");
			// FIXME: io_block_size_ should be power of two

			open_files();

			if (init_buffers())
				throw std::runtime_error("couldn't allocate buffers");

			aio_context_ = 0; /* needed or io_setup will fail */
			int ret = io_setup(iodepth_, &aio_context_);
			if (ret < 0)
				throw std::runtime_error("io_setup failed");
		}

		~async_file_copier() {
			flush();

			if (aio_context_)
				io_destroy(aio_context_);

			destroy_buffers();
			close_files();
		}

		/**
		 * @post  The iocbs not present in the free list or write list
		 *        are submitted to the kernel.
		 */
		virtual void copy(uint64_t begin, uint64_t end) {
			uint64_t b = begin;
			uint64_t e = std::min(end, nr_io_blocks_);
			uint32_t inflight = 0;

			if (state_ == ERROR)
				throw runtime_error("cannot do copy in error state");

			while (b < e) {
				// consume the free list
				iocb **read_cb = consume_free_iocbs(end - b, &inflight);

				// submit reads
				for (iocb **it = read_cb; it < read_cb + inflight; ++it) {
					(*it)->data = reinterpret_cast<void *>(READ);
					(*it)->aio_lio_opcode = IO_CMD_PREAD;
					(*it)->aio_fildes = src_fd_;
					(*it)->u.c.offset = io_block_size_ * b;
					++b;
				}
				submit_io(read_cb, inflight, true);

				// consume the write list
				iocb **write_cb = consume_write_iocbs(nr_write_, &inflight);

				// submit writes
				submit_io(write_cb, inflight, true);

				// wait for IO completion
				wait_for_events(1, iodepth_, true);
			}

			// handle the leftover bytes
			while (b < end && residual_bytes_ && b < e + 1) {
				// consume the free list
				iocb **read_cb = consume_free_iocbs(1, &inflight);

				// submit the last one read
				if (inflight) {
					iocb *cb = read_cb[0];
					cb->data = reinterpret_cast<void *>(READ);
					cb->aio_lio_opcode = IO_CMD_PREAD;
					cb->aio_fildes = src_fd_;
					cb->u.c.offset = io_block_size_ * b;
					cb->u.c.nbytes = residual_bytes_;
					++b;
				}
				submit_io(read_cb, inflight, true);

				// consume the write list
				iocb **write_cb = consume_write_iocbs(nr_write_, &inflight);

				// submit writes
				submit_io(write_cb, inflight, true);

				// wait for IO completion
				wait_for_events(1, iodepth_, true);
			}

			if (b != end) {
				state_ = ERROR;
				throw runtime_error("unexpected end of device");
			}
		}

		/**
                 * @post  All the iocbs are reclaimed to the free list.
                 *        The write list is empty, and there's no inflight IOs.
                 * @note  Do not throw exception from this function
                 */
		virtual void flush() {
			while (nr_free_ < iodepth_) {
				// consume the write list
				uint32_t inflight = 0;
				iocb **write_cb = consume_write_iocbs(nr_write_, &inflight);

				// submit writes
				submit_io(write_cb, inflight, false);

				// wait for IO completion
				wait_for_events(iodepth_ - nr_free_ - nr_write_, iodepth_, false);
			}

			state_ = NORMAL;
		}

	private:
		void open_files() {
			uint64_t file_len = get_file_length(src_);
			src_fd_ = open_file(src_, O_RDONLY);

			if (file_exists(dest_)) {
				// check file size agrees
				if (get_file_length(dest_) < file_len)
					throw std::runtime_error("unexpected file length");
			}
			dest_fd_ = open_file(dest_, O_RDWR);
			nr_io_blocks_ = base::div_down<uint64_t>(file_len, io_block_size_);
			residual_bytes_ = file_len % io_block_size_;
		}

		/**
                 * @note  Do not throw exception from this function
                 */
		void close_files() {
			::close(src_fd_);
			::close(dest_fd_);
		}

		int init_buffers() {
			iocbs_ = static_cast<iocb *>(malloc(sizeof(iocb) * iodepth_));
			if (!iocbs_)
				return -ENOMEM;

			buffers_ = static_cast<char*>(alloc_aligned(io_block_size_ * iodepth_, PAGE_SIZE));
			if (!buffers_)
				return -ENOMEM;

			events_ = static_cast<io_event *>(malloc(sizeof(io_event) * iodepth_));
			if (!events_)
				return -ENOMEM;

			free_cbs_ = static_cast<iocb **>(malloc(sizeof(iocb *) * iodepth_));
			if (!free_cbs_)
				return -ENOMEM;

			write_cbs_ = static_cast<iocb **>(malloc(sizeof(iocb *) * iodepth_));
			if (!write_cbs_)
				return -ENOMEM;

			nr_write_ = nr_free_ = 0;

			memset(iocbs_, 0, sizeof(iocb) * iodepth_);
			char *pbuf = buffers_;
			for (iocb *it = iocbs_; it < iocbs_ + iodepth_; ++it) {
				it->u.c.nbytes = io_block_size_;
				it->u.c.buf = static_cast<void *>(pbuf);
				free_iocb(it);
				pbuf += io_block_size_;
			}

			return 0;
		}

		/**
                 * @note  Do not throw exception from this function
                 */
		void destroy_buffers() {
			free(iocbs_);
			iocbs_ = NULL;

			free(events_);
			events_ = NULL;

			free(buffers_);
			buffers_ = NULL;

			free(free_cbs_);
			free_cbs_ = NULL;

			free(write_cbs_);
			write_cbs_ = NULL;

			nr_write_ = nr_free_ = 0;
		}

		/**
		 * @post  The input list is completely consumed. The input iocbs
		 *        could be either submitted to the kernel, or reclaimed
		 *        to the free list.
		 */
		void submit_io(iocb **cb, uint32_t nr_iocbs, bool throw_ex) {
			if (!nr_iocbs)
				return;

			if (state_ == ERROR) {
				for (; nr_iocbs > 0; ++cb, --nr_iocbs)
					free_iocb(*cb);
				return;
			}

			int ret = io_submit(aio_context_, nr_iocbs, cb);

			if (ret < static_cast<int>(nr_iocbs)) {
				state_ = ERROR;

				iocb **err_cbs;
				uint32_t nr_errs;

				if (ret > 0) {
					err_cbs = cb + ret;
					nr_errs = nr_iocbs - static_cast<uint32_t>(ret);
				} else {
					err_cbs = cb;
					nr_errs = nr_iocbs;
				}
				iocb *first_err = err_cbs[0];

				// Reclaim the unsubmitted IOs to the end of
				// the free list. Assume that kernel submits IOs
				// sequentially (see fs/aio.c).
				for (; err_cbs < cb + nr_iocbs; ++err_cbs)
					free_iocb(*err_cbs);

				if (throw_ex) {
					char const* err_type = "";

					switch (first_err->aio_lio_opcode) {
					case IO_CMD_PREAD:
						err_type = "read";
						break;
					case IO_CMD_PWRITE:
						err_type = "write";
						break;
					}

					std::ostringstream out;
					out << "cannot submit " << nr_errs
					    << err_type
					    << " IOs at offset " << first_err->u.c.offset;
					if (ret < 0) {
						char buf[64];
						char *msg = ::strerror_r(-ret, buf, sizeof(buf));
						out << ", reason: " << msg;
					}
					throw std::runtime_error(out.str());
				}
			}
		}

		/**
		 * @pre   The event_ list is empty.
		 * @post  The event_ list is empty, even through there was
		 *        exception. The events returned by io_getevents()
		 *        could be either queue to the write list, or reclaimed
		 *        to the free list.
		 */
		void wait_for_events(int min_nr, int max_nr, bool throw_ex) {
			int ret = io_getevents(aio_context_, min_nr, max_nr, events_, NULL);
			uint32_t const nr_events = ret > 0 ? ret : 0;

			if (ret < 0) {
				state_ = ERROR;
				if (throw_ex) {
					char buf[64];
					char *msg = ::strerror_r(-ret, buf, sizeof(buf));
					std::ostringstream out;
					out << "io_getevents failed: " << msg;
					throw std::runtime_error(out.str());
				}
			}

			io_event *ev = events_;
			io_event *first_err = NULL;
			for (; ev < (events_ + nr_events) && state_ != ERROR; ++ev) {
				switch (reinterpret_cast<intptr_t>(ev->data)) {
				case READ:
					ret = rd_callback(ev->obj, ev->res, ev->res2);
					break;
				case WRITE:
					ret = wr_callback(ev->obj, ev->res, ev->res2);
					break;
				}

				if (ret < 0) {
					first_err = ev;
					state_ = ERROR;
					break;
				}
			}

			// reclaim unsubmitted IOs
			for (; ev < events_ + nr_events; ++ev)
				free_iocb(ev->obj);

			if (first_err && throw_ex) {
				char const *err_type = "";
				switch (reinterpret_cast<intptr_t>(first_err->data)) {
				case READ:
					err_type = "read";
					break;
				case WRITE:
					err_type = "write";
					break;
				}

				std::ostringstream out;
				out << "incomplete " << err_type << " io"
				    << ", res = " << static_cast<long>(first_err->res)
				    << ", res2 = " << static_cast<long>(first_err->res2)
				    << ", offset = " << first_err->obj->u.c.offset
				    << ", nbytes = " << first_err->obj->u.c.nbytes;
				throw std::runtime_error(out.str());
			}
		}

		inline int rd_callback(iocb *cb, long res, long res2) {
			if (res != static_cast<int64_t>(io_block_size_))
				return -1;

			return queue_write_iocb(cb);
		}

		inline int wr_callback(iocb *cb, long res, long res2) {
			if (res != static_cast<int64_t>(io_block_size_))
				return -1;

			return free_iocb(cb);
		}

		//------------------------------------------------------------

		iocb** consume_free_iocbs(uint32_t nr, uint32_t *nr_consumed) {
			*nr_consumed = std::min(nr_free_, nr);
			nr_free_ -= *nr_consumed;

			return free_cbs_ + nr_free_;
		}

		iocb** consume_write_iocbs(uint32_t nr, uint32_t *nr_consumed) {
			*nr_consumed = std::min(nr_write_, nr);
			nr_write_ -= *nr_consumed;

			return write_cbs_ + nr_write_;
		}

		int queue_write_iocb(iocb *cb) {
			cb->data = reinterpret_cast<void *>(WRITE);
			cb->aio_lio_opcode = IO_CMD_PWRITE;
			cb->aio_fildes = dest_fd_;
			write_cbs_[nr_write_++] = cb;

			return 0;
		}

		int free_iocb(iocb *cb) {
			cb->data = reinterpret_cast<void *>(FREE);
			free_cbs_[nr_free_++] = cb;

			return 0;
		}

		//------------------------------------------------------------

		std::string src_;
		std::string dest_;
		uint32_t io_block_size_; // in bytes
		uint32_t iodepth_;    // num of outstanding (read+write) IOs

		int src_fd_;
		int dest_fd_;
		uint64_t nr_io_blocks_;
		uint32_t residual_bytes_;

		io_context_t aio_context_;
		iocb *iocbs_;
		io_event *events_;
		char *buffers_;
		iocb **free_cbs_;   // list of free iocbs
		uint32_t nr_free_;  // number of free iocbs
		iocb **write_cbs_;  // list of iocbs for writing (excludes in-flight)
		uint32_t nr_write_; // number of iocbs for writing

		state state_;
	};
}

//----------------------------------------------------------------

std::auto_ptr<file_copier>
base::create_async_file_copier(std::string const &src,
			       std::string const &dest,
			       uint32_t io_block_size,
			       uint32_t iodepth) {
	return auto_ptr<file_copier>(new async_file_copier(src, dest, io_block_size, iodepth));
}

//----------------------------------------------------------------

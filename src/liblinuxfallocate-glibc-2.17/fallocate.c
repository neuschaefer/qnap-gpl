/* Copyright (C) 2007, 2009, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef QNAPNAS
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "sysdep-cancel.h"
#include "kernel-features.h"
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) \
	        syscall(__NR_##name, ##args)
#undef __set_errno
#define __set_errno(val) (errno = (val))
#else /* QNAPNAS */
#include <errno.h>
#include <fcntl.h>
#include <sysdep-cancel.h>
#endif /* QNAPNAS */


/* Reserve storage for the data of the file associated with FD.  */
int
#ifdef QNAPNAS
fallocate (int fd, int mode, off_t offset, off_t len)
#else
fallocate (int fd, int mode, __off_t offset, __off_t len)
#endif /* QNAPNAS */
{
#ifdef __NR_fallocate
#ifdef QNAPNAS
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (fallocate, 4, fd, mode, offset, len);
#else
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (fallocate, 6, fd, mode,
			   __LONG_LONG_PAIR (offset >> 31, offset),
			   __LONG_LONG_PAIR (len >> 31, len));
#endif /* QNAPNAS */

  int result;
  int oldtype = LIBC_CANCEL_ASYNC ();

#ifdef QNAPNAS
  result = INLINE_SYSCALL (fallocate, 4, fd, mode, offset, len);
#else
  result = INLINE_SYSCALL (fallocate, 6, fd, mode,
			   __LONG_LONG_PAIR (offset >> 31, offset),
			   __LONG_LONG_PAIR (len >> 31, len));
#endif /* QNAPNAS */

  LIBC_CANCEL_RESET (oldtype);

  return result;
#else
  __set_errno (ENOSYS);
  return -1;
#endif /* __NR_fallocate */
}

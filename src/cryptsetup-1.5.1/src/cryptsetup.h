/*
 * cryptsetup - setup cryptographic volumes for dm-crypt
 *
 * Copyright (C) 2004, Christophe Saout <christophe@saout.de>
 * Copyright (C) 2004-2007, Clemens Fruhwirth <clemens@endorphin.org>
 * Copyright (C) 2009-2012, Red Hat, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef CRYPTSETUP_H
#define CRYPTSETUP_H

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>
#include <popt.h>
#include <sys/stat.h>

#include "lib/nls.h"
#include "lib/utils_crypt.h"
#include "lib/utils_loop.h"
#include "lib/utils_fips.h"

#include "libcryptsetup.h"

#define CONST_CAST(x) (x)(uintptr_t)
#define DEFAULT_CIPHER(type)	(DEFAULT_##type##_CIPHER "-" DEFAULT_##type##_MODE)
#define SECTOR_SIZE 512
#define ROUND_SECTOR(x) (((x) + SECTOR_SIZE - 1) / SECTOR_SIZE)

extern int opt_debug;
extern int opt_verbose;
extern int opt_batch_mode;

/* Common tools */
void clogger(struct crypt_device *cd, int level, const char *file, int line,
	     const char *format, ...);
void tool_log(int level, const char *msg, void *usrptr __attribute__((unused)));
void quiet_log(int level, const char *msg, void *usrptr);

int yesDialog(const char *msg, void *usrptr __attribute__((unused)));
void show_status(int errcode);
const char *uuid_or_device(const char *spec);
void usage(poptContext popt_context, int exitcode, const char *error, const char *more);
void dbg_version_and_cmd(int argc, const char **argv);
int translate_errno(int r);

/* Log */
#define log_dbg(x...) clogger(NULL, CRYPT_LOG_DEBUG, __FILE__, __LINE__, x)
#define log_std(x...) clogger(NULL, CRYPT_LOG_NORMAL, __FILE__, __LINE__, x)
#define log_verbose(x...) clogger(NULL, CRYPT_LOG_VERBOSE, __FILE__, __LINE__, x)
#define log_err(x...) clogger(NULL, CRYPT_LOG_ERROR, __FILE__, __LINE__, x)

#endif /* CRYPTSETUP_H */

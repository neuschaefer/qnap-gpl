/*
 * Copyright (c) 2002-2007 BalaBit IT Ltd, Budapest, Hungary                    
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Note that this permission is granted for only version 2 of the GPL.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef SYSLOG_NG_H_INCLUDED
#define SYSLOG_NG_H_INCLUDED

#include <config.h>

#if ENABLE_DMALLOC
#define USE_DMALLOC
#endif

#if ENABLE_DEBUG
#define YYDEBUG 1
#endif

#include <glib.h>

#define PATH_SYSLOG_NG_CONF     PATH_SYSCONFDIR "/syslog-ng.conf"
#define PATH_PIDFILE            "/var/run/syslog-ng.pid"
#define PATH_PERSIST_CONFIG     PATH_LOCALSTATEDIR "/syslog-ng.persist"

#define LOG_PRIORITY_LISTEN 0
#define LOG_PRIORITY_READER 0
#define LOG_PRIORITY_WRITER -100
#define LOG_PRIORITY_CONNECT -150

#if !HAVE_STRTOLL
# if HAVE_STRTOIMAX || defined(strtoimax)
   /* HP-UX has an strtoimax macro, not a function */
   #define strtoll(nptr, endptr, base) strtoimax(nptr, endptr, base)
# else
   /* this requires Glib 2.12 */
   #define strtoll(nptr, endptr, base) g_ascii_strtoll(nptr, endptr, base)
# endif
#endif

#endif

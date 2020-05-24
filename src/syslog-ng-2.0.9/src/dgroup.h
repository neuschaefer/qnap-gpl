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

#ifndef DGROUP_H_INCLUDED
#define DGROUP_H_INCLUDED

#include "syslog-ng.h"
#include "logpipe.h"
#include "driver.h"


typedef struct _LogDestGroup
{
  LogPipe super;
  GString *name;
  LogDriver *drivers;
  guint32 *processed_messages;
} LogDestGroup;

static inline LogDestGroup *
log_dest_group_ref(LogDestGroup *self)
{
  return (LogDestGroup *) log_pipe_ref(&self->super);
}

static inline void
log_dest_group_unref(LogDestGroup *self)
{
  log_pipe_unref(&self->super);
}

LogDestGroup *log_dest_group_new(gchar *name, LogDriver *drivers);

#endif

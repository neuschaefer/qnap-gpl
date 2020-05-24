/*
 * Unix SMB/CIFS implementation.
 *
 * Copyright (C) Volker Lendecke, 2005
 * Copyright (C) Aravind Srinivasan, 2009
 * Copyright (C) Guenter Kukkukk, 2013
 * Copyright (C) Ralph Boehme, 2017
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef QNAPNAS
#include <string.h>
#include "uninorm.h"
#endif /* QNAPNAS */

struct char_mappings;

struct char_mappings **string_replace_init_map(const char **mappings);

NTSTATUS string_replace_allocate(connection_struct *conn,
				 const char *name_in,
				 struct char_mappings **cmaps,
				 TALLOC_CTX *mem_ctx,
				 char **mapped_name,
				 enum vfs_translate_direction direction);

#ifdef QNAPNAS
bool IsQmappingEnable();
bool IsClientMacOs();
uint8_t* u8_norm_converter(uninorm_t nf, const uint8_t *src);
char* RunQmapping(const char *name_in);
#endif /* QNAPNAS */


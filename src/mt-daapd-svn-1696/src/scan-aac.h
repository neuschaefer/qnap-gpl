/*
 * $Id: scan-aac.h,v 1.1.1.1 2011/11/03 12:26:52 ken Exp $
 *
 * Copyright (C) 2003 Ron Pedde (ron@pedde.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _SCAN_AAC_H_
#define _SCAN_AAC_H_

#include "io.h"

extern uint64_t scan_aac_drilltoatom(IOHANDLE hfile, char *atom_path, unsigned int *atom_length);
extern uint64_t scan_aac_findatom(IOHANDLE hfile, uint64_t max_offset, char *which_atom, unsigned int *atom_size);

#endif

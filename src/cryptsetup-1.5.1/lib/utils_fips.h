/*
 * FIPS mode utilities
 *
 * Copyright (C) 2011-2012, Red Hat, Inc. All rights reserved.
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

#ifndef _UTILS_FIPS_H
#define _UTILS_FIPS_H

struct crypt_device;

int crypt_fips_mode(void);
void crypt_fips_libcryptsetup_check(struct crypt_device *cd);
void crypt_fips_self_check(struct crypt_device *cd);

#endif /* _UTILS_FIPS_H */

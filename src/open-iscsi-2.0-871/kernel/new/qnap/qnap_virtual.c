/*******************************************************************************
 * Filename:  qnap_virtual.c
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
 ****************************************************************************/

#include "qnap_virtual.h"

int iscsi_dev_arr[MAX_ISCSI_DISK];
struct device *iscsi_dev_ptr[MAX_ISCSI_DISK];

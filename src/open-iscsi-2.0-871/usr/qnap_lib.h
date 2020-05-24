/*
 * QNAP Open-iSCSI Libray
 *
 * Copyright (C) 2018 QNAP SYSTEMS, INC. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef QNAP_LIB_H
#define QNAP_LIB_H

#include <stdint.h>
#include <net/if.h>
#include <sys/time.h>
#include "initiator.h"
#include "transport.h"

/**/
struct brif_dev {
	char if_name[256];
};

struct brif_dev_list {
	int if_count;
	struct brif_dev *if_lists;
};

/* return 0 if str is same */
static inline int qlib_str_is_same(char *s, char *d)
{
	if (s && d 
	&& (strlen(s) == strlen(d))
	&& !strcmp(s, d)
	)
		return 0;
	return -EINVAL;
}


/**/
extern void qlib_dump_mem(unsigned char *pu8Buf, int DumpSize);

/* TO check if_name is br dev or not
 * - ret: 0 is br dev, others is fail
 */
extern int qlib_check_is_br_dev(char *if_name);

/* To get brif dev count from br dev path
 * i.e
 * /sys/class/net/qvs0/brif/eth5@
 * /sys/class/net/qvs0/brif/eth6@
 *
 * it means we have brif dev count is 2
 */
extern int qlib_get_brif_count_from_br_dev(char *if_name);

/* To get brif dev lists information from br dev path and 
 * store into brif_dev_list structure
 *
 * Caller must free memory buffer for brif_dev_list->if_lists member
 */
extern int qlib_get_brif_dev_from_br_dev(char *if_name, struct brif_dev_list *brif_lists);

extern int qlib_check_iser_nic(char *nic_name);
extern int qlib_check_do_iser_bind(struct iscsi_conn *conn, int addr_familiy);
extern int qlib_get_ip_by_if(char *netdev, char *buf, int buf_len, int addr_familiy);


#endif /* QNAP_LIB_H */

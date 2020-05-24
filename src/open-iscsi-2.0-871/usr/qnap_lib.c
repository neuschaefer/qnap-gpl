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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include "initiator.h"
#include "transport.h"
#include "log.h"
#include "qnap_lib.h"


static unsigned char Hex[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

void qlib_dump_mem(
	unsigned char *pu8Buf,
	int DumpSize
	)
{
	unsigned char *Data;
	unsigned char Val[50];
	unsigned char Str[20];
	unsigned char c;
	int Size;
	int Index;

	Data = pu8Buf;

	while (DumpSize) {
		Size = 16;
		if (Size > DumpSize) {
			Size = DumpSize;
		}

		for (Index = 0; Index < Size; Index += 1) {
			c                   = Data[Index];
			Val[Index * 3 + 0]  = Hex[c >> 4];
			Val[Index * 3 + 1]  = Hex[c & 0xF];
			Val[Index * 3 + 2]  = (unsigned char) ((Index == 7) ? '-' : ' ');
			Str[Index]          = (unsigned char) ((c < ' ' || c > 'z') ? '.' : c);
		}

		Val[Index * 3]  = 0;
		Str[Index]      = 0;
		log_debug(7, "addr-0x%p: %s *%s*\n",Data, Val, Str);
		Data += Size;
		DumpSize -= Size;
	}
}

/* TO check if_name is br dev or not
 * - ret: 0 is br dev, others is fail
 */
int qlib_check_is_br_dev(
	char *if_name
	)
{
	char path[256];
	DIR *dir = NULL;

	/* check bridge folder */
	snprintf(path, 256, "/sys/class/net/%s/bridge", if_name);
	
	dir = opendir(path);
	if (!dir) 
		return -EINVAL;
	closedir(dir);

	/* check brif folder */
	snprintf(path, 256, "/sys/class/net/%s/brif", if_name);
	dir = opendir(path);
	if (!dir) 
		return -EINVAL;
	closedir(dir);

	log_debug(7, "%s: found br dev:%s\n", __func__, if_name);
	return 0;

}


/* To get brif dev count from br dev path
 * i.e
 * /sys/class/net/qvs0/brif/eth5@
 * /sys/class/net/qvs0/brif/eth6@
 *
 * it means we have brif dev count is 2
 */
int qlib_get_brif_count_from_br_dev(
	char *if_name
	)
{
	char path[256];
	DIR *dir = NULL;
	struct dirent *if_entry = NULL;
	int count = 0;
	struct stat statbuf;

	if (qlib_check_is_br_dev(if_name))
		return 0;

	snprintf(path, 256, "/sys/class/net/%s/brif", if_name);
	dir = opendir(path);
	if (!dir) 
		return 0;

	while ((if_entry = readdir(dir)) != NULL) {
		if (!strncmp(if_entry->d_name, ".", 1)
		|| !strncmp(if_entry->d_name, "..", 2)
		)
			continue;

		/* (example)
		 * ls -al /sys/class/net/qvs0/brif/
		 * total 0
		 * drwxr-xr-x 2 admin administrators 0 2018-07-20 13:31 ./
		 * drwxr-xr-x 7 admin administrators 0 2018-07-20 13:31 ../
		 * lrwxrwxrwx 1 admin administrators 0 2018-07-23 09:24 eth5 -> ../../../../pci0000:00/0000:00:01.2/0000:03:00.0/net/eth5/brport/
		 */
		snprintf(path, 256, "/sys/class/net/%s/brif/%s", if_name, 
			if_entry->d_name);
		if ((lstat(path, &statbuf) < 0) || !S_ISLNK(statbuf.st_mode))
			continue;
		count++;
	}

	log_debug(7, "%s: brif dev counts:%d\n", __func__, count);
	closedir(dir);
	return count;
}


/* To get brif dev lists information from br dev path and 
 * store into brif_dev_list structure
 *
 * Caller must free memory buffer for brif_dev_list->if_lists member
 */
int qlib_get_brif_dev_from_br_dev(
	char *if_name,
	struct brif_dev_list *brif_lists
	)
{
	int count;
	struct brif_dev *lists = NULL;
	char path[256];
	DIR *dir = NULL;
	struct dirent *if_entry = NULL;
	struct stat statbuf;

	brif_lists->if_count = 0;
	brif_lists->if_lists = NULL;

	count = qlib_get_brif_count_from_br_dev(if_name);
	if (!count)
		/* treat this call is good but need to check brif_lists */
		return 0;

	lists = calloc(1, (count * sizeof(struct brif_dev)));
	if (!lists)
		return -ENOMEM;

	snprintf(path, 256, "/sys/class/net/%s/brif", if_name);
	dir = opendir(path);
	if (!dir) {
		free(lists);
		return -EINVAL;
	}

	count = 0;
	while ((if_entry = readdir(dir)) != NULL) {
		if (!strncmp(if_entry->d_name, ".", 1)
		|| !strncmp(if_entry->d_name, "..", 2)
		)
			continue;

		snprintf(path, 256, "/sys/class/net/%s/brif/%s", if_name, 
				if_entry->d_name);
		if (lstat(path, &statbuf) < 0 || !S_ISLNK(statbuf.st_mode))
			continue;
		snprintf(lists[count].if_name, 256, "%s", if_entry->d_name);

		log_error("%s: found brif dev:%s\n", __func__,
			lists[count].if_name);
		count++;
	}

	brif_lists->if_count = count;
	brif_lists->if_lists = lists;
	closedir(dir);
	return 0;

}

static struct __vendor {
	char *vid;
} iser_vendor[] = {
	{"0x15b3"},
	{NULL},
};

/* 1: found iser nic which vendor id is in iser_vendor[]
 * 0: not found iser nic
 */
int qlib_check_iser_nic(
	char *nic_name
	)
{
	FILE *fp = NULL;
	char buf[16] = {0}, vendor_path[256] = {0};
	char *if_name = NULL;
	int i, found = 0, ret;
	struct brif_dev_list brif_lists;
	struct brif_dev *br_dev = NULL;

	brif_lists.if_count = 0;
	brif_lists.if_lists = NULL;

	if (!qlib_check_is_br_dev(nic_name)) {
		ret = qlib_get_brif_dev_from_br_dev(nic_name, &brif_lists);
		if (!ret && (brif_lists.if_count == 1)) {
			br_dev = brif_lists.if_lists;
			if_name = br_dev->if_name;
		} else {
			goto out;
		}
	} else
		if_name = nic_name;

	snprintf(vendor_path, 256, "/sys/class/net/%s/device/vendor", if_name);

	fp = fopen(vendor_path, "r");
	if (!fp)
		goto out;

	if (!fgets(buf, 16, fp))
		goto out;

	for (i = 0;; i++) {
		if (!strncmp(iser_vendor[i].vid, buf, strlen(iser_vendor[i].vid))) {
			found = 1;
			break;
		}
	}

out:
	if (brif_lists.if_lists)
		free(brif_lists.if_lists);
	if (fp)
		fclose(fp);
	
	return ((found) ? 1 : 0);
}

/* 0    : to do iser binding 
 * <> 0 : NOT do iser binding
 */
int qlib_check_do_iser_bind(
	struct iscsi_conn *conn,
	int addr_familiy
	)
{
	struct iscsi_session *sess = conn->session;
	iface_rec_t *irec = &sess->nrec.iface;
	int ret = -EINVAL;

	log_debug(7, "%s: irec->qnap_netdev: %s\n", __func__, irec->qnap_netdev);

	/* check need to bind src for QNAP iser or not */
	if (qlib_check_iser_nic(irec->qnap_netdev)) {

		log_debug(7, "%s: found qnap_netdev:%s\n", __func__, irec->qnap_netdev);
		
		ret = qlib_get_ip_by_if(irec->qnap_netdev, irec->qnap_ipaddress, 
			NI_MAXHOST, addr_familiy);

		if (!ret) {
			/* Set port to null, the port num is 0. And, the port
			 * will be allocated from cma_get_port() in
			 * infiniband/core/cma.c automatically
			 *
			 * cma_get_port()
			 * -> cma_alloc_any_port() if cma_any_port() is TRUE
			 * --> cma_alloc_port()
			 * ---> cma_bind_port()
			 */
			ret = resolve_address(irec->qnap_ipaddress, NULL, 
				&conn->qnap_iser_bind_addr);

		}
	}

	return ((!ret) ? 0 : -EINVAL);
}

/* 0    : get ip successfully
 * <> 0 : fail to get ip
 */
int qlib_get_ip_by_if(
	char *netdev,
	char *buf,
	int buf_len,
	int addr_familiy
	)
{
	int ret = -EINVAL, found = 0;
	struct ifaddrs *ifaddr = NULL, *ifa = NULL;
	char *host = NULL;

	host = (char *)calloc(1, NI_MAXHOST);
	if (!host)
		return -ENOMEM;

	ret = getifaddrs(&ifaddr);
	if (ret) {
		free(host);
		return -EINVAL;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || !ifa->ifa_name)
			continue;
		/* non-zero : str isn't the same */
		if (qlib_str_is_same(ifa->ifa_name, netdev))
			continue;

		if (!(ifa->ifa_addr->sa_family == AF_INET ||
				ifa->ifa_addr->sa_family == AF_INET6))
			continue;

		if (addr_familiy != ifa->ifa_addr->sa_family)
			continue;
	
		ret = getnameinfo(ifa->ifa_addr, (addr_familiy == AF_INET) ? 
			sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
			host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (ret)
			continue;

		log_debug(7, "%s: host:%s\n", __func__, host);

		snprintf(buf, buf_len, "%s", host);
		found++;
		break;
	}

	if (host)
		free(host);
	if (ifaddr)
		freeifaddrs(ifaddr);

	return ((found) ? 0 : -ENODEV);
}



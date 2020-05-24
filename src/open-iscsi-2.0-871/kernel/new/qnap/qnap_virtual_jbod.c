/*******************************************************************************
 * Filename:  qnap_virtual_jbod.c
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

#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>

#include "../sd.h"

#include "qnap_virtual_jbod.h"

struct dev_arr *iscsi_vjbod_arr;

static u8 hex[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void __dump_mem(char *buf, size_t dump_size)
{
	u8 *data;
	u8 val[50], str[20], c;
	size_t size, idx;

	data = buf;

	while (dump_size) {

		size = 16;
		if (size > dump_size)
			size = dump_size;

		for (idx = 0; idx < size; idx += 1) {
			c = data[idx];
			val[idx * 3 + 0] = hex[c >> 4];
			val[idx * 3 + 1] = hex[c & 0xF];
			val[idx * 3 + 2] = (u8) ((idx == 7) ? '-' : ' ');
			str[idx] = (u8) ((c < ' ' || c > 'z') ? '.' : c);
		}

		val[idx * 3] = 0;
		str[idx] = 0;
		pr_info("addr-0x%p: %s *%s*\n",data, val, str);
		data += size;
		dump_size -= size;
	}
}

static int __qnap_get_iscsi_vjbod_idx(
	struct scsi_device *sdev,
	u32 *idx
	)
{
	int i, found = 0, ret;


	for (i = 0; i < MAX_ISCSI_DISK; i++) {
		if (iscsi_vjbod_arr->vjbod_dev[i] == 0) {
			iscsi_vjbod_arr->vjbod_dev[i] = 1;
			found = 1;
			break;
		}
	}

	if (found == 0)
		return -EBUSY;

	*idx = GET_ISCSI_VJBOD_IDX('x', ('a'+ i));

	if (!IS_ISCSI_VJBOD_IDX(*idx))
		return -ENODEV;

	pr_info("found vjbod sd idx:%d\n", *idx);
	return 0;
}

static int __qnap_get_iscsi_rdisk_vjbod_idx(
	struct scsi_device *sdev,
	u32 *idx
	)
{
	int i, found = 0, ret;
	for (i = 0; i < MAX_ISCSI_DISK; i++) {
		/* check this since iscsi_dev_arr[] will record something
		 * for original remote disk function
		 */
		if (iscsi_dev_arr[i] == 0) {
			iscsi_dev_arr[i] = 1;
			iscsi_vjbod_arr->vjbod_rdisk_dev[i] = 1;
			found = 1;
			break;
		}
	}

	if (found == 0)
		return -EBUSY;

	*idx = Get_iSCSI_Index('w', ('a'+ i));
	if (!Is_iSCSI_Index(*idx))
		return -ENODEV;

	pr_info("found remote disk of vjbod sd idx:%d\n", *idx);

	/* TODO
	 * not touch iscsi_iqn_arr[], iscsi_sn_vpd_arr[]
	 */
	return 0;
}

static void __qnap_clear_iscsi_vjbod_rdisk_idx(
	struct scsi_device *sdev,
	u32 idx
	)
{
	idx -= ISCSI_DEV_START_INDEX;
	iscsi_vjbod_arr->vjbod_rdisk_dev[idx] = 0;
	iscsi_dev_arr[idx] = 0;
	pr_info("Remote disk of vJBOD iSCSI LUN:%d is released\n", idx);
	return;
}

static void __qnap_clear_iscsi_vjbod_idx(
	struct scsi_device *sdev,
	u32 idx
	)
{
	idx -= ISCSI_VJBOD_DEV_START_IDX;
	iscsi_vjbod_arr->vjbod_dev[idx] = 0;
	pr_info("vJBOD iSCSI LUN:%d is released\n", idx);
	return;
}

static int __scsi_get_lun_naa(
	struct scsi_device *sdev,
	unsigned char *buf,
	int buf_len
	)
{
	unsigned char *result = NULL, *desc;
	int found_naa = 0, len = 0, desc_len = 0, ret = -ENOMEM;

	result = kzalloc(buf_len, GFP_KERNEL);
	if (!result)
		goto _out_free_;

	ret = -EINVAL;
	if (scsi_get_vpd_page(sdev, 0x83, result, buf_len))
		goto _out_free_;

	if ((result[0] != TYPE_DISK) || (result[1] != 0x83)
	|| (get_unaligned_be16(&result[2]) == 0)
	)
		goto _out_free_;

	len = get_unaligned_be16(&result[2]);
	desc = &result[4];

	while (len) {
		desc_len = 4 + desc[3];

		if (((desc[0] & 0x0f) == 0x1) /* if binary set ? */
		&& (((desc[1] & 0x30) >> 4) == 0x0) /* if addressed logical unit ? */
		&& ((desc[1] & 0x0f) == 0x3) /* if naa type ? */
		)
		{
			/* 1. NAA IEEE Registered Extended Identifier/Designator
			 * 2. QNAP IEEE Company ID
			 */
			if ((desc[4] == ((0x6 << 4)| 0x0e))
			&& (desc[5] == 0x84)
			&& (desc[6] == 0x3b)
			&& (((desc[7] & 0xf0) >> 4) == 0x6)
			)
			{
				memcpy(buf, &desc[4], 0x10);
				found_naa = 1;
				break;
			}
		}

		len -= desc_len;
		desc += desc_len;
	}

_out_free_:
	if (result)
		kfree(result);

	if (found_naa)
		return 0;

	return ret;
}

static int __scsi_get_lun_sn(
	struct scsi_device *sdev,
	unsigned char *buf,
	int buf_len
	)
{
	unsigned char *result = NULL;
	int found_sn = 0, len = 0, ret = -ENOMEM;

	result = kzalloc(buf_len, GFP_KERNEL);
	if (!result)
		goto _out_free_;

	ret = -EINVAL;
	if (scsi_get_vpd_page(sdev, 0x80, result, buf_len))
		goto _out_free_;

	if ((result[0] != TYPE_DISK) || (result[1] != 0x80)
	|| (get_unaligned_be16(&result[2]) == 0)
	)
		goto _out_free_;

	len = get_unaligned_be16(&result[2]);
	memcpy(buf, &result[4], len);
	found_sn = 1;

_out_free_:
	if (result)
		kfree(result);

	if (found_sn)
		return 0;

	return ret;
}

int qnap_get_sd_dev_name(
	struct scsi_device *sdev,
	char *disk_name
	)
{
#define	ALLOC_BUF_LEN		0x100
	unsigned char *buffer = NULL;

	if (!disk_name)
		return -EINVAL;

	buffer = kzalloc(ALLOC_BUF_LEN, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	sprintf(buffer, "/dev/%s", disk_name);
	sdev->dev_name = buffer;
	return 0;
}
EXPORT_SYMBOL(qnap_get_sd_dev_name);

static int __scsi_get_lun_capacity(
	struct scsi_device *sdev,
	unsigned char *buf,
	sector_t *capacity,
	unsigned *sector_size
	)
{
	/* here refers the read_capacity_16() */

	unsigned char cmd[16];
	struct scsi_sense_hdr sshdr;
	int sense_valid = 0;
	int the_result;
	int retries = 3, reset_retries = 10;
	unsigned int alignment;

	do {
		memset(cmd, 0, 16);
		cmd[0] = SERVICE_ACTION_IN_16;
		cmd[1] = SAI_READ_CAPACITY_16;
		cmd[13] = 32;
		memset(buf, 0, 32);

		the_result = scsi_execute_req(sdev, cmd, DMA_FROM_DEVICE,
					buf, 32, &sshdr,
					SD_TIMEOUT, SD_MAX_RETRIES, NULL);


		if (scsi_sense_valid(&sshdr)) {
			/* not invoked for commands that could return deferred errors */
			switch (sshdr.sense_key) {
			case UNIT_ATTENTION:
			case NOT_READY:
				/* medium not present */
				if (sshdr.asc == 0x3A)
					return -ENODEV;
			}
		}

		if (the_result) {
			sense_valid = scsi_sense_valid(&sshdr);
			if (sense_valid &&
			    sshdr.sense_key == ILLEGAL_REQUEST &&
			    (sshdr.asc == 0x20 || sshdr.asc == 0x24) &&
			    sshdr.ascq == 0x00)
				/* Invalid Command Operation Code or
				 * Invalid Field in CDB, just retry
				 * silently with RC10 */
				return -EINVAL;
			if (sense_valid &&
			    sshdr.sense_key == UNIT_ATTENTION &&
			    sshdr.asc == 0x29 && sshdr.ascq == 0x00)
				/* Device reset might occur several times,
				 * give it one more chance */
				if (--reset_retries > 0)
					continue;
		}
		retries--;
	} while (the_result && retries);

	if (the_result)
		return -EINVAL;

	*sector_size = get_unaligned_be32(&buf[8]);
	*capacity = get_unaligned_be64(&buf[0]);;
	return 0;
}

int qnap_get_iscsi_vjbod_idx(
	struct scsi_device *sdev,
	int type,
	u32 *idx
	)
{
	int ret;

	switch(type) {
	case SD_VJBOD:
		ret = __qnap_get_iscsi_vjbod_idx(sdev, idx);
		break;
	case SD_VJBOD_RDISK:
		ret = __qnap_get_iscsi_rdisk_vjbod_idx(sdev, idx);
		break;
	default:
		ret = -ENODEV;
		break;
	}

	return ret;
}

void qnap_clear_iscsi_vjbod_idx(
	struct scsi_device *sdev,
	int type,
	u32 idx
	)
{
	switch(type) {
	case SD_VJBOD:
		__qnap_clear_iscsi_vjbod_idx(sdev, idx);
		break;
	case SD_VJBOD_RDISK:
		__qnap_clear_iscsi_vjbod_rdisk_idx(sdev, idx);
		break;
	default:
		break;
	}

	return;

}
EXPORT_SYMBOL(qnap_clear_iscsi_vjbod_idx);

int qnap_scsi_get_lun_naa(struct scsi_device *sdev)
{
#define	ALLOC_BUF_LEN		0x100

	unsigned char tmp_naa_buf[ALLOC_BUF_LEN], c;
	unsigned char *naa_buffer = NULL;
	int i, ret;

	/* try to find the NAA information */
	naa_buffer = kzalloc(ALLOC_BUF_LEN, GFP_KERNEL);
	if (!naa_buffer)
		return -ENOMEM;

	ret = __scsi_get_lun_naa(sdev, naa_buffer, ALLOC_BUF_LEN);
	if (ret) {
		kfree(naa_buffer);
		return -ENOTSUPP;
	}

	memset(tmp_naa_buf, 0, ALLOC_BUF_LEN);

	for (i = 0; i < 0x10; i++) {
		c = naa_buffer[i];
		tmp_naa_buf[i*2 + 0] = hex[c >> 4];
		tmp_naa_buf[i*2 + 1] = hex[c & 0xF];
	}

	memcpy(naa_buffer, tmp_naa_buf, ALLOC_BUF_LEN);
	sdev->dev_naa = naa_buffer;

	return 0;
}
EXPORT_SYMBOL(qnap_scsi_get_lun_naa);

int qnap_scsi_get_lun_sn(struct scsi_device *sdev)
{
#define	ALLOC_BUF_LEN		0x100

	unsigned char *sn_buffer = NULL;
	int i, ret;

	sn_buffer = kzalloc(ALLOC_BUF_LEN, GFP_KERNEL);
	if (!sn_buffer)
		return -ENOMEM;

	ret = __scsi_get_lun_sn(sdev, sn_buffer, ALLOC_BUF_LEN);
	if (ret) {
		kfree(sn_buffer);
		return -ENOTSUPP;
	}

	sdev->dev_sn = sn_buffer;
	return 0;
}
EXPORT_SYMBOL(qnap_scsi_get_lun_sn);

int qnap_scsi_get_lun_capacity(struct scsi_device *sdev)
{
#define	ALLOC_BUF_LEN		512

	unsigned char *buffer = NULL;
	int ret;
	sector_t capacity;
	unsigned sector_size;

	buffer = kzalloc(ALLOC_BUF_LEN, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	ret = __scsi_get_lun_capacity(sdev, buffer, &capacity, &sector_size);

	if (!ret) {
		sdev->dev_sector_size = sector_size;
		sdev->dev_capacity = capacity;
		sdev->dev_capacity += 1;
	}

	kfree(buffer);
	return ret;
}
EXPORT_SYMBOL(qnap_scsi_get_lun_capacity);

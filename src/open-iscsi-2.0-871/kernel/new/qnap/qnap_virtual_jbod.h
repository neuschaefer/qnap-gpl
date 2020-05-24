#ifndef _QNAP_VIRTUAL_JBOD_H
#define _QNAP_VIRTUAL_JBOD_H

#include <linux/device.h>
#include "qnap_virtual.h"

/* QNAP VJBOD function (start) VIRTUAL_JBOD */
#define SD_NOOP		0
#define SD_VJBOD	1
#define SD_VJBOD_RDISK	2
/* QNAP VJBOD (end) */

#define ISCSI_VJBOD_DEV_START_IDX	624
#define GET_ISCSI_VJBOD_IDX(c1, c2)	(26 *(c1-'a'+1) +c2-'a')
#define IS_ISCSI_VJBOD_IDX(i)			\
	((i >= ISCSI_VJBOD_DEV_START_IDX &&	\
	i <= (ISCSI_VJBOD_DEV_START_IDX + MAX_ISCSI_DISK-1)) ? 1 : 0)	\

extern struct dev_arr *iscsi_vjbod_arr;

struct dev_arr {
	int vjbod_dev[MAX_ISCSI_DISK];
	int vjbod_rdisk_dev[MAX_ISCSI_DISK];
};

int qnap_get_iscsi_vjbod_idx(struct scsi_device *sdev, int type, u32 *idx);
int qnap_get_sd_dev_name(struct scsi_device *sdev, char *disk_name);
void qnap_clear_iscsi_vjbod_idx(struct scsi_device *sdev, int type, u32 idx);

#endif /* _QNAP_VIRTUAL_JBOD_H */

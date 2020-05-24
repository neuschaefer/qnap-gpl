#ifndef _QNAP_VIRTUAL_H
#define _QNAP_VIRTUAL_H

#define ISCSI_DEV_START_INDEX 598
#define MAX_ISCSI_DISK 26
#define Is_iSCSI_Index(index) ((index >= ISCSI_DEV_START_INDEX && index <= (ISCSI_DEV_START_INDEX+MAX_ISCSI_DISK-1))? 1 : 0)
#define Get_iSCSI_Index(c1, c2) (26*(c1-'a'+1)+c2-'a')
extern int iscsi_dev_arr[MAX_ISCSI_DISK];
extern struct device *iscsi_dev_ptr[MAX_ISCSI_DISK];

#endif /* _QNAP_VIRTUAL_H */

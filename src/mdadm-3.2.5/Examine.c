/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2009 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include	"mdadm.h"
#include	"dlink.h"

#if ! defined(__BIG_ENDIAN) && ! defined(__LITTLE_ENDIAN)
#error no endian defined
#endif
#include	"md_u.h"
#include	"md_p.h"
int Examine(struct mddev_dev *devlist, int brief, int export, int scan,
	    int SparcAdjust, struct supertype *forcest,
	    char *homehost)
{

	/* Read the raid superblock from a device and
	 * display important content.
	 *
	 * If cannot be found, print reason: too small, bad magic
	 *
	 * Print:
	 *   version, ctime, level, size, raid+spare+
	 *   prefered minor
	 *   uuid
	 *
	 *   utime, state etc
	 *
	 * If (brief) gather devices for same array and just print a mdadm.conf line including devices=
	 * if devlist==NULL, use conf_get_devs()
	 */
	int fd;
	int rv = 0;
	int err = 0;

	struct array {
		struct supertype *st;
		struct mdinfo info;
		void *devs;
		struct array *next;
		int spares;
	} *arrays = NULL;

	for (; devlist ; devlist=devlist->next) {
		struct supertype *st;
		int have_container = 0;

		fd = dev_open(devlist->devname, O_RDONLY);
		if (fd < 0) {
			if (!scan) {
				fprintf(stderr,Name ": cannot open %s: %s\n",
					devlist->devname, strerror(errno));
				rv = 1;
			}
			err = 1;
		}
		else {
			int container = 0;
			if (forcest)
				st = dup_super(forcest);
			else if (must_be_container(fd)) {
				/* might be a container */
				st = super_by_fd(fd, NULL);
				container = 1;
			} else
				st = guess_super(fd);
			if (st) {
				err = 1;
				st->ignore_hw_compat = 1;
				if (!container)
					err = st->ss->load_super(st, fd,
								 (brief||scan) ? NULL
								 :devlist->devname);
				if (err && st->ss->load_container) {
					err = st->ss->load_container(st, fd,
								 (brief||scan) ? NULL
								 :devlist->devname);
					if (!err)
						have_container = 1;
				}
				st->ignore_hw_compat = 0;
			} else {
				if (!brief) {
					fprintf(stderr, Name ": No md superblock detected on %s.\n", devlist->devname);
					rv = 1;
				}
				err = 1;
			}
			close(fd);
		}
		if (err)
			continue;

		if (SparcAdjust)
			st->ss->update_super(st, NULL, "sparc2.2",
					     devlist->devname, 0, 0, NULL);
		/* Ok, its good enough to try, though the checksum could be wrong */

		if (brief && st->ss->brief_examine_super == NULL) {
			if (!scan)
				fprintf(stderr, Name ": No brief listing for %s on %s\n",
					st->ss->name, devlist->devname);
		} else if (brief) {
			struct array *ap;
			char *d;
			for (ap=arrays; ap; ap=ap->next) {
				if (st->ss == ap->st->ss &&
				    st->ss->compare_super(ap->st, st)==0)
					break;
			}
			if (!ap) {
				ap = malloc(sizeof(*ap));
				ap->devs = dl_head();
				ap->next = arrays;
				ap->spares = 0;
				ap->st = st;
				arrays = ap;
				st->ss->getinfo_super(st, &ap->info, NULL);
			} else
				st->ss->getinfo_super(st, &ap->info, NULL);
			if (!have_container &&
			    !(ap->info.disk.state & (1<<MD_DISK_SYNC)))
				ap->spares++;
			d = dl_strdup(devlist->devname);
			dl_add(ap->devs, d);
		} else if (export) {
			if (st->ss->export_examine_super)
				st->ss->export_examine_super(st);
			st->ss->free_super(st);
		} else {
			printf("%s:\n",devlist->devname);
			st->ss->examine_super(st, homehost);
			st->ss->free_super(st);
		}
	}
	if (brief) {
		struct array *ap;
		for (ap=arrays; ap; ap=ap->next) {
			char sep='=';
			char *d;
			int newline = 0;

			ap->st->ss->brief_examine_super(ap->st, brief > 1);
			if (ap->spares)
				newline += printf("   spares=%d", ap->spares);
			if (brief > 1) {
				newline += printf("   devices");
				for (d=dl_next(ap->devs); d!= ap->devs; d=dl_next(d)) {
					printf("%c%s", sep, d);
					sep=',';
				}
			}
			if (ap->st->ss->brief_examine_subarrays) {
				if (newline)
					printf("\n");
				ap->st->ss->brief_examine_subarrays(ap->st, brief > 1);
			}
			ap->st->ss->free_super(ap->st);
			/* FIXME free ap */
			if (ap->spares || brief > 1)
				printf("\n");
		}
	}
	return rv;
}

#ifdef QNAPNAS
static void _init_qinfo(QEXAMINE_INFO *qinfo){
	int i =0;
	qinfo->cnt=0;
	qinfo->g_raid_disks=0; //single
	qinfo->g_level=-2; //single
	for(i=0;i<MAX_QDEV;i++){ //init
		qinfo->total_devices[i]=0;
		qinfo->raid_disks[i]=0;
		qinfo->level[i]=-2;
		qinfo->number[i]=-2;
		qinfo->metadata[i]=0;
		strcpy(qinfo->uuid[i], "none");
		strcpy(qinfo->devname[i], "none");
		qinfo->events_hi[i] = 0;
		qinfo->events_lo[i] = 0;

	}
}

static void _qreorder(QEXAMINE_INFO *qinfo){
	int i=0, j=0;
	QEXAMINE_INFO qinfo_tmp;
	char uuid_tmp[64];
	int cnt=0,max_cnt=0;

	//memcpy(&qinfo_tmp, (void*)qinfo, sizeof(QEXAMINE_INFO));

	for (i=0; i<qinfo->cnt; i++){
		cnt=0;
		for (j=0; j<qinfo->cnt; j++){
			if ((i==j) || (!strcmp(qinfo->uuid[i], qinfo->uuid[j]) && i!=j && strcmp(qinfo->devname[i], qinfo->devname[j])))
			{
				cnt++;
			}
			if (max_cnt<cnt){
				max_cnt=cnt;
				strcpy(uuid_tmp, qinfo->uuid[i]);
			}
		}
	}
	if (max_cnt<=0)
		strcpy(uuid_tmp,"not found");
	printf("Dev_Count = %d\n",max_cnt); //including spare disks
	printf("Dev_UUID = %s\n",uuid_tmp);

	_init_qinfo(&qinfo_tmp);

	for (i=0;i<qinfo->cnt;i++){
		if (!strcmp(qinfo->uuid[i], uuid_tmp) && qinfo->number[i]>=0 && qinfo->number[i]<qinfo->raid_disks[i]){ //filter the necessary(active) devices
			if (qinfo_tmp.number[qinfo->number[i]] >= 0)
				continue;//prevent overwriting
			qinfo_tmp.number[qinfo->number[i]]=qinfo->number[i];
			qinfo_tmp.level[qinfo->number[i]]=qinfo->level[i];
			qinfo_tmp.raid_disks[qinfo->number[i]]=qinfo->raid_disks[i];
			qinfo_tmp.metadata[qinfo->number[i]]=qinfo->metadata[i];
			strcpy(qinfo_tmp.uuid[qinfo->number[i]], qinfo->uuid[i]);
			strcpy(qinfo_tmp.devname[qinfo->number[i]], qinfo->devname[i]);
			qinfo_tmp.total_devices[qinfo->number[i]]=qinfo->total_devices[i];
			qinfo_tmp.events_hi[qinfo->number[i]]=qinfo->events_hi[i];
			qinfo_tmp.events_lo[qinfo->number[i]]=qinfo->events_lo[i];
			if (qinfo_tmp.g_raid_disks<=0) qinfo_tmp.g_raid_disks=qinfo->raid_disks[i];
			if (qinfo_tmp.g_level<=-2) qinfo_tmp.g_level=qinfo->level[i];
			qinfo_tmp.cnt++;
		}
	}
	if (qinfo_tmp.cnt>0 && max_cnt>qinfo_tmp.cnt){
		printf("Dev_Spare_Count = %d\n",max_cnt - qinfo_tmp.cnt);
		printf("Dev_Spare_List = ");
		j = 0;
		for (i=0;i<qinfo->cnt;i++){
			if (!strcmp(qinfo->uuid[i],uuid_tmp) && qinfo->number[i]>=qinfo->raid_disks[i]){
					printf("%s",qinfo->devname[i]);
					j++;
					if(j<(max_cnt - qinfo_tmp.cnt))
						printf(",");
					else
						break;
			}
		}
		printf("\n");
	}

	memcpy((void*)qinfo, &qinfo_tmp, sizeof(QEXAMINE_INFO));
}

static void _qdegrade(QEXAMINE_INFO *qinfo){
	int i=0,j=0,swapped=0, degrade_flag=0, max_missed_dev=-1;
	QEXAMINE_INFO qinfo_tmp;
	QEXAMINE_INFO_BUF buf;

	if (qinfo->cnt<=0 || qinfo->g_raid_disks<=0)
		return;
	if(qinfo->g_level==1)
		max_missed_dev=1;//useless, RAID1 might have more than 2 active member devices
	else if(qinfo->g_level==5)
		max_missed_dev=1;
	else if(qinfo->g_level==6)
		max_missed_dev=2;
	else if(qinfo->g_level==0 || qinfo->g_level==-1)
		max_missed_dev=0;
	else
		return;

	memcpy(&qinfo_tmp, (void*)qinfo, sizeof(QEXAMINE_INFO));

	if (qinfo_tmp.cnt >= 2){
		do{
			swapped =0;
			for (i=0;i<MAX_QDEV-1;i++){
				if ((qinfo_tmp.events_hi[i] < qinfo_tmp.events_hi[i+1])
					|| ((qinfo_tmp.events_hi[i] == qinfo_tmp.events_hi[i+1]) && (qinfo_tmp.events_lo[i] < qinfo_tmp.events_lo[i+1]))
					|| (qinfo_tmp.raid_disks[i]<=0 && qinfo_tmp.raid_disks[i+1]>0))
				{
					//swap
					buf.total_devices = qinfo_tmp.total_devices[i];
					qinfo_tmp.total_devices[i] = qinfo_tmp.total_devices[i+1];
					qinfo_tmp.total_devices[i+1] = buf.total_devices;

					buf.raid_disks = qinfo_tmp.raid_disks[i];
					qinfo_tmp.raid_disks[i] = qinfo_tmp.raid_disks[i+1];
					qinfo_tmp.raid_disks[i+1] = buf.raid_disks;

					strcpy(buf.devname, qinfo_tmp.devname[i]);
					strcpy(qinfo_tmp.devname[i], qinfo_tmp.devname[i+1]);
					strcpy(qinfo_tmp.devname[i+1], buf.devname);

					buf.number = qinfo_tmp.number[i];
					qinfo_tmp.number[i] = qinfo_tmp.number[i+1];
					qinfo_tmp.number[i+1] = buf.number;

					buf.events_hi = qinfo_tmp.events_hi[i];
					qinfo_tmp.events_hi[i] = qinfo_tmp.events_hi[i+1];
					qinfo_tmp.events_hi[i+1] = buf.events_hi;

					buf.events_lo = qinfo_tmp.events_lo[i];
					qinfo_tmp.events_lo[i] = qinfo_tmp.events_lo[i+1];
					qinfo_tmp.events_lo[i+1] = buf.events_lo;

					buf.metadata = qinfo_tmp.metadata[i];
					qinfo_tmp.metadata[i] = qinfo_tmp.metadata[i+1];
					qinfo_tmp.metadata[i+1] = buf.metadata;

					swapped = 1;
				}
			}
		}while (swapped);
	}

	for(i=0;i<MAX_QDEV;i++){
		if(qinfo_tmp.raid_disks[i]>0){
			printf("Degrade_Dev_Events%d = %s( %llu.%llu)\n",i,qinfo_tmp.devname[i], qinfo_tmp.events_hi[i], qinfo_tmp.events_lo[i]);
		}
	}

	j=0;
	if(qinfo_tmp.g_level==1 && (qinfo_tmp.cnt >= 1)){
		printf("Degrade_Avail = yes\n");
		printf("Degrade_Dev_List =");
		for(i=0;i<qinfo->g_raid_disks;i++){
			for(j=0;j<1;j++){
				if(qinfo_tmp.raid_disks[j]>0 && !strcmp(qinfo->devname[i],qinfo_tmp.devname[j])){
					printf(" %s",qinfo_tmp.devname[j]);
					degrade_flag = 1;
					break;
				}
			}
			if (!degrade_flag) printf(" missing");
			degrade_flag = 0;
			//if(i<qinfo->g_raid_disks-1)
			//	printf(",");
		}
		printf("\n");
		j=0;
		for(i=0;i<1;i++){
			if(qinfo_tmp.raid_disks[i]>0){
				printf("Degrade_Dev%d = %s\n",qinfo_tmp.number[i],qinfo_tmp.devname[i]);
				j++;
			}
		}
		printf("Degrade_Dev_Count = %d\n",j);
	}
	else if((qinfo_tmp.g_level==5 || qinfo_tmp.g_level==6 || qinfo_tmp.g_level==0 || qinfo_tmp.g_level==-1)
		&& (qinfo_tmp.cnt >= qinfo_tmp.g_raid_disks-max_missed_dev)){ //FIXME
		//check superblocks version, 1.x not test yet, disable it here
		/*
		for(i=0;i<qinfo_tmp.g_raid_disks-max_missed_dev;i++){
			if(qinfo_tmp.metadata[i]>=1.0 ){
				printf("Degrade_Avail = no\n");
				return;
			}
		}
		*/
		//heuristic rules
//		if((qinfo_tmp.metadata[i] < 1.0) && (qinfo_tmp.g_level==5 || qinfo_tmp.g_level==6)){ //check total_devices & events conunt
		if((qinfo_tmp.g_level==5 || qinfo_tmp.g_level==6)){ //check total_devices & events conunt
			if ((qinfo_tmp.total_devices[qinfo_tmp.g_raid_disks-max_missed_dev-1] - qinfo_tmp.total_devices[0]) >= 2
				|| (qinfo_tmp.total_devices[qinfo_tmp.g_raid_disks-max_missed_dev-1] - qinfo_tmp.total_devices[0]) < 0){
				printf("Degrade_Avail = no\n");
				return;
			}/*
			else { //seems uncessary, FIXME
				if (qinfo_tmp.total_devices[qinfo_tmp.g_raid_disks-max_missed_dev-1] != qinfo_tmp.total_devices[0]){
					if ((qinfo_tmp.g_raid_disks-max_missed_dev > 2) && (qinfo_tmp.total_devices[0] != qinfo_tmp.total_devices[1])){
						printf("Degrade_Avail = no\n");
						return;
					}
					else if ((qinfo_tmp.g_raid_disks-max_missed_dev == 2)
									&& (qinfo_tmp.total_devices[0] != qinfo_tmp.total_devices[1])
									&& qinfo_tmp.total_devices[0] == 2){
							printf("Degrade_Avail = no\n"); // FIXME, rebuilding failure
							return;
					}
				}
			}
			*/
		}

		printf("Degrade_Avail = yes\n");
		printf("Degrade_Dev_List =");
		for(i=0;i<qinfo->g_raid_disks;i++){
			for(j=0;j<qinfo_tmp.g_raid_disks-max_missed_dev;j++){
				if(qinfo_tmp.raid_disks[j]>0 && !strcmp(qinfo->devname[i],qinfo_tmp.devname[j])){
					printf(" %s",qinfo_tmp.devname[j]);
					degrade_flag = 1;
					break;
				}
			}
			if (!degrade_flag) printf(" missing");
			degrade_flag = 0;
			//if(i<qinfo->g_raid_disks-max_missed_dev)
			//	printf(",");
		}
		printf("\n");
		j=0;
		for(i=0;i<qinfo_tmp.g_raid_disks-max_missed_dev;i++){
			if(qinfo_tmp.raid_disks[i]>0){
				printf("Degrade_Dev%d = %s\n",qinfo_tmp.number[i],qinfo_tmp.devname[i]);
				j++;
			}
		}
		printf("Degrade_Dev_Count = %d\n",j);
	}
	else
		printf("Degrade_Avail = no\n");

}

int QdevExamine(struct mddev_dev *devlist, int brief, int scan, int SparcAdjust, struct supertype *forcest, char *homehost)
{

	int fd;
	int rv = 0;
	int err = 0;
	int i = 0, j = 0;

	QEXAMINE_INFO qinfo;
	_init_qinfo(&qinfo);

	for (; devlist ; devlist=devlist->next) {
		struct supertype *st = forcest;

		if (qinfo.cnt >= MAX_QDEV)
			break;

		fd = dev_open(devlist->devname, O_RDONLY);
		if (fd < 0) {
			if (!scan) {
				fprintf(stderr,Name ": cannot open %s: %s\n",
					devlist->devname, strerror(errno));
				rv = 1;
			}
			err = 1;
		}
		else {
			if (!st)
				st = guess_super(fd);
			if (st)
				err = st->ss->load_super(st, fd, (brief||scan)?NULL:devlist->devname);
			else {
				if (!brief) {
					fprintf(stderr, Name ": No md superblock detected on %s.\n", devlist->devname);
					rv = 1;
				}
				err = 1;
			}
			close(fd);
		}
		if (err)
			continue;

		if (SparcAdjust)
			st->ss->update_super(st, NULL, "sparc2.2", devlist->devname, 0, 0, NULL);

		printf("[%s]\n",devlist->devname);
		strcpy(qinfo.devname[qinfo.cnt],devlist->devname);

		st->ss->qexamine_super(st, &qinfo);
	}
	printf("[RaidDevResult]\n");
	_qreorder(&qinfo);
	printf("Dev_Level = %d\n",qinfo.g_level);
	printf("Active_Dev_List =");
	j = 0;
	for(i=0;i<MAX_QDEV;i++){
		if(qinfo.raid_disks[i]>0){
			printf("%s ",qinfo.devname[i]);
			j++;
			if(j>=qinfo.cnt)	break;
		}
	}
	printf("\n");

	printf("[RaidDevDegrade]\n");
	_qdegrade(&qinfo);
	printf("\n");

	return rv;
}
#endif

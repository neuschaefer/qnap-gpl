/*
 * Routines to output progress information during a file transfer.
 *
 * Copyright (C) 1996-2000 Andrew Tridgell
 * Copyright (C) 1996 Paul Mackerras
 * Copyright (C) 2001, 2002 Martin Pool <mbp@samba.org>
 * Copyright (C) 2003-2009 Wayne Davison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, visit the http://fsf.org website.
 */

#include "rsync.h"
#ifdef QNAPNAS
#include "Util.h"
extern int qnap_mode;
enum { NORMAL, QRAID1, USB_COPY ,HD_COPY_USB, SNAPSHOT_COPY, SNAPSHOT_COPY_BY_SIZE, SNAPSHOT_RECEIVE_BY_SIZE};
#endif

//--req#1535, Myron Su, v4.1.2, 2014/09/08
//Write speed data to schedule job config
//Working on '--schedule=' option
#ifdef QNAPNAS
char szSpeed[20];
#endif
//[ok]--req#1535, Myron Su, v4.1.2, 2014/09/17

extern int am_server;
extern int need_unsorted_flist;
extern struct stats stats;
extern struct file_list *cur_flist;

#define PROGRESS_HISTORY_SECS 5

#ifdef GETPGRP_VOID
#define GETPGRP_ARG
#else
#define GETPGRP_ARG 0
#endif

struct progress_history {
	struct timeval time;
	OFF_T ofs;
};

int progress_is_active = 0;

static struct progress_history ph_start;
static struct progress_history ph_list[PROGRESS_HISTORY_SECS];
static int newest_hpos, oldest_hpos;
static int current_file_index;

static unsigned long msdiff(struct timeval *t1, struct timeval *t2)
{
	return (t2->tv_sec - t1->tv_sec) * 1000L
	     + (t2->tv_usec - t1->tv_usec) / 1000;
}

static unsigned long long startTime, endTime;
/**
 * @param ofs Current position in file
 * @param size Total size of file
 * @param is_last True if this is the last time progress will be
 * printed for this file, so we should output a newline.  (Not
 * necessarily the same as all bytes being received.)
 **/
static void rprint_progress(OFF_T ofs, OFF_T size, struct timeval *now,
			    int is_last)
{
	char rembuf[64], eol[128];
	const char *units;
	int pct = ofs == size ? 100 : (int) (100.0 * ofs / size);
	unsigned long diff;
	double rate, remain;

	if (is_last) {
		/* Compute stats based on the starting info. */
		if (!ph_start.time.tv_sec
		    || !(diff = msdiff(&ph_start.time, now)))
			diff = 1;
		rate = (double) (ofs - ph_start.ofs) * 1000.0 / diff / 1024.0;
		/* Switch to total time taken for our last update. */
		remain = (double) diff / 1000.0;
	} else {
		/* Compute stats based on recent progress. */
		if (!(diff = msdiff(&ph_list[oldest_hpos].time, now)))
			diff = 1;
		rate = (double) (ofs - ph_list[oldest_hpos].ofs) * 1000.0
		     / diff / 1024.0;
		remain = rate ? (double) (size - ofs) / rate / 1000.0 : 0.0;
	}
	//--Bug#89661 & Bug#96775 Bandwidth usage are inconsistent --
	if (!startTime)
		startTime = Tick_Diff(0);
	double interval_tmp = ((Tick_Diff(0)-startTime)) /1000 + 0.5;
	if (interval_tmp<=0)
		interval_tmp = 1;
	rate = (double) ((stats.total_written/1000) / interval_tmp);
	//--
	if (rate > 1024*1024) {
		rate /= 1024.0 * 1024.0;
		units = "GB/s";
	} else if (rate > 1024) {
		rate /= 1024.0;
		units = "MB/s";
	} else {
		units = "kB/s";
	}

	if (remain < 0)
		strlcpy(rembuf, "  ??:??:??", sizeof rembuf);
	else {
		snprintf(rembuf, sizeof rembuf, "%4d:%02d:%02d",
			 (int) (remain / 3600.0),
			 (int) (remain / 60.0) % 60,
			 (int) remain % 60);
	}

	if (is_last) {
		snprintf(eol, sizeof eol, " (xfer#%d, to-check=%d/%d)\n",
			stats.num_transferred_files,
			stats.num_files - current_file_index - 1,
			stats.num_files);
#ifdef QNAPNAS
		if(qnap_mode == QRAID1){
			Set_Profile_Integer("QRAID1", "Progress", (int)(((current_file_index+1)*100.0)/stats.num_files));
		}
		if(qnap_mode == HD_COPY_USB){
			FILE *fp;
			fp=fopen("/tmp/hdcopyusb_process_log","w+");
			fprintf(fp,"%2.1f%%",(double)(((current_file_index+1)*100.0)/stats.num_files));
			fclose(fp);
		}
#endif
	} else
		strlcpy(eol, "\r", sizeof eol);
	progress_is_active = 0;
	
//--req#1535, Myron Su, v4.1.2, 2014/09/08
//Write speed data to schedule job config
//Working on '--schedule=' option
#ifdef QNAPNAS
	extern char  *pszSchedule;
	if (pszSchedule)
		sprintf(szSpeed, "%.2f%s", rate, units);

	if(qnap_mode == SNAPSHOT_COPY){
		FILE *fp;
		extern char *g_cProgressLog;
		if (g_cProgressLog)
			fp=fopen(g_cProgressLog,"w+");
		else
			fp=fopen("/tmp/snapshotcopy_progress_log","w+");
		if (fp) {
			fprintf(fp,"%2.0f",(double)(((current_file_index+1)*100.0)/stats.num_files));
			fclose(fp);
		}
	}
	else if(qnap_mode == SNAPSHOT_COPY_BY_SIZE){
		FILE *fp;
		extern char *g_cProgressLog;
		if (g_cProgressLog)
			fp=fopen(g_cProgressLog,"w+");
		else
			fp=fopen("/tmp/snapshotcopy_progress_log","w+");
		if (fp) {
                        // Bug 106077, foder size is not caculated in total_size, neglect the size
			if (stats.total_size && stats.total_written<=stats.total_size)
				fprintf(fp,"%d",stats.total_written*100/stats.total_size);
			else
				fprintf(fp,"100");
//			fprintf(fp,"%2.1f%%",(double)((stats.total_written*100.0)/stats.total_size));
			fclose(fp);
		}
	}
	/* used by rsync over ssh to copy file to local; total_written does not increase but total_read does */
	else if(qnap_mode == SNAPSHOT_RECEIVE_BY_SIZE){
		FILE *fp;
		extern char *g_cProgressLog;
		if (g_cProgressLog)
			fp=fopen(g_cProgressLog,"w+");
		else
			fp=fopen("/tmp/snapshotcopy_progress_log","w+");
		if (fp) {
                        // Bug 106077, foder size is not caculated in total_size, neglect the size
			if (stats.total_size && stats.total_read<=stats.total_size)
				fprintf(fp,"%d",stats.total_read*100/stats.total_size);
			else
				fprintf(fp,"100");
			fclose(fp);
		}
	}
#endif
//[ok]--req#1535, Myron Su, v4.1.2, 2014/09/17

	rprintf(FCLIENT, "%12s %3d%% %7.2f%s %s%s",
		human_num(ofs), pct, rate, units, rembuf, eol);
	if (!is_last)
		progress_is_active = 1;
}

void set_current_file_index(struct file_struct *file, int ndx)
{
	if (need_unsorted_flist)
		current_file_index = flist_find(cur_flist, file) + cur_flist->ndx_start;
	else
		current_file_index = ndx;
	current_file_index -= cur_flist->flist_num;
}

void end_progress(OFF_T size)
{
	if (!am_server) {
		struct timeval now;
		gettimeofday(&now, NULL);
		rprint_progress(size, size, &now, True);
	}
	memset(&ph_start, 0, sizeof ph_start);
}

void show_progress(OFF_T ofs, OFF_T size)
{
	struct timeval now;
#if defined HAVE_GETPGRP && defined HAVE_TCGETPGRP
	static pid_t pgrp = -1;
	pid_t tc_pgrp;
#endif

	if (am_server)
		return;

#if defined HAVE_GETPGRP && defined HAVE_TCGETPGRP
	if (pgrp == -1)
		pgrp = getpgrp(GETPGRP_ARG);
#endif

	gettimeofday(&now, NULL);

	if (!ph_start.time.tv_sec) {
		int i;

		/* Try to guess the real starting time when the sender started
		 * to send us data by using the time we last received some data
		 * in the last file (if it was recent enough). */
		if (msdiff(&ph_list[newest_hpos].time, &now) <= 1500) {
			ph_start.time = ph_list[newest_hpos].time;
			ph_start.ofs = 0;
		} else {
			ph_start.time.tv_sec = now.tv_sec;
			ph_start.time.tv_usec = now.tv_usec;
			ph_start.ofs = ofs;
		}

		for (i = 0; i < PROGRESS_HISTORY_SECS; i++)
			ph_list[i] = ph_start;
	}
	else {
		if (msdiff(&ph_list[newest_hpos].time, &now) < 1000)
			return;

		newest_hpos = oldest_hpos;
		oldest_hpos = (oldest_hpos + 1) % PROGRESS_HISTORY_SECS;
		ph_list[newest_hpos].time.tv_sec = now.tv_sec;
		ph_list[newest_hpos].time.tv_usec = now.tv_usec;
		ph_list[newest_hpos].ofs = ofs;
	}

#if defined HAVE_GETPGRP && defined HAVE_TCGETPGRP
	tc_pgrp = tcgetpgrp(STDOUT_FILENO);
	if (tc_pgrp != pgrp && tc_pgrp != -1)
		return;
#endif

	rprint_progress(ofs, size, &now, False);
}


#ifdef	RSYNC_PROGRESS

/// variable to control the progress updating frequency
unsigned long long  smsSent = 0;

/**
 * \brief	Get the time difference between now and the specified reference time in msec.
 * \param	itBase		the reference time base getting by (gettimeofday() / 1000)
 * \return	the time difference in msec
 */
unsigned long long Tick_Diff(unsigned long long itBase)
{
	struct timeval ttNow;
	gettimeofday(&ttNow, NULL);
	return ((ttNow.tv_sec * 1000LL + ttNow.tv_usec / 1000L) - itBase);
}

/**
 * \brief	Update the current rsync progress to the rsync schedule file.
 * \param	msSent		total sent time in msec
 * \param	nbSent		total sent data in byte
 * \param	nbSkip		total skipped data in byte
 * \param	nbTotal		total data to be sent in byte
 */
void Update_Progress(int64 msSent, int64 nbSent, int64 nbSkip, int64 nbTotalRR, int64 nbTotalTT)
{
	int  iRet, iSpeed=0, nsRemain = 0, iProgress = 0;
	int64  nbTotal=nbTotalRR;
	extern int  g_bDbgProg;
	extern char  *pszSchedule;	// current schedule name

	if (nbTotal)
	{
		if (nbTotalTT > nbTotal)  nbTotal = nbTotalTT;
		iSpeed = ((msSent) ? ((nbSent * 1000) / msSent) : (10 * 1024 * 1024));
		nsRemain = (iSpeed) ? (nbTotal - (nbSent + nbSkip)) / iSpeed : 86399L;
		if (100 > (iProgress = (((nbSent + nbSkip) * 100) / nbTotal)))  nsRemain ++;
	}
	// Special handle empty folders
	else if ((-1 == msSent) && !nbSent && !nbSkip)
	{
		msSent = 0;
		iProgress = 100;
		nsRemain = 0;
	}
	// Update progress and remain time to rsync schedule file
	if (pszSchedule)
	{
		char szBuf[64];
		//printf("Schedule name: '%s'\n", pszSchedule);
		sprintf(szBuf, "%d", iProgress);
		iRet = Conf_Set_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_PROGRESS, szBuf);
		if ((SUCCESS != iRet) && g_bDbgProg)  printf("rsync: Can't update rsync conf file 1!\n");
		sprintf(szBuf, "%d", nsRemain);
		iRet = Conf_Set_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_REMAIN_TIME, szBuf);
		if ((SUCCESS != iRet) && g_bDbgProg)  printf("rsync: Can't update rsync conf file 2!\n");

//--req#1535, Myron Su, v4.1.2, 2014/09/08
//Write speed data to schedule job config
//Working on '--schedule=' option
		iRet = Conf_Set_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_SPEED, szSpeed);
		if ((SUCCESS != iRet) && g_bDbgProg)  printf("rsync: Can't update rsync conf speed!\n");
//[ok]--req#1535, Myron Su, v4.1.2, 2014/09/17

		// Debug progress.
		if (g_bDbgProg)  printf("rsync progress: %3d%%, speed:%d KB, remain: %4d sec (%lld/%lld/%lld/%lld)\n", 
			iProgress, iSpeed>>10, nsRemain, nbSent, nbSkip, nbTotalRR, nbTotalTT);
	}
}

/**
 * \brief	Tell progress mechanism a file is going to send.
 * \param	ptStats		the statistic variable of progress
 */
void Begin_Send_File(struct stats *ptStats)
{
	ptStats->msBase = Tick_Diff(0);
}

/**
 * \brief	Tell progress mechanism the file sending progress.
 * \param	ptStats		the statistic variable of progress
 * \param	nbFile		the total size of the file in byte
 * \param	nbSent		the size of the file be sent in byte
 */
void Sending_File(struct stats *ptStats, int64 nbFile, int64 nbSent)
{
	unsigned long long  msSent, msDiff, msSentTotal, nbSentTotal;
	if (MIN_TICK_DIFF_UPDATE > (msDiff = Tick_Diff(smsSent)))  return;
	smsSent += msDiff;
	msSent = Tick_Diff(ptStats->msBase);
	msSentTotal = msSent + ptStats->msSent;
	nbSentTotal = nbSent + ptStats->nbSent;
	Update_Progress(msSentTotal, nbSentTotal, ptStats->nbSkip, ptStats->nbTotal, ptStats->ttStat.cbFiles);
}

/**
 * \brief	Tell progress mechanism a file was sent.
 * \param	ptStats		the statistic variable of progress
 * \param	nbFile		the size of the just sent file in byte
 */
void End_Send_File(struct stats *ptStats, int64 nbFile)
{
	unsigned long long  msSent = Tick_Diff(ptStats->msBase);
	Sending_File(ptStats, nbFile, nbFile);
	ptStats->msSent += msSent;
	ptStats->nfDone ++;
	ptStats->nbSent += nbFile;
}

#endif	//RSYNC_PROGRESS


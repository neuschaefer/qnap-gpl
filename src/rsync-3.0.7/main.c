/*
 * The startup routines, including main(), for rsync.
 *
 * Copyright (C) 1996-2001 Andrew Tridgell <tridge@samba.org>
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
#include "ifuncs.h"
#include "io.h"
#if defined CONFIG_LOCALE && defined HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef QNAPNAS
#include "Util.h"
#endif	//QNAPNAS

#ifdef QTS_SNAPSHOT
#include "storage_man.h"
#include "v2/storage_pool.h"
#include "v2/logical_volume.h"
#include "v2/nas_snapshot.h"
#endif

extern int verbose;
extern int dry_run;
extern int list_only;
extern int am_root;
extern int am_server;
extern int am_sender;
extern int am_daemon;
extern int inc_recurse;
extern int blocking_io;
extern int remove_source_files;
extern int need_messages_from_generator;
extern int kluge_around_eof;
extern int do_stats;
extern int got_xfer_error;
extern int module_id;
extern int copy_links;
extern int copy_dirlinks;
extern int copy_unsafe_links;
extern int keep_dirlinks;
extern int preserve_hard_links;
extern int protocol_version;
extern int file_total;
extern int recurse;
extern int xfer_dirs;
extern int protect_args;
extern int relative_paths;
extern int sanitize_paths;
extern int curr_dir_depth;
extern int curr_dir_len;
extern int module_id;
extern int rsync_port;
extern int whole_file;
extern int read_batch;
extern int write_batch;
extern int batch_fd;
extern int filesfrom_fd;
extern int connect_timeout;
extern pid_t cleanup_child_pid;
extern unsigned int module_dirlen;
extern struct stats stats;
extern char *filesfrom_host;
extern char *partial_dir;
extern char *dest_option;
extern char *basis_dir[];
extern char *rsync_path;
extern char *shell_cmd;
extern char *batch_name;
extern char *password_file;
extern char curr_dir[MAXPATHLEN];
extern struct file_list *first_flist;
extern struct filter_list_struct daemon_filter_list;

#ifdef RSYNC_PROGRESS
extern struct filter_list_struct filter_list;
#endif

#ifdef QNAPNAS
#define NORMAL_LOCK_FILE	"/var/run/rsync-normal.pid"
#define QRAID1_LOCK_FILE	"/var/run/rsync-qraid1.pid"
#define USBCOPY_LOCK_FILE	"/var/run/rsync-usbcopy.pid"
#define HDCOPYUSB_LOCK_FILE	"/var/run/rsync-hdcopyusb.pid"
enum { NORMAL, QRAID1, USB_COPY ,HD_COPY_USB, SNAPSHOT_COPY, SNAPSHOT_COPY_BY_SIZE, SNAPSHOT_RECEIVE_BY_SIZE};
extern int do_progress;
extern int qnap_mode;
extern int do_xfers;
// BUG #13663: mkstemp() will hung on NTFS if the filename with ':' char.
int  g_bNTFS = 0;
// Modified by Jeff Chang 2011/5/4, for service binding
int  g_bHup=False;
/**
 * \brief	The SIGINT signal handler. (Ctrl-C)
 */
static void Signal_Handler_SigHup(int idSig)
{
	g_bHup = True;
}

#ifdef SUPPORT_LIMITRATE
#include <sys/inotify.h>
#include <sys/shm.h>
int g_bDRateChild = FALSE;		//<<< The flag of stop monitering band width of child process.
int g_bRate = FALSE;			//<<< The flag of stop monitering band width.

#ifndef QNAP_CLIENT_LIMITRATE_BY_THREAD
#define SIGRATE	SIGTRAP
/**
 * \brief	The SIGRATE signal handler.
 */
static void Signal_Handler_SigRate(int idSig)
{
	char szSLimitRate[11], szRLimitRate[11];
	extern char *pszSchedule;

	//jeff modify 2012.06.27, if execute rysnc alone but not by QNAP job, we can't set limit rate.(let power users use --bwlimit in command line)
	if(!g_bQnapBwlimit)
		return;
	
	// Read the conf file.
	if (0 != Conf_Get_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNCD_SLIMITRATE, szSLimitRate, sizeof(szSLimitRate)))  strcpy(szSLimitRate, SZV_RSYNCD_SLIMITRATE_DEF);
	if (0 != Conf_Get_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNCD_RLIMITRATE, szRLimitRate, sizeof(szRLimitRate)))  strcpy(szRLimitRate, SZV_RSYNCD_RLIMITRATE_DEF);
	
	am_sender ? (bwlimit = atoi(szSLimitRate) >> 10) : (bwlimit = atoi(szRLimitRate) >> 10);
	
	if (bwlimit) {
		bwlimit_writemax = (size_t)bwlimit * 128;
		if (bwlimit_writemax < 512)
			bwlimit_writemax = 512;
	}
	else
		bwlimit_writemax = 0;
}
#endif /* QNAP_CLIENT_LIMITRATE_BY_THREAD */
#endif /* SUPPORT_LIMITRATE */
#endif	//QNAPNAS



uid_t our_uid;
int am_receiver = 0;  /* Only set to 1 after the receiver/generator fork. */
int am_generator = 0; /* Only set to 1 after the receiver/generator fork. */
int local_server = 0;
int daemon_over_rsh = 0;
mode_t orig_umask = 0;
int batch_gen_fd = -1;

/* There's probably never more than at most 2 outstanding child processes,
 * but set it higher, just in case. */
#ifdef QTS_SNAPSHOT
/* When create snapshot, there will be many child process when manipulate config, so enlarge the pid array.
 * I observed that at most 100 child process shown in remember_children.
 * To prevent the following error
 * [~] # rsync -H -a --sever-mode=0 --sparse --qnap-bwlimit --schedule=Schedule0 --password=rsync --timeout=600 --port=873 /share/CACHEDEV1_DATA/Public/ /share/CACHEDEV1_DATA/Web/
 * rsync: waitpid: No child processes (10)
 * rsync error: waitpid() failed (code 21) at main.c(1572) [sender=3.0.7]
 * [~] #
 * enlarge from 128 to 384 on 20170215
 * */
#define MAXCHILDPROCS 384
#else
/* Bug 118929 - [Mantis#28195#28266][Hybrid Backup Sync] Rsync job enable encryption fail with "No child processes (10)"
 * In the none snapshot model, original MAXCHILDPROCS 7 is not enough when rsync job over ssh.
 * enlarge from 7 to 32 on 20180108
 * */
#define MAXCHILDPROCS 32
#endif

#ifdef HAVE_SIGACTION
# ifdef HAVE_SIGPROCMASK
#  define SIGACTMASK(n,h) SIGACTION(n,h), sigaddset(&sigmask,(n))
# else
#  define SIGACTMASK(n,h) SIGACTION(n,h)
# endif
static struct sigaction sigact;
#endif

#ifdef QTS_SNAPSHOT
struct pid_status {
	pid_t pid;
	int status;
};
    struct pid_status *pid_stat_table = NULL;
    int pid_stat_table_size = 0;
#else
struct pid_status {
	pid_t pid;
	int status;
} pid_stat_table[MAXCHILDPROCS];
#endif

static time_t starttime, endtime;
static int64 total_read, total_written;

static void show_malloc_stats(void);

#ifdef	RSYNC_PROGRESS

#include <pthread.h>

// The link list for string data.
typedef struct _T_STR_NODE_
{
	struct _T_STR_NODE_		*ptNext;
	short					cbData;
	char					szData[0];
} T_STR_NODE, *PT_STR_NODE;

typedef struct _T_TST_PARAM_
{
	PT_TREE_STAT		pttStat;		// The pointer to a tree stat buffer.
	long				bDone;			// The flag to indicate tree scan is done.
	int					*pbAbort;		// The flag to ask thread to abort.
	char				*pszpTree;		// The tree pathes to be scanned.
} T_TST_PARAM, *PT_TST_PARAM;

// Global variable for debuging.
extern int g_bDbgProg;

// The thread structure for tree scan.
pthread_t  g_tThread;

// The parameter struct of tree scan thread.
T_TST_PARAM  g_ttParam;


int Tree_Stat(PT_TREE_STAT pttStat, const char *pszpTree, int *pbAbort)
{
	int  iRet=0, cbPath, cbName, fdFolder;
	char  szpTree[PATH_MAX];
	DIR  *ptFolder=NULL;
	struct stat  tStat;
	struct dirent  *ptDirent;
	PT_STR_NODE  ptsnHead=NULL, ptsnFolder;

	// Check parameters first.
	if (!pttStat || !pszpTree || !pbAbort)  return -EINVAL;
	// Setup the folder path to be scanned.
	cbPath = strlen(pszpTree);
	memcpy(szpTree, pszpTree, cbPath);
	if ('/' != szpTree[cbPath-1])  szpTree[cbPath++] = '/';
	szpTree[cbPath] = 0;

	// Check tree root folder status
	if (0 != stat(szpTree, &tStat))
	{
		iRet = -errno;
		if (g_bDbgProg)  printf("rsync: Can't fstat folder %s error! (%d)\n", szpTree, iRet);
		goto exit_tree_stat;
	}

	// Enumerate all sub-folders.
	while (!*pbAbort)
	{
		if (NULL != (ptFolder = opendir(szpTree)))
		{
			fdFolder = dirfd(ptFolder);
			while (!*pbAbort && (NULL != (ptDirent = readdir(ptFolder))))
			{
				// Check file status
				if (0 != fstatat(fdFolder, ptDirent->d_name, &tStat, AT_SYMLINK_NOFOLLOW))
				{
					iRet = -errno;
					if (g_bDbgProg)  printf("rsync: Can't fstat file %s error! (%d)\n", ptDirent->d_name, iRet);
					continue;
				}
				switch (S_IFMT & tStat.st_mode)
				{
				case S_IFDIR:		// Is a directory?
					// Skip '.' and '..'
					if (('.' == ptDirent->d_name[0]) && (!ptDirent->d_name[1] || (('.' == ptDirent->d_name[1]) && !ptDirent->d_name[2])))  continue;
					//20170704 bug 105648 add handle --exclude
					if (filter_list.head &&
						(check_filter(&filter_list, FLOG, ptDirent->d_name, 1) < 0)) {
						continue;
					}
					pttStat->cnFolders ++;
					pttStat->cnBlocks += tStat.st_blocks;
					// Push a folder node into the link list.
					cbName = strlen(ptDirent->d_name);
					if (NULL == (ptsnFolder = malloc(sizeof(T_STR_NODE)+cbName+1)))
					{
						iRet = -ENOMEM;
						goto exit_tree_stat;
					}
					ptsnFolder->ptNext = ptsnHead;
					ptsnFolder->cbData = cbName;
					memcpy(ptsnFolder->szData, ptDirent->d_name, cbName+1);
					ptsnHead = ptsnFolder;
					break;
				
				case S_IFREG:		// Is a regular file?
				case S_IFLNK:		// Is a symbolic link?
					//20170704 bug 105648 add handle --exclude
					if (filter_list.head &&
						(check_filter(&filter_list, FLOG, ptDirent->d_name, 0) < 0)) {
						rprintf(FINFO, "%s() skip file [%s]\n", __FUNCTION__, ptDirent->d_name);
						continue;
					}
					pttStat->cnFiles ++;
					pttStat->cbFiles += tStat.st_size;
					pttStat->cnBlocks += tStat.st_blocks;
					break;
					
				default:
					break;
				}
			}
			closedir(ptFolder);
			ptFolder = NULL;
		}
		else
		{
			iRet = -errno;
			if (g_bDbgProg)  printf("rsync: Can't opendir %s error! (%d)\n", szpTree, iRet);
		}

		// Pop the stated sub-folder if it is enumerated.
		while (ptsnHead && ('/' == ptsnHead->szData[0]))
		{
			ptsnFolder = ptsnHead;
			ptsnHead = ptsnFolder->ptNext;
			cbPath -= (ptsnFolder->cbData + 1);
			free(ptsnFolder);
		}

		// Enumerate the next sub-folder.
		if (ptsnHead)
		{
			// Reconstruct the tree path.
			memcpy(szpTree+cbPath, ptsnHead->szData, ptsnHead->cbData);
			cbPath += ptsnHead->cbData;
			szpTree[cbPath++] = '/';
			szpTree[cbPath] = 0;
			// mark this sub-folder as enumerated.
			ptsnHead->szData[0] = '/';
		}
		// Break the loop if there are no sub-folder to be enumerated.
		else  break;
	}

exit_tree_stat:
	if (NULL != ptFolder)  closedir(ptFolder);
	while (ptsnHead)
	{
		ptsnFolder = ptsnHead;
		ptsnHead = ptsnFolder->ptNext;
		free(ptsnFolder);
	}
	return iRet;
}

void *Tree_Scan_Thread(void *pParam)
{
	int  iRet=0;
	PT_TST_PARAM  ptParam = (PT_TST_PARAM)pParam;
	if (ptParam && ptParam->pttStat)
	{
		iRet = Tree_Stat(ptParam->pttStat, ptParam->pszpTree, ptParam->pbAbort);
		ptParam->bDone = (0 > iRet) ? 2 : 1;
	}
	if (g_bDbgProg)  printf("rsync: scan tree thread exit: Folder: %lld, File: %lld, Size: %lld, Block: %lld (%d)\n",
		ptParam->pttStat->cnFolders, ptParam->pttStat->cnFiles, ptParam->pttStat->cbFiles, ptParam->pttStat->cnBlocks, iRet);
	pthread_exit(NULL);
}

#endif	//RSYNC_PROGRESS


/* Works like waitpid(), but if we already harvested the child pid in our
 * remember_children(), we succeed instead of returning an error. */
pid_t wait_process(pid_t pid, int *status_ptr, int flags)
{
	pid_t waited_pid;
	
	do {
		waited_pid = waitpid(pid, status_ptr, flags);
	} while (waited_pid == -1 && errno == EINTR);

	if (waited_pid == -1 && errno == ECHILD) {
		/* Status of requested child no longer available:  check to
		 * see if it was processed by remember_children(). */
		int cnt;
#ifdef QTS_SNAPSHOT
		for (cnt = 0; cnt < pid_stat_table_size; cnt++) {
#else
		for (cnt = 0; cnt < MAXCHILDPROCS; cnt++) {
#endif
			if (pid == pid_stat_table[cnt].pid) {
				*status_ptr = pid_stat_table[cnt].status;
				pid_stat_table[cnt].pid = 0;
				return pid;
			}
		}
	}

	return waited_pid;
}

/* Wait for a process to exit, calling io_flush while waiting. */
static void wait_process_with_flush(pid_t pid, int *exit_code_ptr)
{
	pid_t waited_pid;
	int status;

	while ((waited_pid = wait_process(pid, &status, WNOHANG)) == 0) {
		msleep(20);
		io_flush(FULL_FLUSH);
	}

	/* TODO: If the child exited on a signal, then log an
	 * appropriate error message.  Perhaps we should also accept a
	 * message describing the purpose of the child.  Also indicate
	 * this to the caller so that they know something went wrong. */
	if (waited_pid < 0) {
		rsyserr(FERROR, errno, "waitpid");
		*exit_code_ptr = RERR_WAITCHILD;
	} else if (!WIFEXITED(status)) {
#ifdef WCOREDUMP
		if (WCOREDUMP(status))
			*exit_code_ptr = RERR_CRASHED;
		else
#endif
		if (WIFSIGNALED(status))
			*exit_code_ptr = RERR_TERMINATED;
		else
			*exit_code_ptr = RERR_WAITCHILD;
	} else
		*exit_code_ptr = WEXITSTATUS(status);
}

/* This function gets called from all 3 processes.  We want the client side
 * to actually output the text, but the sender is the only process that has
 * all the stats we need.  So, if we're a client sender, we do the report.
 * If we're a server sender, we write the stats on the supplied fd.  If
 * we're the client receiver we read the stats from the supplied fd and do
 * the report.  All processes might also generate a set of debug stats, if
 * the verbose level is high enough (this is the only thing that the
 * generator process and the server receiver ever do here). */
static void handle_stats(int f)
{
	endtime = time(NULL);

	/* Cache two stats because the read/write code can change it. */
	total_read = stats.total_read;
	total_written = stats.total_written;

	if (do_stats && verbose > 1) {
		/* These come out from every process */
		show_malloc_stats();
		show_flist_stats();
	}

	if (am_generator)
		return;

	if (am_daemon) {
		if (f == -1 || !am_sender)
			return;
	}

	if (am_server) {
		if (am_sender) {
			write_varlong30(f, total_read, 3);
			write_varlong30(f, total_written, 3);
			write_varlong30(f, stats.total_size, 3);
			if (protocol_version >= 29) {
				write_varlong30(f, stats.flist_buildtime, 3);
				write_varlong30(f, stats.flist_xfertime, 3);
			}
		}
		return;
	}

	/* this is the client */

	if (f < 0 && !am_sender) /* e.g. when we got an empty file list. */
		;
	else if (!am_sender) {
		/* Read the first two in opposite order because the meaning of
		 * read/write swaps when switching from sender to receiver. */
		total_written = read_varlong30(f, 3);
		total_read = read_varlong30(f, 3);
		stats.total_size = read_varlong30(f, 3);
		if (protocol_version >= 29) {
			stats.flist_buildtime = read_varlong30(f, 3);
			stats.flist_xfertime = read_varlong30(f, 3);
		}
	} else if (write_batch) {
		/* The --read-batch process is going to be a client
		 * receiver, so we need to give it the stats. */
		write_varlong30(batch_fd, total_read, 3);
		write_varlong30(batch_fd, total_written, 3);
		write_varlong30(batch_fd, stats.total_size, 3);
		if (protocol_version >= 29) {
			write_varlong30(batch_fd, stats.flist_buildtime, 3);
			write_varlong30(batch_fd, stats.flist_xfertime, 3);
		}
	}
}

static void output_summary(void)
{
	if (do_stats) {
		rprintf(FCLIENT, "\n");
		rprintf(FINFO,"Number of files: %d\n", stats.num_files);
		rprintf(FINFO,"Number of files transferred: %d\n",
			stats.num_transferred_files);
		rprintf(FINFO,"Total file size: %s bytes\n",
			human_num(stats.total_size));
		rprintf(FINFO,"Total transferred file size: %s bytes\n",
			human_num(stats.total_transferred_size));
		rprintf(FINFO,"Literal data: %s bytes\n",
			human_num(stats.literal_data));
		rprintf(FINFO,"Matched data: %s bytes\n",
			human_num(stats.matched_data));
		rprintf(FINFO,"File list size: %s\n",
			human_num(stats.flist_size));
		if (stats.flist_buildtime) {
			rprintf(FINFO,
				"File list generation time: %.3f seconds\n",
				(double)stats.flist_buildtime / 1000);
			rprintf(FINFO,
				"File list transfer time: %.3f seconds\n",
				(double)stats.flist_xfertime / 1000);
		}
		rprintf(FINFO,"Total bytes sent: %s\n",
			human_num(total_written));
		rprintf(FINFO,"Total bytes received: %s\n",
			human_num(total_read));
	}

	if (verbose || do_stats) {
#ifdef QNAPNAS
		if(do_progress && (qnap_mode == QRAID1))
		{
			Set_Profile_Integer("QRAID1", "Progress", 100);
			Set_Profile_Integer("QRAID1", STATUS_FIELD, S_READY);
		}
		if(do_progress && (qnap_mode == HD_COPY_USB))
		{
			FILE *fp;
			fp=fopen("/tmp/hdcopyusb_process_log","w+");
			fprintf(fp,"100%%");
			fclose(fp);
		}
#endif
		rprintf(FCLIENT, "\n");
		rprintf(FINFO,
			"sent %s bytes  received %s bytes  %s bytes/sec\n",
			human_num(total_written), human_num(total_read),
			human_dnum((total_written + total_read)/(0.5 + (endtime - starttime)), 2));
		rprintf(FINFO, "total size is %s  speedup is %.2f%s\n",
			human_num(stats.total_size),
			(double)stats.total_size / (total_written+total_read),
			write_batch < 0 ? " (BATCH ONLY)" : dry_run ? " (DRY RUN)" : "");
	}

	fflush(stdout);
	fflush(stderr);
}


/**
 * If our C library can get malloc statistics, then show them to FINFO
 **/
static void show_malloc_stats(void)
{
#ifdef HAVE_MALLINFO
	struct mallinfo mi;

	mi = mallinfo();

	rprintf(FCLIENT, "\n");
	rprintf(FINFO, RSYNC_NAME "[%d] (%s%s%s) heap statistics:\n",
		getpid(), am_server ? "server " : "",
		am_daemon ? "daemon " : "", who_am_i());
	rprintf(FINFO, "  arena:     %10ld   (bytes from sbrk)\n",
		(long)mi.arena);
	rprintf(FINFO, "  ordblks:   %10ld   (chunks not in use)\n",
		(long)mi.ordblks);
	rprintf(FINFO, "  smblks:    %10ld\n",
		(long)mi.smblks);
	rprintf(FINFO, "  hblks:     %10ld   (chunks from mmap)\n",
		(long)mi.hblks);
	rprintf(FINFO, "  hblkhd:    %10ld   (bytes from mmap)\n",
		(long)mi.hblkhd);
	rprintf(FINFO, "  allmem:    %10ld   (bytes from sbrk + mmap)\n",
		(long)mi.arena + mi.hblkhd);
	rprintf(FINFO, "  usmblks:   %10ld\n",
		(long)mi.usmblks);
	rprintf(FINFO, "  fsmblks:   %10ld\n",
		(long)mi.fsmblks);
	rprintf(FINFO, "  uordblks:  %10ld   (bytes used)\n",
		(long)mi.uordblks);
	rprintf(FINFO, "  fordblks:  %10ld   (bytes free)\n",
		(long)mi.fordblks);
	rprintf(FINFO, "  keepcost:  %10ld   (bytes in releasable chunk)\n",
		(long)mi.keepcost);
#endif /* HAVE_MALLINFO */
}


/* Start the remote shell.   cmd may be NULL to use the default. */
static pid_t do_cmd(char *cmd, char *machine, char *user, char **remote_argv, int remote_argc,
		    int *f_in_p, int *f_out_p)
{
	int i, argc = 0;
	char *args[MAX_ARGS];
	pid_t pid;
	int dash_l_set = 0;

	if (!read_batch && !local_server) {
		char *t, *f, in_quote = '\0';
		char *rsh_env = getenv(RSYNC_RSH_ENV);
		if (!cmd)
			cmd = rsh_env;
		if (!cmd)
			cmd = RSYNC_RSH;
		cmd = strdup(cmd); /* MEMORY LEAK */
		if (!cmd)
			goto oom;

		for (t = f = cmd; *f; f++) {
			if (*f == ' ')
				continue;
			/* Comparison leaves rooms for server_options(). */
			if (argc >= MAX_ARGS - MAX_SERVER_ARGS)
				goto arg_overflow;
			args[argc++] = t;
			while (*f != ' ' || in_quote) {
				if (!*f) {
					if (in_quote) {
						rprintf(FERROR,
						    "Missing trailing-%c in remote-shell command.\n",
						    in_quote);
						exit_cleanup(RERR_SYNTAX);
					}
					f--;
					break;
				}
				if (*f == '\'' || *f == '"') {
					if (!in_quote) {
						in_quote = *f++;
						continue;
					}
					if (*f == in_quote && *++f != in_quote) {
						in_quote = '\0';
						continue;
					}
				}
				*t++ = *f++;
			}
			*t++ = '\0';
		}

		/* check to see if we've already been given '-l user' in
		 * the remote-shell command */
		for (i = 0; i < argc-1; i++) {
			if (!strcmp(args[i], "-l") && args[i+1][0] != '-')
				dash_l_set = 1;
		}

#ifdef HAVE_REMSH
		/* remsh (on HPUX) takes the arguments the other way around */
		args[argc++] = machine;
		if (user && !(daemon_over_rsh && dash_l_set)) {
			args[argc++] = "-l";
			args[argc++] = user;
		}
#else
		if (user && !(daemon_over_rsh && dash_l_set)) {
			args[argc++] = "-l";
			args[argc++] = user;
		}
		args[argc++] = machine;
#endif

		args[argc++] = rsync_path;

		if (blocking_io < 0) {
			char *cp;
			if ((cp = strrchr(cmd, '/')) != NULL)
				cp++;
			else
				cp = cmd;
			if (strcmp(cp, "rsh") == 0 || strcmp(cp, "remsh") == 0)
				blocking_io = 1;
		}

		server_options(args,&argc);

		if (argc >= MAX_ARGS - 2)
			goto arg_overflow;
	}
	// jeff modify 2012.11.9, for ssh bandwidth limit.
	if(g_bQnapSSHBwlimit)
		args[argc++] = SZ_OPT_QNAP_BWLIMIT;
	
	args[argc++] = ".";

	if (!daemon_over_rsh) {
		while (remote_argc > 0) {
			if (argc >= MAX_ARGS - 1) {
			  arg_overflow:
				rprintf(FERROR, "internal: args[] overflowed in do_cmd()\n");
				exit_cleanup(RERR_SYNTAX);
			}
			args[argc++] = *remote_argv++;
			remote_argc--;
		}
	}

	args[argc] = NULL;

	if (verbose > 3) {
		for (i = 0; i < argc; i++)
			rprintf(FCLIENT, "cmd[%d]=%s ", i, args[i]);
		rprintf(FCLIENT, "\n");
	}

	if (read_batch) {
		int from_gen_pipe[2];
		set_allow_inc_recurse();
		if (fd_pair(from_gen_pipe) < 0) {
			rsyserr(FERROR, errno, "pipe");
			exit_cleanup(RERR_IPC);
		}
		batch_gen_fd = from_gen_pipe[0];
		*f_out_p = from_gen_pipe[1];
		*f_in_p = batch_fd;
		pid = (pid_t)-1; /* no child pid */
#ifdef ICONV_CONST
		setup_iconv();
#endif
	} else if (local_server) {
		/* If the user didn't request --[no-]whole-file, force
		 * it on, but only if we're not batch processing. */
		if (whole_file < 0 && !write_batch)
			whole_file = 1;
		set_allow_inc_recurse();
		pid = local_child(argc, args, f_in_p, f_out_p, child_main);
#ifdef ICONV_CONST
		setup_iconv();
#endif
	} else {
		pid = piped_child(args, f_in_p, f_out_p);
#ifdef ICONV_CONST
		setup_iconv();
#endif
		if (protect_args && !daemon_over_rsh)
			send_protected_args(*f_out_p, args);
	}

	return pid;

  oom:
	out_of_memory("do_cmd");
	return 0; /* not reached */
}

/* The receiving side operates in one of two modes:
 *
 * 1. it receives any number of files into a destination directory,
 * placing them according to their names in the file-list.
 *
 * 2. it receives a single file and saves it using the name in the
 * destination path instead of its file-list name.  This requires a
 * "local name" for writing out the destination file.
 *
 * So, our task is to figure out what mode/local-name we need.
 * For mode 1, we change into the destination directory and return NULL.
 * For mode 2, we change into the directory containing the destination
 * file (if we aren't already there) and return the local-name. */
static char *get_local_name(struct file_list *flist, char *dest_path)
{
	STRUCT_STAT st;
	int statret;
	char *cp;

	if (verbose > 2) {
		rprintf(FINFO, "get_local_name count=%d %s\n",
			file_total, NS(dest_path));
	}

	if (!dest_path || list_only)
		return NULL;

	if (daemon_filter_list.head) {
		char *slash = strrchr(dest_path, '/');
		if (slash && (slash[1] == '\0' || (slash[1] == '.' && slash[2] == '\0')))
			*slash = '\0';
		else
			slash = NULL;
		if ((*dest_path != '.' || dest_path[1] != '\0')
		 && (check_filter(&daemon_filter_list, FLOG, dest_path, 0) < 0
		  || check_filter(&daemon_filter_list, FLOG, dest_path, 1) < 0)) {
			rprintf(FERROR, "skipping daemon-excluded destination \"%s\"\n",
				dest_path);
			exit_cleanup(RERR_FILESELECT);
		}
		if (slash)
			*slash = '/';
	}

	/* See what currently exists at the destination. */
	if ((statret = do_stat(dest_path, &st)) == 0) {
		/* If the destination is a dir, enter it and use mode 1. */
		if (S_ISDIR(st.st_mode)) {
			if (!change_dir(dest_path, CD_NORMAL)) {
				rsyserr(FERROR, errno, "change_dir#1 %s failed",
					full_fname(dest_path));
				exit_cleanup(RERR_FILESELECT);
			}
			return NULL;
		}
		if (file_total > 1) {
			rprintf(FERROR,
				"ERROR: destination must be a directory when"
				" copying more than 1 file\n");
			exit_cleanup(RERR_FILESELECT);
		}
		if (file_total == 1 && S_ISDIR(flist->files[0]->mode)) {
			rprintf(FERROR,
				"ERROR: cannot overwrite non-directory"
				" with a directory\n");
			exit_cleanup(RERR_FILESELECT);
		}
	} else if (errno != ENOENT) {
		/* If we don't know what's at the destination, fail. */
		rsyserr(FERROR, errno, "ERROR: cannot stat destination %s",
			full_fname(dest_path));
		exit_cleanup(RERR_FILESELECT);
	}

	cp = strrchr(dest_path, '/');

	/* If we need a destination directory because the transfer is not
	 * of a single non-directory or the user has requested one via a
	 * destination path ending in a slash, create one and use mode 1. */
	if (file_total > 1 || (cp && !cp[1])) {
		/* Lop off the final slash (if any). */
		if (cp && !cp[1])
			*cp = '\0';

		if (statret == 0) {
			rprintf(FERROR,
			    "ERROR: destination path is not a directory\n");
			exit_cleanup(RERR_SYNTAX);
		}

		if (mkdir_defmode(dest_path) != 0) {
			rsyserr(FERROR, errno, "mkdir %s failed",
				full_fname(dest_path));
			exit_cleanup(RERR_FILEIO);
		}

		if (flist->high >= flist->low
		 && strcmp(flist->files[flist->low]->basename, ".") == 0)
			flist->files[0]->flags |= FLAG_DIR_CREATED;

		if (verbose)
			rprintf(FINFO, "created directory %s\n", dest_path);

		if (dry_run) {
			/* Indicate that dest dir doesn't really exist. */
			dry_run++;
		}

		if (!change_dir(dest_path, dry_run > 1 ? CD_SKIP_CHDIR : CD_NORMAL)) {
			rsyserr(FERROR, errno, "change_dir#2 %s failed",
				full_fname(dest_path));
			exit_cleanup(RERR_FILESELECT);
		}

		return NULL;
	}

	/* Otherwise, we are writing a single file, possibly on top of an
	 * existing non-directory.  Change to the item's parent directory
	 * (if it has a path component), return the basename of the
	 * destination file as the local name, and use mode 2. */
	if (!cp)
		return dest_path;

	if (cp == dest_path)
		dest_path = "/";

	*cp = '\0';
	if (!change_dir(dest_path, CD_NORMAL)) {
		rsyserr(FERROR, errno, "change_dir#3 %s failed",
			full_fname(dest_path));
		exit_cleanup(RERR_FILESELECT);
	}
	*cp = '/';

	return cp + 1;
}

/* This function checks on our alternate-basis directories.  If we're in
 * dry-run mode and the destination dir does not yet exist, we'll try to
 * tweak any dest-relative paths to make them work for a dry-run (the
 * destination dir must be in curr_dir[] when this function is called).
 * We also warn about any arg that is non-existent or not a directory. */
static void check_alt_basis_dirs(void)
{
	STRUCT_STAT st;
	char **dir_p, *slash = strrchr(curr_dir, '/');

	for (dir_p = basis_dir; *dir_p; dir_p++) {
		if (dry_run > 1 && **dir_p != '/') {
			int len = curr_dir_len + 1 + strlen(*dir_p) + 1;
			char *new = new_array(char, len);
			if (!new)
				out_of_memory("check_alt_basis_dirs");
			if (slash && strncmp(*dir_p, "../", 3) == 0) {
			    /* We want to remove only one leading "../" prefix for
			     * the directory we couldn't create in dry-run mode:
			     * this ensures that any other ".." references get
			     * evaluated the same as they would for a live copy. */
			    *slash = '\0';
			    pathjoin(new, len, curr_dir, *dir_p + 3);
			    *slash = '/';
			} else
			    pathjoin(new, len, curr_dir, *dir_p);
			*dir_p = new;
		}
		if (do_stat(*dir_p, &st) < 0) {
			rprintf(FWARNING, "%s arg does not exist: %s\n",
				dest_option, *dir_p);
		} else if (!S_ISDIR(st.st_mode)) {
			rprintf(FWARNING, "%s arg is not a dir: %s\n",
				dest_option, *dir_p);
		}
	}
}

/* This is only called by the sender. */
static void read_final_goodbye(int f_in)
{
	int i, iflags, xlen;
	uchar fnamecmp_type;
	char xname[MAXPATHLEN];

	if (protocol_version < 29)
		i = read_int(f_in);
	else {
		i = read_ndx_and_attrs(f_in, &iflags, &fnamecmp_type,
				       xname, &xlen);
	}

	if (i != NDX_DONE) {
		rprintf(FERROR, "Invalid packet at end of run (%d) [%s]\n",
			i, who_am_i());
		exit_cleanup(RERR_PROTOCOL);
	}
}

#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
/**
 * \brief	Update the band width value from share memory
 * \param	pVoid	The buffer of connection info.
 */
void* Update_LimitRate_Child_Thread(void *pVoid)
{
	int iRet=0, idx=0, idShm=0;
	PT_LIMITRATE piShm=NULL;
	
	// Allocate & attach the share memory for limit rate.
	if (-1 == (idShm = shmget(IKEY_SHM_LIMITRATE_SERVER, NB_SHM_LIMITRATE_SERVER, SHM_R | SHM_W)))
	{
		iRet = -errno;
		rsyserr(FERROR, iRet, "Can't get %d bytes share memory. Rsync daemon doesn't exist but all function will work normally.(%d)\n", NB_SHM_LIMITRATE_SERVER, iRet);
		goto exit_thread;
	}	
	if (NULL == (piShm = (PT_LIMITRATE)shmat(idShm, NULL, 0)))
	{
		iRet = -errno;
		rsyserr(FERROR, iRet, "Can't attach a %d bytes share memory! (%d)\n", NB_SHM_LIMITRATE_SERVER, iRet);
		goto exit_thread;
	}
	
	for (idx=0; idx<CN_BWLIMIT_SLOT; idx++)
	{
		if(-1 == piShm[idx].idPID)
		{
			piShm[idx].idPID = getpid();
			piShm[idx].bSender = am_sender;
			piShm[idx].cbSBwlimit = 0;
			piShm[idx].cbRBwlimit = 0;
			break;
		}
	}
	
	while(!g_bAbort && !g_bDRateChild)
	{
		daemon_bwlimit = bwlimit = am_sender ? (piShm[idx].cbSBwlimit >> 10) : (piShm[idx].cbRBwlimit >> 10);
		bwlimit_writemax = (size_t)bwlimit << 7;
		msleep(1000);
	}
exit_thread:
	if (NULL != piShm)  shmdt(piShm);
	pthread_exit(NULL);
}
#endif /* QNAPNAS && SUPPORT_LIMITRATE */

static void do_server_sender(int f_in, int f_out, int argc, char *argv[])
{
	struct file_list *flist;
	char *dir = argv[0];
	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
	int iRet=0, idx;
	pthread_t thLimitRateIDChild=-1;
	g_bDRateChild = FALSE;
	//jeff modify 2012.06.27, if execute rysnc alone but not by QNAP job, we can't set limit rate.(let power users use --bwlimit in command line)
	if(g_bQnapBwlimit)
	{
		// Launch an additional thread to handle the band width related variables to the share memory.
		if (0 != pthread_create(&thLimitRateIDChild, NULL, Update_LimitRate_Child_Thread, &idx))
		{
			rsyserr(FERROR, errno, "Can't create a thread to moniter limit rate!");
		}
	}
	#endif

	if (verbose > 2) {
		rprintf(FINFO, "server_sender starting pid=%ld\n",
			(long)getpid());
	}

	if (am_daemon && lp_write_only(module_id)) {
		rprintf(FERROR, "ERROR: module is write only\n");
		#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
		iRet = RERR_SYNTAX;
		goto exit_sender;
		#else
		exit_cleanup(RERR_SYNTAX);
		return;
		#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	}
	if (am_daemon && lp_read_only(module_id) && remove_source_files) {
		rprintf(FERROR,
		    "ERROR: --remove-%s-files cannot be used with a read-only module\n",
		    remove_source_files == 1 ? "source" : "sent");
		#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
		iRet = RERR_SYNTAX;
		goto exit_sender;
		#else
		exit_cleanup(RERR_SYNTAX);
		return;
		#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	}

	if (!relative_paths) {
		if (!change_dir(dir, CD_NORMAL)) {
			rsyserr(FERROR, errno, "change_dir#3 %s failed",
				full_fname(dir));
			#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
			iRet = RERR_FILESELECT;
			goto exit_sender;
			#else
			exit_cleanup(RERR_FILESELECT);
			#endif /* QNAPNAS && SUPPORT_LIMITRATE */
		}
	}
	argc--;
	argv++;

	if (argc == 0 && (recurse || xfer_dirs || list_only)) {
		argc = 1;
		argv--;
		argv[0] = ".";
	}

	flist = send_file_list(f_out,argc,argv);
	if (!flist || flist->used == 0)
	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
		goto exit_sender;
	#else
		exit_cleanup(0);
	#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	

	io_start_buffering_in(f_in);

	send_files(f_in, f_out);
	io_flush(FULL_FLUSH);
	handle_stats(f_out);
	if (protocol_version >= 24)
		read_final_goodbye(f_in);
	io_flush(FULL_FLUSH);

	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
exit_sender:
	g_bDRateChild = TRUE;
	if(-1 != (int)thLimitRateIDChild)
		pthread_join(thLimitRateIDChild, NULL);
	exit_cleanup(iRet);
	#else
	exit_cleanup(0);
	#endif /* QNAPNAS && SUPPORT_LIMITRATE */
}

static int do_recv(int f_in, int f_out, char *local_name)
{
	int pid;
	int exit_code = 0;
	int error_pipe[2];
	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
	int iRet=0, idx;
	pthread_t thLimitRateIDChild=-1;
	#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	

	/* The receiving side mustn't obey this, or an existing symlink that
	 * points to an identical file won't be replaced by the referent. */
	copy_links = copy_dirlinks = copy_unsafe_links = 0;

#ifdef SUPPORT_HARD_LINKS
	if (preserve_hard_links && !inc_recurse)
		match_hard_links(first_flist);
#endif

	if (fd_pair(error_pipe) < 0) {
		rsyserr(FERROR, errno, "pipe failed in do_recv");
		exit_cleanup(RERR_IPC);
	}

	io_flush(NORMAL_FLUSH);

	if ((pid = do_fork()) == -1) {
		rsyserr(FERROR, errno, "fork failed in do_recv");
		exit_cleanup(RERR_IPC);
	}

// BUG #13663: mkstemp() will hung on NTFS if the filename with ':' char.
#ifdef QNAPNAS
	{
		#include <sys/vfs.h>
		struct statfs  tStatFS;
		if (0 == statfs(".", &tStatFS))
		{
			g_bNTFS = (0x5346544EL == tStatFS.f_type);
			//printf("!!! FS type: 0x%X, ID: 0x%X, 0x%X\n", (int)tStatFS.f_type, ((int*)(&tStatFS.f_fsid))[0], ((int*)(&tStatFS.f_fsid))[1]);
		}
	}
#endif	//QNAPNAS

	if (pid == 0) {
		// jeff modify 2011.8.26, for limit band width. store the pid of child process.
		#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
		g_bDRateChild = FALSE;
		// jeff modify 2013.11.21, function do_recv is shared by rsync client and daemon. if daemon runs,
		// it should create limit-rate thread to throttle band width as receiving file. if client runs,
		// it'll use do_recv to receive file list "under share folder layer", so we can't create limit-rate
		// thread when rsync is client. But! we can't put the code "create limit-rate thread" in function
		// do_server_recv, cause's we should throttle band width after "fork".
		if (am_daemon && g_bQnapBwlimit)
		{
			// Launch an additional thread to handle the band width related variables to the share memory.
			if (0 != pthread_create(&thLimitRateIDChild, NULL, Update_LimitRate_Child_Thread, &idx))
			{
				rsyserr(FERROR, errno, "Can't create a thread to moniter limit rate!");
			}
		}
		#endif /* QNAPNAS && SUPPORT_LIMITRATE */
		
		am_receiver = 1;

		close(error_pipe[0]);
		if (f_in != f_out)
			close(f_out);

		/* we can't let two processes write to the socket at one time */
		io_end_multiplex_out();

		/* set place to send errors */
		set_msg_fd_out(error_pipe[1]);
		io_start_buffering_out(error_pipe[1]);

		recv_files(f_in, local_name);
		io_flush(FULL_FLUSH);
		handle_stats(f_in);

		send_msg(MSG_DONE, "", 1, 0);
		write_varlong(error_pipe[1], stats.total_read, 3);
		io_flush(FULL_FLUSH);

		/* Handle any keep-alive packets from the post-processing work
		 * that the generator does. */
		if (protocol_version >= 29) {
			int iflags, xlen;
			uchar fnamecmp_type;
			char xname[MAXPATHLEN];

			kluge_around_eof = -1;

			/* This should only get stopped via a USR2 signal. */
			read_ndx_and_attrs(f_in, &iflags, &fnamecmp_type,
					   xname, &xlen);

			rprintf(FERROR, "Invalid packet at end of run [%s]\n",
				who_am_i());
			#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
			iRet = RERR_PROTOCOL;
			goto exit_child;
			#else
			exit_cleanup(RERR_PROTOCOL);
			#endif /* QNAPNAS && SUPPORT_LIMITRATE */
			
		}
		#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
exit_child:
		g_bDRateChild = TRUE;
		if(-1 != (int)thLimitRateIDChild)
			pthread_join(thLimitRateIDChild, NULL);
		if(0 != iRet)
			exit_cleanup(iRet);
		#endif /* QNAPNAS && SUPPORT_LIMITRATE */

		/* Finally, we go to sleep until our parent kills us with a
		 * USR2 signal.  We sleep for a short time, as on some OSes
		 * a signal won't interrupt a sleep! */
		while (1)
			msleep(20);
	}

	am_generator = 1;

	io_end_multiplex_in();
	if (write_batch && !am_server)
		stop_write_batch();

	close(error_pipe[1]);
	if (f_in != f_out)
		close(f_in);

	io_start_buffering_out(f_out);

	set_msg_fd_in(error_pipe[0]);
	io_start_buffering_in(error_pipe[0]);

#ifdef SUPPORT_HARD_LINKS
	if (preserve_hard_links && inc_recurse) {
		struct file_list *flist;
		for (flist = first_flist; flist; flist = flist->next)
			match_hard_links(flist);
	}
#endif

	generate_files(f_out, local_name);

	handle_stats(-1);
	io_flush(FULL_FLUSH);
	if (protocol_version >= 24) {
		/* send a final goodbye message */
		write_ndx(f_out, NDX_DONE);
	}
	io_flush(FULL_FLUSH);

	set_msg_fd_in(-1);
	kill(pid, SIGUSR2);
	wait_process_with_flush(pid, &exit_code);
	return exit_code;
}

static void do_server_recv(int f_in, int f_out, int argc, char *argv[])
{
	int exit_code;
	struct file_list *flist;
	char *local_name = NULL;
	int save_verbose = verbose;

	if (filesfrom_fd >= 0) {
		/* We can't mix messages with files-from data on the socket,
		 * so temporarily turn off verbose messages. */
		verbose = 0;
	}

	if (verbose > 2) {
		rprintf(FINFO, "server_recv(%d) starting pid=%ld\n",
			argc, (long)getpid());
	}

	if (am_daemon && lp_read_only(module_id)) {
		rprintf(FERROR,"ERROR: module is read only\n");
		exit_cleanup(RERR_SYNTAX);
		return;
	}

	if (argc > 0) {
		char *dir = argv[0];
		argc--;
		argv++;
		if (!am_daemon && !change_dir(dir, CD_NORMAL)) {
			rsyserr(FERROR, errno, "change_dir#4 %s failed",
				full_fname(dir));
			exit_cleanup(RERR_FILESELECT);
		}
	}

	if (protocol_version >= 30)
		io_start_multiplex_in();
	else
		io_start_buffering_in(f_in);
	recv_filter_list(f_in);

	if (filesfrom_fd >= 0) {
		/* We need to send the files-from names to the sender at the
		 * same time that we receive the file-list from them, so we
		 * need the IO routines to automatically write out the names
		 * onto our f_out socket as we read the file-list.  This
		 * avoids both deadlock and extra delays/buffers. */
		io_set_filesfrom_fds(filesfrom_fd, f_out);
		filesfrom_fd = -1;
	}

	flist = recv_file_list(f_in);
	if (!flist) {
		rprintf(FERROR,"server_recv: recv_file_list error\n");
		exit_cleanup(RERR_FILESELECT);
	}
	if (inc_recurse && file_total == 1)
		recv_additional_file_list(f_in);
	verbose = save_verbose;

	if (argc > 0)
		local_name = get_local_name(flist,argv[0]);

	/* Now that we know what our destination directory turned out to be,
	 * we can sanitize the --link-/copy-/compare-dest args correctly. */
	if (sanitize_paths) {
		char **dir_p;
		for (dir_p = basis_dir; *dir_p; dir_p++)
			*dir_p = sanitize_path(NULL, *dir_p, NULL, curr_dir_depth, SP_DEFAULT);
		if (partial_dir)
			partial_dir = sanitize_path(NULL, partial_dir, NULL, curr_dir_depth, SP_DEFAULT);
	}
	check_alt_basis_dirs();

	if (daemon_filter_list.head) {
		char **dir_p;
		struct filter_list_struct *elp = &daemon_filter_list;

		for (dir_p = basis_dir; *dir_p; dir_p++) {
			char *dir = *dir_p;
			if (*dir == '/')
				dir += module_dirlen;
			if (check_filter(elp, FLOG, dir, 1) < 0)
				goto options_rejected;
		}
		if (partial_dir && *partial_dir == '/'
		 && check_filter(elp, FLOG, partial_dir + module_dirlen, 1) < 0) {
		    options_rejected:
			rprintf(FERROR,
				"Your options have been rejected by the server.\n");
			exit_cleanup(RERR_SYNTAX);
		}
	}

	exit_code = do_recv(f_in, f_out, local_name);
	exit_cleanup(exit_code);
}


int child_main(int argc, char *argv[])
{
	start_server(STDIN_FILENO, STDOUT_FILENO, argc, argv);
	return 0;
}

#ifdef QTS_SNAPSHOT
//input decryption_path, encryption_path_size
//output encryption_path
static void path_dec2enc_folder(IN char* decryption_path, OUT char* encryption_path,IN int encryption_path_size)
{
        int last_match_idx=0;
        int i=0, len=strlen(decryption_path);
        char *folder_name;
        
        for(i=0;i<len;i++)
        {
            if(decryption_path[i]=='/')            
            {                
                last_match_idx=i;            
            }
        }
        folder_name = decryption_path;
        folder_name = folder_name+ last_match_idx +1;
        
        strncpy(encryption_path, decryption_path, last_match_idx+1);
        encryption_path[last_match_idx+1]='\0';
        strcat(encryption_path, ".__eN__");
        strcat(encryption_path, folder_name);
}

//input share path
//0:no, 1:yes
static int isFolderEncryption_self(char *share_path)
{
    char tmp_share_path[256];
    char enc_path[256];
    int ret =0;
    struct stat st;
    snprintf(tmp_share_path, sizeof(tmp_share_path), "%s", share_path);
     
    if(tmp_share_path[strlen(tmp_share_path)-1]=='/')  //end with 
            tmp_share_path[strlen(tmp_share_path)-1]='\0';
    

    path_dec2enc_folder(tmp_share_path, enc_path, sizeof(enc_path));


    if (stat(enc_path, &st) == 0)
        ret =1 ;
    
    return ret;
}

#endif


void start_server(int f_in, int f_out, int argc, char *argv[])
{
	set_nonblocking(f_in);
	set_nonblocking(f_out);

	io_set_sock_fds(f_in, f_out);
	setup_protocol(f_out, f_in);

	if (protocol_version >= 23)
		io_start_multiplex_out();

	if (am_sender) {
		keep_dirlinks = 0; /* Must be disabled on the sender. */
		if (need_messages_from_generator)
			io_start_multiplex_in();
		recv_filter_list(f_in);
		do_server_sender(f_in, f_out, argc, argv);
	} else
		do_server_recv(f_in, f_out, argc, argv);
	exit_cleanup(0);
}


/*
 * This is called once the connection has been negotiated.  It is used
 * for rsyncd, remote-shell, and local connections.
 */
int client_run(int f_in, int f_out, pid_t pid, int argc, char *argv[])
{
	struct file_list *flist = NULL;
	int exit_code = 0, exit_code2 = 0;
	char *local_name = NULL;
#ifdef QNAPNAS
	int ret;
#endif

#ifdef QTS_SNAPSHOT
        int qret;
        int bUseSnapshot;
        int i, j, qpid;
        int vol_id, snapshot_id=0;
        char *path = NULL;
        char *new_path = NULL;
        char *share_path = NULL;
        char *pszSlash = NULL, *pszShare = NULL;
        char snapshot_path[256];
        Snapshot_Create_CONFIG conf;
        int *vol_idAry = NULL;
        int *snapshot_idAry = NULL;
#endif

	cleanup_child_pid = pid;
	if (!read_batch) {
		set_nonblocking(f_in);
		set_nonblocking(f_out);
	}

	io_set_sock_fds(f_in, f_out);
	setup_protocol(f_out,f_in);

	/* We set our stderr file handle to blocking because ssh might have
	 * set it to non-blocking.  This can be particularly troublesome if
	 * stderr is a clone of stdout, because ssh would have set our stdout
	 * to non-blocking at the same time (which can easily cause us to lose
	 * output from our print statements).  This kluge shouldn't cause ssh
	 * any problems for how we use it.  Note also that we delayed setting
	 * this until after the above protocol setup so that we know for sure
	 * that ssh is done twiddling its file descriptors.  */
	set_blocking(STDERR_FILENO);

	if (am_sender) {

#ifdef	RSYNC_PROGRESS
		extern char  *pszSchedule;
		if (pszSchedule)
		{
			// Create a report sender thread.
			if (0 <= argc) g_ttParam.pszpTree = argv[0];
			if (0 != pthread_create(&g_tThread, NULL, Tree_Scan_Thread, &g_ttParam))
			{
				if (g_bDbgProg)  printf("rsync: Can't create scan tree thread error! (%d)\n", -errno);
				g_ttParam.bDone = 2;
			}
		}
#endif	//RSYNC_PROGRESS

		keep_dirlinks = 0; /* Must be disabled on the sender. */
		if (protocol_version >= 30)
			io_start_multiplex_out();
		else
			io_start_buffering_out(f_out);
		if (!filesfrom_host)
			set_msg_fd_in(f_in);
		send_filter_list(f_out);
		if (filesfrom_host)
			filesfrom_fd = f_in;

		if (write_batch && !am_server)
			start_write_batch(f_out);
#ifdef QTS_SNAPSHOT
//take snapshot and change argv path here
		bUseSnapshot = g_bSnapshot;
		if (!bUseSnapshot)
                        goto EXIT_MALLOC_FAIL;
		bUseSnapshot = NAS_Snapshot_Is_Supported();
		if (!bUseSnapshot)
                        goto EXIT_MALLOC_FAIL;
                vol_idAry = malloc(sizeof(int)*argc);
                snapshot_idAry = malloc(sizeof(int)*argc);
                if (!vol_idAry || !snapshot_idAry) {
                        bUseSnapshot = 0;
			if(vol_idAry) { free(vol_idAry); vol_idAry = NULL; }
			if(snapshot_idAry) { free(snapshot_idAry); snapshot_idAry = NULL; }
                        goto EXIT_MALLOC_FAIL;
                }

                memset(vol_idAry, 0, sizeof(int)*argc);
                memset(snapshot_idAry, 0, sizeof(int)*argc);

                for (i=0; i<argc; i++) {
                        path = NULL;
                        share_path = NULL;
                        path = malloc(sizeof(char)*(strlen(argv[i])+1));
                        share_path = malloc(sizeof(char)*(strlen(argv[i])+1));

                        if (path && share_path) {
                                strcpy(path, argv[i]);
                                if(isFolderEncryption_self(path) == 1) //it is a encryption share folder not take snapshot
                                    continue;
                                if (path[strlen(path)-1] == '/')
                                        path[strlen(path)-1] = '\0';
                                //20171110 bug 114787 sub folder pair not got snapshot.
                                pszShare = strstr(path,"/share/");
                                if (pszShare && strlen(path) > 7) {
                                        pszShare += 7;
                                } else {
                                        pszShare = path;
                                }
                                if(NULL != (pszSlash = strchr(pszShare, '/'))) {
                                        strcpy(share_path, (pszSlash+1));
                                        *pszSlash = '\0';
                                }
                                rprintf(FINFO,"snapshot volume path[%s]\n", path);
                                if (!Volume_Get_Vol_ID_By_Mount_Path(path, &vol_id) && NAS_Snapshot_Volume_Is_Supported(vol_id)) {
                                        vol_idAry[i] = vol_id;
                                } else {//vol_id==0; not to take a snapshot
					rprintf(FINFO,"not to take a snapshot\n");
					if(path) { free(path); path = NULL; }
					if(share_path) { free(share_path); share_path = NULL; }
					continue;
				}

                                memset(&conf, 0, sizeof(Snapshot_Create_CONFIG));
#ifdef	RSYNC_PROGRESS
				if (pszSchedule)
					snprintf(conf.name, sizeof(conf.name), "Replication-%s", pszSchedule);
				else
#endif
					snprintf(conf.name, sizeof(conf.name), "Replication-%s", "Job");

                                snapshot_id = 0;
                                for (j=0; j<i; j++) { //search previous vol_id and snapshot_id
                                        if ((vol_idAry[j]==vol_id) && (snapshot_idAry[j]!=0)) { //got previously used snapshot; reuse it
                                                snapshot_id = snapshot_idAry[j];
                                                break;
                                        }
                                }
                                if (!snapshot_id) {
                                        snapshot_id = NAS_Snapshot_Create_For_App(vol_id, &conf);
                                        if (snapshot_id > 0) { //create and then set config
                                                qpid = getpid();
						qret = NAS_Snapshot_Mount(snapshot_id);
						qret |= NAS_Snapshot_Mount_Msg(snapshot_id, MSG_JB);
                                                qret |= Snapshot_Set_PID(snapshot_id, qpid);
                                                snapshot_idAry[i] = snapshot_id;
                                        }
                                }

                                if (snapshot_id>0 && qret==0) { //path to snapshot path
                                        NAS_Snapshot_Get_Preview_Path(snapshot_id, snapshot_path, sizeof(snapshot_path));
					new_path = NULL;
                                        new_path = malloc(sizeof(char)*(strlen(snapshot_path)+strlen(share_path)+3));
					if (new_path) {
	                                        sprintf(new_path, "%s/%s/", snapshot_path, share_path);
        	                                free(argv[i]);
                	                        argv[i] = strdup(new_path);
					}
                                }
                        }
                        if(path) { free(path); path = NULL; }
                        if(share_path) { free(share_path); share_path = NULL; }
                        if(new_path) {free(new_path); new_path = NULL;}
                }
EXIT_MALLOC_FAIL:
#endif
		flist = send_file_list(f_out, argc, argv);
		if (verbose > 3)
			rprintf(FINFO,"file list sent\n");

		if (protocol_version >= 23)
			io_start_multiplex_in();

		io_flush(NORMAL_FLUSH);

#ifdef QNAPNAS
		ret = send_files(f_in, f_out);
#else
		send_files(f_in, f_out);
#endif

#ifdef QTS_SNAPSHOT
//delete snapshot here
		if (bUseSnapshot) {
			for (i=0; i<argc; i++) {
				snapshot_id = snapshot_idAry[i];
				//Strange!!! The last snapshot will not be deleted; the process is killed at that time;However, qsnapman will delete unused snapshot
				if (snapshot_id > 0)
					Snapshot_Set_Expiremin(snapshot_id, 1);
			}
			if(vol_idAry) { free(vol_idAry); vol_idAry = NULL; }
			if(snapshot_idAry) { free(snapshot_idAry); snapshot_idAry = NULL; }
		}
#endif

		io_flush(FULL_FLUSH);
		handle_stats(-1);
		if (protocol_version >= 24)
			read_final_goodbye(f_in);
		if (pid != -1) {
			if (verbose > 3)
				rprintf(FINFO,"client_run waiting on %d\n", (int) pid);
			io_flush(FULL_FLUSH);
			wait_process_with_flush(pid, &exit_code);
		}
		output_summary();
		io_flush(FULL_FLUSH);
#ifdef QNAPNAS
		if(!do_xfers)
			return ret;
		else
#endif
		exit_cleanup(exit_code);
	}

	if (!read_batch) {
		if (protocol_version >= 23)
			io_start_multiplex_in();
		if (need_messages_from_generator)
			io_start_multiplex_out();
	}

	send_filter_list(read_batch ? -1 : f_out);

	if (filesfrom_fd >= 0) {
		io_set_filesfrom_fds(filesfrom_fd, f_out);
		filesfrom_fd = -1;
	}

	if (write_batch && !am_server)
		start_write_batch(f_in);
	flist = recv_file_list(f_in);
	if (inc_recurse && file_total == 1)
		recv_additional_file_list(f_in);

	if (flist && flist->used > 0) {
		local_name = get_local_name(flist, argv[0]);

		check_alt_basis_dirs();

		exit_code2 = do_recv(f_in, f_out, local_name);
	} else {
		handle_stats(-1);
		output_summary();
	}

	if (pid != -1) {
		if (verbose > 3)
			rprintf(FINFO,"client_run2 waiting on %d\n", (int) pid);
		io_flush(FULL_FLUSH);
		wait_process_with_flush(pid, &exit_code);
	}

	return MAX(exit_code, exit_code2);
}

static int copy_argv(char *argv[])
{
	int i;

	for (i = 0; argv[i]; i++) {
		if (!(argv[i] = strdup(argv[i]))) {
			rprintf (FERROR, "out of memory at %s(%d)\n",
				 __FILE__, __LINE__);
			return RERR_MALLOC;
		}
	}

	return 0;
}

#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
#ifdef QNAP_CLIENT_LIMITRATE_BY_THREAD
/**
 * \brief	Update the band width value from config file
 * \param	pVoid	The buffer of daemon info.
 */
void* Update_LimitRate_Client_Thread(void *pVoid)
{
	#define MSEC_PER_SECOND 1000
	#define CBMAX_CONF_PATH 128
	int iRet, mfMask=IN_ONESHOT|IN_MOVE|IN_DELETE|IN_MODIFY, fdiNotify=-1, idNotify=-1, bDirty=FALSE;
	char  szBufVnt[sizeof(struct inotify_event)+CBMAX_CONF_PATH], szSLimitRate[11], szRLimitRate[11];
	//char *pszSchedule = (char *)pVoid;
	struct inotify_event  *ptinEvent=(struct inotify_event*)&szBufVnt;
	struct stat  tStat;
	time_t itModify=0;
	
	// Initial the inotify first.
	if (-1 == (fdiNotify = inotify_init()))
	{
		iRet = -errno;
		rsyserr(FERROR, errno, "Can't init inotify device file! (%d)", iRet);
		goto exit_thread;
	}
	// Add inotify watch to this conf file.
	if (-1 == (idNotify = inotify_add_watch(fdiNotify, RSYNC_SCHEDULE_CONF, mfMask)))
	{
		iRet = -errno;
		rsyserr(FERROR, errno, "Can't add an inotify watch of \"%s\" error! (%d)", RSYNC_SCHEDULE_CONF, iRet);
		goto exit_thread;
	}

	// The main loop of QSync monitor.
	while (!g_bAbort && !g_bRate)
	{
		// Check if there is an inotify event!
		if (0 < (iRet = Is_FD_Ready_To_Read(fdiNotify, MSEC_PER_SECOND)))
		{
			memset(szBufVnt, 0, sizeof(szBufVnt));
			if (-1 == read(fdiNotify, ptinEvent, sizeof(szBufVnt)))
			{
				iRet = -errno;
				rsyserr(FERROR, errno, "Can't read the inotify event error! (%d)\n", iRet);
				break;
			}
			//DEBUG_PRINTF("inotify - wd: %d, mask: 0x%X, cookie: 0x%X, len: %d, name: %s\n", ptinEvent->wd, ptinEvent->mask, ptinEvent->cookie, ptinEvent->len, ptinEvent->name);

			// Check if conf file is modified.
			if (ptinEvent->wd == idNotify)
			{
				bDirty = TRUE;
				if (IN_IGNORED & ptinEvent->mask)  idNotify = -1;
				//DEBUG_PRINTF("%s is modified!\n", ptsfConf->szpConf);
			}
			continue;
		}
		// Catched a signal?
		else if (-EINTR == iRet)
		{
			// Don't exit if the signal is not an INT event.
			if (-EINTR != g_bAbort)
			{
				iRet = 0;
				continue;
			}
			//DEBUG_PRINTF("Catch a INT signal!\n");
			break;
		}
		// Encounter error!
		else if (0 > iRet)
		{
			if(-EINTR == iRet)
			rsyserr(FERROR, iRet, "Check inotify device error! (%d)\n", iRet);
			break;
		}
		// No inotify event is found till timeout.
		else
		{
			// In case the inotify mechanism failed, we also check the modified time of the conf files.
			stat(RSYNC_SCHEDULE_CONF, &tStat);
			// In case the inotify mechanism failed, we also check the modified time of the conf file.
			if (!bDirty)
			{
				// Check if the modified time of the conf file was changed or not.
				if (tStat.st_mtime != itModify)
				{
					if ((-1 != idNotify) && (-1 == inotify_rm_watch(fdiNotify, idNotify)))
					{
						//rsyserr(FERROR, errno, "Can't remove the inotify watch of \"%s\" %d error! (%d)\n", RSYNC_SCHEDULE_CONF, idNotify, -errno);
					}
					idNotify = -1;
					bDirty = TRUE;
				}
			}
			itModify = tStat.st_mtime;
		}
		if(TRUE == bDirty)
		{
			// Read the conf file.
			if (0 != Conf_Get_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNCD_SLIMITRATE, szSLimitRate, sizeof(szSLimitRate)))  strcpy(szSLimitRate, SZV_RSYNCD_SLIMITRATE_DEF);
			if (0 != Conf_Get_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNCD_RLIMITRATE, szRLimitRate, sizeof(szRLimitRate)))  strcpy(szRLimitRate, SZV_RSYNCD_RLIMITRATE_DEF);

			// Add inotify watch to this conf file.
			if (-1 == (idNotify = inotify_add_watch(fdiNotify, RSYNC_SCHEDULE_CONF, mfMask)))
			{
				iRet = -errno;
				rsyserr(FERROR, iRet, "Can't add an inotify watch of \"%s\" error! (%d)\n", RSYNC_SCHEDULE_CONF, iRet);
				goto exit_thread;
			}
			bDirty = FALSE;
		}
		
		am_sender ? (bwlimit = atoi(szSLimitRate) >> 10) : (bwlimit = atoi(szRLimitRate) >> 10);
		
		if (bwlimit) {
			bwlimit_writemax = (size_t)bwlimit * 128;
			if (bwlimit_writemax < 512)
				bwlimit_writemax = 512;
		}
		else
			bwlimit_writemax = 0;
	}
exit_thread:
	if(fdiNotify != -1)
	{
		if(idNotify != -1)
			inotify_rm_watch(fdiNotify, idNotify);
		close(fdiNotify);
	}
	pthread_exit(NULL);
}
#else
/**
 * \brief	Fork a process to monitor limit rate.
 * \param	pszSchedule	Schedule file to monitor.
 * \return	pid of child process.
 */
int Fork_Monitor_LimitRate(char *pszSchedule)
{
	int pid=-1;
	pid = fork();
	
	//child
	if(0 == pid)
	{
		#define MSEC_PER_SECOND 1000
		#define CBMAX_CONF_PATH 128
		int iRet=0, mfMask=IN_ONESHOT|IN_MOVE|IN_DELETE|IN_MODIFY, fdiNotify=-1, idNotify=-1, bDirty=FALSE, idPPid=-1;
		char  szBufVnt[sizeof(struct inotify_event)+CBMAX_CONF_PATH], szSLimitRate[11]={0}, szRLimitRate[11]={0}, szSLimitOld[11]={0}, szRLimitOld[11]={0};
		//char *pszSchedule = (char *)pVoid;
		struct inotify_event  *ptinEvent=(struct inotify_event*)&szBufVnt;
		struct stat  tStat;
		time_t itModify=0;

		idPPid = getppid();

		if (0 >= idPPid)
		{
			iRet = ESRCH;
			goto exit_child;
		}
		if (0 != (iRet = kill(idPPid, 0)))
			goto exit_child;
		
		// Initial the inotify first.
		if (-1 == (fdiNotify = inotify_init()))
		{
			iRet = errno;
			rsyserr(FERROR, errno, "Can't init inotify device file! (%d)", iRet);
			goto exit_child;
		}
		// Add inotify watch to this conf file.
		if (-1 == (idNotify = inotify_add_watch(fdiNotify, RSYNC_SCHEDULE_CONF, mfMask)))
		{
			iRet = errno;
			rsyserr(FERROR, errno, "Can't add an inotify watch of \"%s\" error! (%d)", RSYNC_SCHEDULE_CONF, iRet);
			goto exit_child;
		}

		// The main loop of QSync monitor.
		while (!g_bAbort && !g_bRate)
		{
			// if parent died, child exits.
			if (0 != (iRet = kill(idPPid, 0)))
				goto exit_child;
			
			// Check if there is an inotify event!
			if (0 < (iRet = Is_FD_Ready_To_Read(fdiNotify, MSEC_PER_SECOND)))
			{
				memset(szBufVnt, 0, sizeof(szBufVnt));
				if (-1 == read(fdiNotify, ptinEvent, sizeof(szBufVnt)))
				{
					iRet = errno;
					rsyserr(FERROR, errno, "Can't read the inotify event error! (%d)\n", iRet);
					break;
				}
				//DEBUG_PRINTF("inotify - wd: %d, mask: 0x%X, cookie: 0x%X, len: %d, name: %s\n", ptinEvent->wd, ptinEvent->mask, ptinEvent->cookie, ptinEvent->len, ptinEvent->name);

				// Check if conf file is modified.
				if (ptinEvent->wd == idNotify)
				{
					bDirty = TRUE;
					if (IN_IGNORED & ptinEvent->mask)  idNotify = -1;
					//DEBUG_PRINTF("%s is modified!\n", ptsfConf->szpConf);
				}
				continue;
			}
			// Catched a signal?
			else if (EINTR == iRet)
			{
				// Don't exit if the signal is not an INT event.
				if (EINTR != g_bAbort)
				{
					iRet = 0;
					continue;
				}
				//DEBUG_PRINTF("Catch a INT signal!\n");
				break;
			}
			// Encounter error!
			else if (0 > iRet)
			{
				if(EINTR == iRet)
				rsyserr(FERROR, iRet, "Check inotify device error! (%d)\n", iRet);
				break;
			}
			// No inotify event is found till timeout.
			else
			{
				stat(RSYNC_SCHEDULE_CONF, &tStat);
				// In case the inotify mechanism failed, we also check the modified time of the conf file.
				if (!bDirty)
				{
					// Check if the modified time of the conf file was changed or not.
					if (tStat.st_mtime != itModify)
					{				
						if ((-1 != idNotify) && (-1 == inotify_rm_watch(fdiNotify, idNotify)))
						{
							//rsyserr(FERROR, errno, "Can't remove the inotify watch of \"%s\" %d error! (%d)\n", RSYNC_SCHEDULE_CONF, idNotify, -errno);
						}
						idNotify = -1;
						bDirty = TRUE;
					}
				}
				itModify = tStat.st_mtime;
			}
			if(TRUE == bDirty)
			{
				// Read the conf file.
				if (0 != Conf_Get_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNCD_SLIMITRATE, szSLimitRate, sizeof(szSLimitRate)))  strcpy(szSLimitRate, SZV_RSYNCD_SLIMITRATE_DEF);
				if (0 != Conf_Get_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNCD_RLIMITRATE, szRLimitRate, sizeof(szRLimitRate)))  strcpy(szRLimitRate, SZV_RSYNCD_RLIMITRATE_DEF);

				// Add inotify watch to this conf file.
				if (-1 == (idNotify = inotify_add_watch(fdiNotify, RSYNC_SCHEDULE_CONF, mfMask)))
				{
					iRet = errno;
					rsyserr(FERROR, iRet, "Can't add an inotify watch of \"%s\" error! (%d)\n", RSYNC_SCHEDULE_CONF, iRet);
					goto exit_child;
				}
				bDirty = FALSE;
				if((0 != memcmp(szSLimitRate, szSLimitOld, sizeof(szSLimitRate))) || (0 != memcmp(szRLimitRate, szRLimitOld, sizeof(szRLimitRate))))
				{
					kill(idPPid, SIGRATE);
					memcpy(szSLimitOld, szSLimitRate, sizeof(szSLimitRate));
					memcpy(szRLimitOld, szRLimitRate, sizeof(szRLimitRate));
				}
			}
		}
	exit_child:
		if(fdiNotify != -1)
		{
			if(idNotify != -1)
				inotify_rm_watch(fdiNotify, idNotify);
			close(fdiNotify);
		}
		exit(iRet);
	}
	return pid;
}
#endif /* QNAP_CLIENT_LIMITRATE_BY_THREAD */
#endif /* QNAPNAS && SUPPORT_LIMITRATE */

/**
 * Start a client for either type of remote connection.  Work out
 * whether the arguments request a remote shell or rsyncd connection,
 * and call the appropriate connection function, then run_client.
 *
 * Calls either start_socket_client (for sockets) or do_cmd and
 * client_run (for ssh).
 **/
static int start_client(int argc, char *argv[])
{
	char *p, *shell_machine = NULL, *shell_user = NULL;
	char **remote_argv;
	int remote_argc;
	int f_in, f_out;
	int ret;
	pid_t pid;
	// jeff modify 2011.8.26, for limit band width.
	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
	extern char *pszSchedule;	// current schedule name
	#ifdef QNAP_CLIENT_LIMITRATE_BY_THREAD
	pthread_t thLimitRateID=-1;
	//jeff modify 2012.06.27, if execute rysnc alone but not by QNAP job, we can't set limit rate.(let power users use --bwlimit in command line)
	if(g_bQnapBwlimit)
	{
		// Launch an additional thread to handle the band width related variables to the share memory.
		if (0 != pthread_create(&thLimitRateID, NULL, Update_LimitRate_Client_Thread, pszSchedule))
		{
			rsyserr(FERROR, errno, "Can't create a thread to moniter limit rate!");
		}
	}
	#else
	if(g_bQnapBwlimit)
	{
		Signal_Handler_SigRate(SIGRATE);//bug 107860 update "bwlimit" variable immediately
		if(-1 == Fork_Monitor_LimitRate(pszSchedule))
			rsyserr(FERROR, errno, "Can't fork a process to moniter limit rate!");
	}
	#endif /* QNAP_CLIENT_LIMITRATE_BY_THREAD */
	#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	
	/* Don't clobber argv[] so that ps(1) can still show the right
	 * command line. */
	if ((ret = copy_argv(argv)) != 0)
	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
		goto exit;
	#else
		return ret;
	#endif /* QNAPNAS && SUPPORT_LIMITRATE */

	if (!read_batch) { /* for read_batch, NO source is specified */
		char *path = check_for_hostspec(argv[0], &shell_machine, &rsync_port);
		if (path) { /* source is remote */
			char *dummy_host;
			int dummy_port = 0;
			*argv = path;
			remote_argv = argv;
			remote_argc = argc;
			argv += argc - 1;
			if (argc == 1 || **argv == ':')
				argc = 0; /* no dest arg */
			else if (check_for_hostspec(*argv, &dummy_host, &dummy_port)) {
				rprintf(FERROR,
					"The source and destination cannot both be remote.\n");
				if ((ret = copy_argv(argv)) != 0)
				#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
				{
					ret = RERR_SYNTAX;
					goto exit_clean;
				}
				#else
					exit_cleanup(RERR_SYNTAX);
				#endif /* QNAPNAS && SUPPORT_LIMITRATE */
			} else {
				remote_argc--; /* don't count dest */
				argc = 1;
			}
			if (filesfrom_host && *filesfrom_host
			    && strcmp(filesfrom_host, shell_machine) != 0) {
				rprintf(FERROR,
					"--files-from hostname is not the same as the transfer hostname\n");
				#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
					ret = RERR_SYNTAX;
					goto exit_clean;
				#else
					exit_cleanup(RERR_SYNTAX);
				#endif /* QNAPNAS && SUPPORT_LIMITRATE */
			}
			am_sender = 0;
			if (rsync_port)
				daemon_over_rsh = shell_cmd ? 1 : -1;
		} else { /* source is local, check dest arg */
			am_sender = 1;

			if (argc > 1) {
				p = argv[--argc];
				remote_argv = argv + argc;
			} else {
				static char *dotarg[1] = { "." };
				p = dotarg[0];
				remote_argv = dotarg;
			}
			remote_argc = 1;

			path = check_for_hostspec(p, &shell_machine, &rsync_port);
			if (path && filesfrom_host && *filesfrom_host
			    && strcmp(filesfrom_host, shell_machine) != 0) {
				rprintf(FERROR,
					"--files-from hostname is not the same as the transfer hostname\n");
				#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
					ret = RERR_SYNTAX;
					goto exit_clean;
				#else
					exit_cleanup(RERR_SYNTAX);
				#endif /* QNAPNAS && SUPPORT_LIMITRATE */
			}
			if (!path) { /* no hostspec found, so src & dest are local */
				local_server = 1;
				if (filesfrom_host) {
					rprintf(FERROR,
						"--files-from cannot be remote when the transfer is local\n");
					#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
						ret = RERR_SYNTAX;
						goto exit_clean;
					#else
						exit_cleanup(RERR_SYNTAX);
					#endif /* QNAPNAS && SUPPORT_LIMITRATE */
				}
				shell_machine = NULL;
			} else { /* hostspec was found, so dest is remote */
				argv[argc] = path;
				if (rsync_port)
					daemon_over_rsh = shell_cmd ? 1 : -1;
			}
		}
	} else {  /* read_batch */
		local_server = 1;
		if (check_for_hostspec(argv[argc-1], &shell_machine, &rsync_port)) {
			rprintf(FERROR, "remote destination is not allowed with --read-batch\n");
			#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
				ret = RERR_SYNTAX;
				goto exit_clean;
			#else
				exit_cleanup(RERR_SYNTAX);
			#endif /* QNAPNAS && SUPPORT_LIMITRATE */
		}
		remote_argv = argv += argc - 1;
		remote_argc = argc = 1;
	}

	if (am_sender) {
		char *dummy_host;
		int dummy_port = rsync_port;
		int i;
		/* For local source, extra source args must not have hostspec. */
		for (i = 1; i < argc; i++) {
			if (check_for_hostspec(argv[i], &dummy_host, &dummy_port)) {
				rprintf(FERROR, "Unexpected remote arg: %s\n", argv[i]);
				#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
					ret = RERR_SYNTAX;
					goto exit_clean;
				#else
					exit_cleanup(RERR_SYNTAX);
				#endif /* QNAPNAS && SUPPORT_LIMITRATE */
			}
		}
	} else {
		char *dummy_host;
		int dummy_port = rsync_port;
		int i;
		/* For remote source, any extra source args must have either
		 * the same hostname or an empty hostname. */
		for (i = 1; i < remote_argc; i++) {
			char *arg = check_for_hostspec(remote_argv[i], &dummy_host, &dummy_port);
			if (!arg) {
				rprintf(FERROR, "Unexpected local arg: %s\n", remote_argv[i]);
				rprintf(FERROR, "If arg is a remote file/dir, prefix it with a colon (:).\n");
				exit_cleanup(RERR_SYNTAX);
			}
			if (*dummy_host && strcmp(dummy_host, shell_machine) != 0) {
				rprintf(FERROR, "All source args must come from the same machine.\n");
				exit_cleanup(RERR_SYNTAX);
			}
			if (rsync_port != dummy_port) {
				if (!rsync_port || !dummy_port)
					rprintf(FERROR, "All source args must use the same hostspec format.\n");
				else
					rprintf(FERROR, "All source args must use the same port number.\n");
				exit_cleanup(RERR_SYNTAX);
			}
			remote_argv[i] = arg;
		}
	}

	if (daemon_over_rsh < 0)
		return start_socket_client(shell_machine, remote_argc, remote_argv, argc, argv);

	if (password_file && !daemon_over_rsh) {
		rprintf(FERROR, "The --password-file option may only be "
				"used when accessing an rsync daemon.\n");
		#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
			ret = RERR_SYNTAX;
			goto exit_clean;
		#else
			exit_cleanup(RERR_SYNTAX);
		#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	}

	if (connect_timeout) {
		rprintf(FERROR, "The --contimeout option may only be "
				"used when connecting to an rsync daemon.\n");
		#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
			ret = RERR_SYNTAX;
			goto exit_clean;
		#else
			exit_cleanup(RERR_SYNTAX);
		#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	}

	if (shell_machine) {
		p = strrchr(shell_machine,'@');
		if (p) {
			*p = 0;
			shell_user = shell_machine;
			shell_machine = p+1;
		}
	}

	if (verbose > 3) {
		rprintf(FINFO,"cmd=%s machine=%s user=%s path=%s\n",
			NS(shell_cmd), NS(shell_machine), NS(shell_user),
			remote_argv ? NS(remote_argv[0]) : "");
	}

	pid = do_cmd(shell_cmd, shell_machine, shell_user, remote_argv, remote_argc,
		     &f_in, &f_out);

	/* if we're running an rsync server on the remote host over a
	 * remote shell command, we need to do the RSYNCD protocol first */
	if (daemon_over_rsh) {
		int tmpret;
		tmpret = start_inband_exchange(f_in, f_out, shell_user, remote_argc, remote_argv);
		if (tmpret < 0)
		#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
		{
			ret = tmpret;
			goto exit;
		}
		#else
			return tmpret;
		#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	}

	ret = client_run(f_in, f_out, pid, argc, argv);

	fflush(stdout);
	fflush(stderr);

exit_clean:
	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
	// terminate thread of limit rate
	g_bRate = TRUE;
	#ifdef QNAP_CLIENT_LIMITRATE_BY_THREAD
	if(-1 != (int)thLimitRateID)
		pthread_join(thLimitRateID, NULL);
	#endif /* QNAP_CLIENT_LIMITRATE_BY_THREAD */
	#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	exit_cleanup(ret);
exit:
	#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
	// terminate thread of limit rate
	g_bRate = TRUE;
	#ifdef QNAP_CLIENT_LIMITRATE_BY_THREAD
	if(-1 != (int)thLimitRateID)
		pthread_join(thLimitRateID, NULL);
	#endif /* QNAP_CLIENT_LIMITRATE_BY_THREAD */
	#endif /* QNAPNAS && SUPPORT_LIMITRATE */
	return ret;
}


static RETSIGTYPE sigusr1_handler(UNUSED(int val))
{
	exit_cleanup(RERR_SIGNAL1);
}

static RETSIGTYPE sigusr2_handler(UNUSED(int val))
{
	if (!am_server)
		output_summary();
	close_all();
	if (got_xfer_error)
		_exit(RERR_PARTIAL);
	_exit(0);
}

RETSIGTYPE remember_children(UNUSED(int val))
{
#ifdef WNOHANG
	int cnt, status;
	pid_t pid;
	/* An empty waitpid() loop was put here by Tridge and we could never
	 * get him to explain why he put it in, so rather than taking it
	 * out we're instead saving the child exit statuses for later use.
	 * The waitpid() loop presumably eliminates all possibility of leaving
	 * zombie children, maybe that's why he did it. */
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		/* save the child's exit status */
#ifdef QTS_SNAPSHOT
		for (cnt = 0; cnt < pid_stat_table_size; cnt++) {
			if (pid_stat_table[cnt].pid == 0) {
				pid_stat_table[cnt].pid = pid;
				pid_stat_table[cnt].status = status;
				break;
			}
		}
		if (cnt == pid_stat_table_size) { /* table is too small */
			int tmp_pid_stat_table_size = pid_stat_table_size + MAXCHILDPROCS;
			struct pid_status *tmp_table = NULL;
			tmp_table = malloc(sizeof(struct pid_status)*tmp_pid_stat_table_size);
			if (!tmp_table)
				break;
			memset(tmp_table, 0, sizeof(struct pid_status)*tmp_pid_stat_table_size);
			for (cnt = 0; cnt < pid_stat_table_size; cnt++) {
				tmp_table[cnt].pid = pid_stat_table[cnt].pid;
				tmp_table[cnt].status = pid_stat_table[cnt].status;
			}
			tmp_table[cnt].pid = pid;
			tmp_table[cnt].status = status;
			free(pid_stat_table);
			pid_stat_table = tmp_table;
                        pid_stat_table_size = tmp_pid_stat_table_size;
		}
#else              
		for (cnt = 0; cnt < MAXCHILDPROCS; cnt++) {
			if (pid_stat_table[cnt].pid == 0) {
				pid_stat_table[cnt].pid = pid;
				pid_stat_table[cnt].status = status;
				break;
			}
		}
#endif
	}
#endif
#ifndef HAVE_SIGACTION
	signal(SIGCHLD, remember_children);
#endif
}


/**
 * This routine catches signals and tries to send them to gdb.
 *
 * Because it's called from inside a signal handler it ought not to
 * use too many library routines.
 *
 * @todo Perhaps use "screen -X" instead/as well, to help people
 * debugging without easy access to X.  Perhaps use an environment
 * variable, or just call a script?
 *
 * @todo The /proc/ magic probably only works on Linux (and
 * Solaris?)  Can we be more portable?
 **/
#ifdef MAINTAINER_MODE
const char *get_panic_action(void)
{
	const char *cmd_fmt = getenv("RSYNC_PANIC_ACTION");

	if (cmd_fmt)
		return cmd_fmt;
	else
		return "xterm -display :0 -T Panic -n Panic "
			"-e gdb /proc/%d/exe %d";
}


/**
 * Handle a fatal signal by launching a debugger, controlled by $RSYNC_PANIC_ACTION.
 *
 * This signal handler is only installed if we were configured with
 * --enable-maintainer-mode.  Perhaps it should always be on and we
 * should just look at the environment variable, but I'm a bit leery
 * of a signal sending us into a busy loop.
 **/
static RETSIGTYPE rsync_panic_handler(UNUSED(int whatsig))
{
	char cmd_buf[300];
	int ret;

	snprintf(cmd_buf, sizeof cmd_buf, get_panic_action(),
		 getpid(), getpid());

	/* Unless we failed to execute gdb, we allow the process to
	 * continue.  I'm not sure if that's right. */
	ret = system(cmd_buf);
	if (ret)
		_exit(ret);
}
#endif


int main(int argc,char *argv[])
{
	int ret;
	int orig_argc = argc;
	char **orig_argv = argv;

#ifdef	QNAPNAS
	extern char *pszSchedule;	///< current schedule name
	extern char *pszProgressConf;
#endif	//QNAPNAS

#ifdef HAVE_SIGACTION
# ifdef HAVE_SIGPROCMASK
	sigset_t sigmask;

	sigemptyset(&sigmask);
# endif
	sigact.sa_flags = SA_NOCLDSTOP;
#endif
	SIGACTMASK(SIGUSR1, sigusr1_handler);
	SIGACTMASK(SIGUSR2, sigusr2_handler);
	SIGACTMASK(SIGCHLD, remember_children);
#ifdef MAINTAINER_MODE
	SIGACTMASK(SIGSEGV, rsync_panic_handler);
	SIGACTMASK(SIGFPE, rsync_panic_handler);
	SIGACTMASK(SIGABRT, rsync_panic_handler);
	SIGACTMASK(SIGBUS, rsync_panic_handler);
#endif

	starttime = time(NULL);
	our_uid = MY_UID();
	am_root = our_uid == 0;

	memset(&stats, 0, sizeof(stats));

	if (argc < 2) {
		usage(FERROR);
		exit_cleanup(RERR_SYNTAX);
	}

	/* we set a 0 umask so that correct file permissions can be
	 * carried across */
	orig_umask = umask(0);

#if defined CONFIG_LOCALE && defined HAVE_SETLOCALE
	setlocale(LC_CTYPE, "");
#endif

	if (!parse_arguments(&argc, (const char ***) &argv)) {
		/* FIXME: We ought to call the same error-handling
		 * code here, rather than relying on getopt. */
		option_error();
		exit_cleanup(RERR_SYNTAX);
	}

#ifdef	QNAPNAS
	if(pszProgressConf)
	    rsync_conf = pszProgressConf;
	else
	    rsync_conf = RSYNC_DEFAULT_SCHEDULE_CONF;
	// Lower the rsync priority.
	 nice(5);

	// reset the progress to the rsync schedule conf file
	if (pszSchedule)
	{
		char  szBuf[64];

		// Check if there is already a process executing for this job.
		if (0 == Conf_Get_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_PID, szBuf, sizeof(szBuf)))
		{
			long  iPid = atol(szBuf);
			if ((0 < iPid) && (0 == kill(iPid, 0)))
			{
				exit_cleanup(RERR_JOBRUNNING);
			}
		}

		sprintf(szBuf, "%d", getpid());		
		Conf_Set_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_PID, szBuf);
		Conf_Set_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_PROGRESS, "0");
		Conf_Set_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_REMAIN_TIME, "0");
		Conf_Remove_Field(RSYNC_SCHEDULE_CONF, pszSchedule, SZK_RSYNC_PAUSED);

#ifdef	RSYNC_PROGRESS
		// Initial scan tree status.
		{
			extern int  g_bAbort;
			memset(&g_ttParam, 0, sizeof(g_ttParam));
			g_ttParam.pttStat = &stats.ttStat;
			g_ttParam.pbAbort = &g_bAbort;
		}
#endif	//RSYNC_PROGRESS

	}
#endif	//QNAPNAS

	SIGACTMASK(SIGINT, sig_int);
	SIGACTMASK(SIGHUP, sig_int);
	SIGACTMASK(SIGTERM, sig_int);
#if defined HAVE_SIGACTION && HAVE_SIGPROCMASK
	sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
#endif

	/* Ignore SIGPIPE; we consistently check error codes and will
	 * see the EPIPE. */
	SIGACTION(SIGPIPE, SIG_IGN);
#ifdef SIGXFSZ
	SIGACTION(SIGXFSZ, SIG_IGN);
#endif

#ifdef	QNAPNAS
	SIGACTION(SIGHUP, Signal_Handler_SigHup);
#ifdef	SUPPORT_LIMITRATE
#ifndef QNAP_CLIENT_LIMITRATE_BY_THREAD
	SIGACTION(SIGRATE, Signal_Handler_SigRate);
#endif  //QNAP_CLIENT_LIMITRATE_BY_THREAD
#endif	//SUPPORT_LIMITRATE
#endif	//QNAPNAS

	/* Initialize change_dir() here because on some old systems getcwd
	 * (implemented by forking "pwd" and reading its output) doesn't
	 * work when there are other child processes.  Also, on all systems
	 * that implement getcwd that way "pwd" can't be found after chroot. */
	change_dir(NULL, CD_NORMAL);

	init_flist();

	if ((write_batch || read_batch) && !am_server) {
		if (write_batch)
			write_batch_shell_file(orig_argc, orig_argv, argc);

		if (read_batch && strcmp(batch_name, "-") == 0)
			batch_fd = STDIN_FILENO;
		else {
			batch_fd = do_open(batch_name,
				   write_batch ? O_WRONLY | O_CREAT | O_TRUNC
				   : O_RDONLY, S_IRUSR | S_IWUSR);
		}
		if (batch_fd < 0) {
			rsyserr(FERROR, errno, "Batch file %s open error",
				full_fname(batch_name));
			exit_cleanup(RERR_FILEIO);
		}
		if (read_batch)
			read_stream_flags(batch_fd);
		else
			write_stream_flags(batch_fd);
	}
	if (write_batch < 0)
		dry_run = 1;

	if (am_server) {
#ifdef ICONV_CONST
		setup_iconv();
#endif
	} else if (am_daemon)
		return daemon_main();

	if (am_server && protect_args) {
		char buf[MAXPATHLEN];
		protect_args = 2;
		read_args(STDIN_FILENO, NULL, buf, sizeof buf, 1, &argv, &argc, NULL);
		if (!parse_arguments(&argc, (const char ***) &argv)) {
			option_error();
			exit_cleanup(RERR_SYNTAX);
		}
	}

	if (argc < 1) {
		usage(FERROR);
		exit_cleanup(RERR_SYNTAX);
	}

	if (am_server) {
		set_nonblocking(STDIN_FILENO);
		set_nonblocking(STDOUT_FILENO);
		if (am_daemon)
			return start_daemon(STDIN_FILENO, STDOUT_FILENO);
		start_server(STDIN_FILENO, STDOUT_FILENO, argc, argv);
	}

#ifdef QNAPNAS
	{
		char* pid_fileP = NULL;
		switch(qnap_mode) {
			case NORMAL:	pid_fileP = NORMAL_LOCK_FILE;	break;
			case QRAID1:	pid_fileP = QRAID1_LOCK_FILE;	break;
			case USB_COPY:	pid_fileP = USBCOPY_LOCK_FILE;	break;
			case HD_COPY_USB:	pid_fileP = HDCOPYUSB_LOCK_FILE;	break;
		}
		if(pid_fileP)
		{
			char cmd[64];
			sprintf(cmd, "echo %d > %s", getpid(), pid_fileP);
			system(cmd);
		}
	}
#endif

	ret = start_client(argc, argv);
	if (ret == -1)
		exit_cleanup(RERR_STARTCLIENT);
	else
#ifdef QNAPNAS
		if(do_xfers)
#endif
		exit_cleanup(ret);

	return ret;
}

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * printjob -- print jobs in the queue.
 *
 *	NOTE: the lock file is used to pass information to lpq and lprm.
 *	it does not need to be removed because file locks are dynamic.
 */

/*
 *
 * DAI - Cisco. 26 Oct 1996
 * Added an extra 'E' flag into the control file. This will allow the
 * passing of extra enviroment variables into the filter program.
 * the lines are of the form 'Evar=value'
 */


#include "lp.h"
#include "pathnames.h"

#include <time.h>
#include <string.h>

// Modified by DavidYang 2001/9/25
#include <sys/mman.h>
//#include <sys/time.h>
// driver related include file
//#include <sysmgrd.h>
//#include <msgeng.h>

// library related include file
//#include <msg.h>
// End of modification

#define DORETURN	0	/* absorb fork error */
#define DOABORT		1	/* abort if dofork fails */

/*
 * Error tokens
 */
#define REPRINT		-2
#define ERROR		-1
#define	OK		0
#define	FATALERR	1
#define	NOACCT		2
#define	FILTERERR	3
#define	ACCESS		4
#define UNLINK		5
#define	BADLINK		6
#define NOSTAT		7

#define LPOPT_EVAR     "LPOPTS"

static dev_t	 fdev;		/* device of file pointed to by symlink */
static ino_t	 fino;		/* inode of file pointed to by symlink */
static FILE	*cfp;		/* control file */
static int	 child;		/* id of any filters */
static int	 lfd;		/* lock file descriptor */
static int	 ofd;		/* output filter file descriptor */
static int	 ofilter;	/* id of output filter, if any */

static int	 pfd;		/* printer file descriptor */
static int	 pid;		/* pid of lpd process */
static int	 prchild;	/* id of pr process */
static char	 title[80];	/* ``pr'' title */
static int	 tof;		/* true if at top of form */

static char	class[32];		/* classification field */
static char	fromhost[MAXHOSTNAMELEN];		/* user's host machine */
				/* indentation size in static characters */
static char	indent[10] = "-i0";
static char	jobname[100];		/* job or file name */
static char	extparms[256];		/* extended parms */
static char	length[10] = "-l";	/* page length in lines */
static char	logname[32];		/* user's login name */
static char	euser[32];		/* effective user */
static char	pxlength[10] = "-y";	/* page length in pixels */
static char	pxwidth[10] = "-x";	/* page width in pixels */
static char	tempfile[] = "errsXXXXXX"; /* file name for filter errors */
static char	width[10] = "-w";	/* page width in static characters */

	int	remote;		/* true if sending files to remote */
static	short SF;		/* suppress FF on each print job */

static void	abortpr	(int signo);
static void	banner	(char *name1, char *name2);
static int	dofork	(int action);
static int	doforkuser(int action, char *user);
static int	dropit	(int c);
static void	init	(void);
static void	openpr	(void);
static int	print	(int format, char *file);
static int	printit	(char *file);
static void	pstatus	(const char *msg, ...);
static char	response(void);
static void	scan_out(int scfd, char *scsp, int dlm);
static char *	scnline	(int key, char *p, int c);
static int	sendit	(char *file);
static int	sendfile(int type, char *file);
static void	sendmail(char *user, int bombed);
static void	setty	(void);
static void	unlinkfile(char *file, char *host, char *user);

// Modified by DavidYang 2001/9/25
int 	dydebug = 1;
int	print_trace_intval = 20; // in sec
char	msg_print[] = "/sbin/msg_print";
  
extern void mydebug(char *thestr);
/*
static void mydebug(char *thestr)
{
	struct timeval tval;
	FILE *fp = NULL;

	gettimeofday(&tval, NULL);

	fp = fopen("/lpd_dblog", "a+");
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		fprintf(fp, "(PID:%d)[%02d:%02d:%02d:%04d]%s\n", getpid(),
				(int)((tval.tv_sec/3600)%24)+8,
				(int)((tval.tv_sec%3600)/60),
				(int)(tval.tv_sec%60), 
				(int)(tval.tv_usec/1000),
				thestr);
		fclose(fp);
	}
}
*/

static void SendMsg(int cmd)
{
	//int msg = cmd + SYSMGR_PRINT_STARTED - 1;
	char syscmd[128]="";
	sprintf(syscmd,"%s %d", msg_print, cmd);
	if ( dydebug )
		mydebug(syscmd);
	system(syscmd);
	//SendMessage("sysmond", MSG_SYSMGR, msg, 0);
}

static void on_print_start()
{
	SendMsg(1);
}

static void on_print_finish()
{
	// because of the limitation od display panel, finish message 
	// can't be sent after error. So we send start first...
	SendMsg(1);
	SendMsg(2);
}

static void on_print_error()
{
	SendMsg(3);
}
// End of modification
void
printjob()
{
	struct stat stb;
	register struct queue *q, **qp;
	struct queue **queue;
	register int i, nitems;
	off_t pidoff;
	int errcnt, count = 0;

	init();					/* set up capabilities */
	(void) write(1, "", 1);			/* ack that daemon is started */
	(void) close(2);			/* set up log file */
	if (open(LF, O_WRONLY|O_APPEND, 0664) < 0) {
		syslog(LOG_ERR, "%s: %m", LF);
		(void) open(_PATH_DEVNULL, O_WRONLY);
	}
	setgid(getegid());
	pid = getpid();				/* for use with lprm */
	setpgrp();
	signal(SIGHUP, abortpr);
	signal(SIGINT, abortpr);
	signal(SIGQUIT, abortpr);
	signal(SIGTERM, abortpr);

	(void) mktemp(tempfile);

	/*
	 * uses short form file names
	 */
	if (chdir(SD) < 0) {
		syslog(LOG_ERR, "%s: %m", SD);
		exit(1);
	}
	if (stat(LO, &stb) == 0 && (stb.st_mode & 0100))
		exit(0);		/* printing disabled */
	lfd = open(LO, O_WRONLY|O_CREAT, 0644);
	if (lfd < 0) {
		syslog(LOG_ERR, "%s: %s: %m", printer, LO);
		exit(1);
	}
	if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK)	/* active deamon present */
// Modified by DavidYang 2001/9/25
		{
			if (getq(&queue) == 0) {
				// Send finish message
				on_print_finish();
			}		
			exit(0);
		}
// End of modification
		syslog(LOG_ERR, "%s: %s: %m", printer, LO);
		exit(1);
	}
	ftruncate(lfd, 0);
	/*
	 * write process id for others to know
	 */
	snprintf(line, BUFSIZ, "%u\n", pid);
	pidoff = i = strlen(line);
	if (write(lfd, line, i) != i) {
		syslog(LOG_ERR, "%s: %s: %m", printer, LO);
		exit(1);
	}
	/*
	 * search the spool directory for work and sort by queue order.
	 */
	if ((nitems = getq(&queue)) < 0) {
		syslog(LOG_ERR, "%s: can't scan %s", printer, SD);
		exit(1);
	}
	if (nitems == 0)		/* no work to do */
		exit(0);
	if (stb.st_mode & 01) {		/* reset queue flag */
		if (fchmod(lfd, stb.st_mode & 0776) < 0)
			syslog(LOG_ERR, "%s: %s: %m", printer, LO);
	}
// Modified by DavidYang 2001/9/25
	// Send start printing message
	on_print_start();	
// end of modification
	openpr();			/* open printer or remote */
again:
	/*
	 * we found something to do now do it --
	 *    write the name of the current control file into the lock file
	 *    so the spool queue program can tell what we're working on
	 */
	for (qp = queue; nitems--; free((char *) q)) {
		q = *qp++;
		if (stat(q->q_name, &stb) < 0)
			continue;
		errcnt = 0;
	restart:
		(void) lseek(lfd, pidoff, 0);
		(void) snprintf(line, sizeof(line), "%s\n", q->q_name);
		i = strlen(line);
		if (write(lfd, line, i) != i)
			syslog(LOG_ERR, "%s: %s: %m", printer, LO);
		if (!remote)
			i = printit(q->q_name);
		else
			i = sendit(q->q_name);
		/*
		 * Check to see if we are supposed to stop printing or
		 * if we are to rebuild the queue.
		 */
		if (fstat(lfd, &stb) == 0) {
			/* stop printing before starting next job? */
			if (stb.st_mode & 0100)
				goto done;
			/* rebuild queue (after lpc topq) */
			if (stb.st_mode & 01) {
				for (free((char *) q); nitems--; free((char *) q))
					q = *qp++;
				if (fchmod(lfd, stb.st_mode & 0776) < 0)
					syslog(LOG_WARNING, "%s: %s: %m",
						printer, LO);
				break;
			}
		}
		if (i == OK)		/* file ok and printed */
			count++;
		else if (i == REPRINT && ++errcnt < 5) {
			/* try reprinting the job */
			syslog(LOG_INFO, "restarting %s", printer);
			if (ofilter > 0) {
				kill(ofilter, SIGCONT);	/* to be sure */
				(void) close(ofd);
				while ((i = wait(NULL)) > 0 && i != ofilter)
					;
				ofilter = 0;
			}
			(void) close(pfd);	/* close printer */
			if (ftruncate(lfd, pidoff) < 0)
				syslog(LOG_WARNING, "%s: %s: %m", printer, LO);
			openpr();		/* try to reopen printer */
			goto restart;
		} else {
			syslog(LOG_WARNING, "%s: job could not be %s (%s)", printer,
				remote ? "sent to remote host" : "printed", q->q_name);
			if (i == REPRINT) {
				/* ensure we don't attempt this job again */
				(void) unlink(q->q_name);
				q->q_name[0] = 'd';
				(void) unlink(q->q_name);
				if (logname[0])
					sendmail(logname, FATALERR);
			}
		}
	}
	free((char *) queue);
	/*
	 * search the spool directory for more work.
	 */
	nitems = getq(&queue);
	if (nitems == 0) {		/* no more work to do */
	done:
// Modified by DavidYang 2001/9/25
		// Send finish printing message
		on_print_finish();	
// End of modification
		flock(lfd,LOCK_UN);	/* Unlock the lock now, to avoid deadlocks */
		if (count > 0) {	/* Files actually printed */
			if (!SF && !tof)
				(void) write(ofd, FF, strlen(FF));
			if (TR != NULL)		/* output trailer */
				(void) write(ofd, TR, strlen(TR));
		}
		(void) close(ofd);
		(void) wait(NULL);
		(void) unlink(tempfile);
		exit(0);
	} else if (nitems < 0) {
		syslog(LOG_ERR, "%s: can't scan %s", printer, SD);
		exit(1);
	}
	goto again;
}

char	fonts[4][50];	/* fonts for troff */

char ifonts[4][40] = {
	_PATH_VFONTR,
	_PATH_VFONTI,
	_PATH_VFONTB,
	_PATH_VFONTS,
};

/*
 * The remaining part is the reading of the control file (cf)
 * and performing the various actions.
 */
static int
printit(file)
	char *file;
{
	register int i;
	char *cp;
	int bombed = OK;
	int fenvcnt=0;

        fenv[0]=NULL;

	/*
	 * open control file; ignore if no longer there.
	 */
	if ((cfp = fopen(file, "r")) == NULL) {
		syslog(LOG_INFO, "%s: %s: %m", printer, file);
		return(OK);
	}
	/*
	 * Reset troff fonts.
	 */
	for (i = 0; i < 4; i++)
		strcpy(fonts[i], ifonts[i]);
	sprintf(&width[2], "%d", PW);
	strcpy(indent+2, "0");

	/*
	 *      read the control file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      valid commands are:
	 *
	 *		S -- "stat info" for symbolic link protection
	 *		J -- "job name" on banner page
	 *		C -- "class name" on banner page
	 *           L -- "literal" user's name to print on banner
	 *		T -- "title" for pr
	 *		H -- "host name" of machine where lpr was done
	 *           P -- "person" user's login name
	 *           I -- "indent" amount to indent output
	 *		R -- laser dpi "resolution"
	 *           f -- "file name" name of text file to print
	 *		l -- "file name" text file with control chars
	 *		p -- "file name" text file to print with pr(1)
	 *		t -- "file name" troff(1) file to print
	 *		n -- "file name" ditroff(1) file to print
	 *		d -- "file name" dvi file to print
	 *		g -- "file name" plot(1G) file to print
	 *		v -- "file name" plain raster file to print
	 *		c -- "file name" cifplot file to print
	 *		1 -- "R font file" for troff
	 *		2 -- "I font file" for troff
	 *		3 -- "B font file" for troff
	 *		4 -- "S font file" for troff
	 *		N -- "name" of file (used by lpq)
	 *           U -- "unlink" name of file to remove
	 *                (after we print it. (Pass 2 only)).
	 *		M -- "mail" to user when done printing
	 *
	 *      getline reads a line and expands tabs to blanks
	 */

	/* pass 0 */
	/* Henri Gomez <gomez@slib.fr> 19991104 */
	/* Pre scan to get the Z flags (RFC Filter Extension) */
	while (getline(cfp))
		switch (line[0]) {
        case 'Z':
            if (line[1] != '\0') {
                strncpy(extparms, line+1, sizeof(extparms) - 1);
                extparms[sizeof(extparms) - 1] = '\0';
            } else
                strcpy(extparms, " ");
            continue;
	case 'E':
	  /* ignore too many options */
	  if(fenvcnt!=MAXFENV-1){
            fenv[fenvcnt++]=strdup(line+1);
            fenv[fenvcnt]=NULL;
	  }
	  continue;
	}

	rewind(cfp);

	/* pass 1 */

	while (getline(cfp))
		switch (line[0]) {
		case 'H':
			strncpy(fromhost, line+1, sizeof(fromhost) - 1);
			fromhost[sizeof(fromhost) - 1] = '\0';
			if (class[0] == '\0') {
				strncpy(class, line+1, sizeof(class) - 1);
				class[sizeof(class) - 1] = '\0';
			}
			continue;

		case 'P':
			{
			struct passwd *dude;

			strncpy(logname, line+1, sizeof(logname) - 1);
			logname[sizeof(logname) - 1] = '\0';
			dude = getpwnam(logname);

			if (RS && dude == NULL) {	/* no such user plus
							 * restricted */
					bombed = NOACCT;
					sendmail(line+1, bombed);
					goto pass2;
			}
			if(dude == NULL || dude->pw_uid == 0)
				   /* if user root, set euid to lp  */
				strncpy(euser, "lp", 3);
			else
				   /* since the user does exist run filter as user */
				strncpy(euser, logname, sizeof(euser));
			}
			continue;

		case 'S':
			cp = line+1;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fdev = i;
			cp++;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fino = i;
			continue;

		case 'Z':
			continue; /* Already handled */

		case 'J':
			if (line[1] != '\0') {
				strncpy(jobname, line+1, sizeof(jobname) - 1);
				jobname[sizeof(jobname) - 1] = '\0';
			} else
				strcpy(jobname, " ");
			continue;

		case 'C':
			if (line[1] != '\0')
				strncpy(class, line+1, sizeof(class) - 1);
			else if (class[0] == '\0')
				gethostname(class, sizeof(class));
			class[sizeof(class) - 1] = '\0';
			continue;

		case 'T':	/* header title for pr */
			strncpy(title, line+1, sizeof(title) - 1);
			title[sizeof(title) - 1] = '\0';
			continue;

		case 'L':	/* identification line */
			if (!SH && !HL)
				banner(line+1, jobname);
			continue;

		case '1':	/* troff fonts */
		case '2':
		case '3':
		case '4':
			if (line[1] != '\0') {
				strncpy(fonts[line[0]-'1'], line+1,
					sizeof(fonts[0])-1);
				fonts[line[0]-'1'][sizeof(fonts[0])-1] = '\0';
			}
			continue;

		case 'W':	/* page width */
			strncpy(width+2, line+1, sizeof(width) - 3);
			width[2+sizeof(width) - 3] = '\0';
			continue;

		case 'I':	/* indent amount */
			strncpy(indent+2, line+1, sizeof(indent) - 3);
			indent[2+sizeof(indent) - 3] = '\0';
			continue;

		case 'c':	/* some file to print */
		case 'f':	/* This adds 'proper' rfc 1179 command handling */
		case 'g':	/* But does not guarantee that all these formats */
		case 'l':	/* work */
		case 'n':
		case 'o':
		case 'p':
		case 'r':
		case 't':
		case 'v':
		case 'd':
			cp = NULL;
			switch (i = print(line[0], line+1)) {
			case ERROR:
				if (bombed == OK)
					bombed = FATALERR;
				break;
			case REPRINT:
			        zerofenv();
				(void) fclose(cfp);
				return(REPRINT);
			case FILTERERR:
				if (! cp) cp = "FILTERERR";
			case ACCESS:
				if (! cp) cp = "ACCESS";
			case NOSTAT:
				if (! cp) cp = "NOSTAT";
			case BADLINK:
				if (! cp) cp = "BADLINK";
				bombed = i;
				sendmail(logname, bombed);
                                syslog(LOG_ERR, "%s: %s: %s error on line \"%s\"",
                                       printer, file, cp, line);
			}
			title[0] = '\0';
			continue;

		case 'N':
		case 'U':
		case 'M':
		case 'R':
		case 'E':
		default:
			continue;
		}

	/* pass 2 */

pass2:
	fseek(cfp, 0L, 0);
	while (getline(cfp))
		switch (line[0]) {
		case 'L':	/* identification line */
			if (!SH && HL)
				banner(line+1, jobname);
			continue;

		case 'M':
			if (bombed < NOACCT)	/* already sent if >= NOACCT */
				sendmail(line+1, bombed);
			continue;

		case 'U':
			if (strchr(line+1, '/'))
				continue;
			(void) unlinkfile(line+1, fromhost, logname);
		}
	/*
	 * clean-up in case another control file exists
	 */
	zerofenv();
	(void) fclose(cfp);
	(void) unlink(file);
	return(bombed == OK ? OK : ERROR);
}

/*
 * Print a file.
 * Set up the chain [ PR [ | {IF, OF} ] ] or {IF, RF, TF, NF, DF, CF, VF}.
 * Return -1 if a non-recoverable error occured,
 * 2 if the filter detected some errors (but printed the job anyway),
 * 1 if we should try to reprint this job and
 * 0 if all is well.
 * Note: all filters take stdin as the file, stdout as the printer,
 * stderr as the log file, and must not ignore SIGINT.
 */
static int
print(format, file)
	int format;
	char *file;
{
	register int n;
	register char *prog;
	int fi, fo;
	FILE *fp;
	char *av[15], buf[BUFSIZ];
	int pid, p[2], stopped = 0;
	union wait status;
	struct stat stb;
        char *optspace;
        int optlen;
        char *sp1, *sp2;

	if (lstat(file, &stb) < 0)
		return(NOSTAT);

	if ((fi = open(file, O_RDONLY)) < 0) {
		return(ACCESS);
	}

	/*
	 * Check to see if data file is a symbolic link. If so, it should
	 * still point to the same file or someone is trying to print
	 * something he shouldn't.
	 */
	if ((stb.st_mode & S_IFMT) == S_IFLNK && fstat(fi, &stb) == 0 &&
	    (stb.st_dev != fdev || stb.st_ino != fino))
		return(BADLINK);
	if (!SF && !tof) {		/* start on a fresh page */
		(void) write(ofd, FF, strlen(FF));
		tof = 1;
	}

	if (IF == NULL && (format == 'f' || format == 'l')) {
// Modified by DavidYang 2001/9/25
		pid_t dypid;
		int usemap=0;
		caddr_t maparea;
		int *pcount = NULL;
		int lcount = 0;
		int toler = 0;
		int prnstate = 0;
		int warned = 0;
		 	
		tof = 0;
		// create memory map as shared memory
		if ( (maparea = mmap(0, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0)) != 
			(caddr_t)-1 ) {
			usemap = 1;
			pcount = (int *)maparea;
			*pcount = 0;
			
			// create monitor process
			if ( (dypid = fork()) < 0 ) {
				// fail... forget it
				usemap = 0;
			}
			if ( dypid == 0 )	{
				// shutdown signal handler
				signal(SIGHUP, SIG_DFL);
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				signal(SIGTERM, SIG_DFL);
				while(*pcount != -1) {
					if ( *pcount == lcount ) { // check test count and current
						toler++;
					}
					else {
						lcount = *pcount;
						if (prnstate == 1) {
							// Send start printing message
							on_print_start();
							warned = 0;
						}
						prnstate = 0;
						toler = 0;
						// since there is some progressing, sleep longer
						sleep(print_trace_intval);
						continue;
					}
					if (toler >= print_trace_intval) { // execeed tolerent time...
						// Send error printing message
						if ( warned == 0 )
						{
							on_print_error();
							warned = 1;
						}
						prnstate = 1;
						toler = 0;
					}
					sleep(1);
				}
				exit(0);
			}
		}
		while ((n = read(fi, buf, BUFSIZ)) > 0)	{
			if (write(ofd, buf, n) != n) {
				(void) close(fi);
				if ( usemap != 0 ) { // wait for monitor process or kill
					*pcount = -1;
					//waitpid(dypid, NULL, 0);
					kill(dypid, SIGTERM);
				}
				return(REPRINT);
			}
			if ( usemap != 0 ) {
				*pcount+=n;
			}
		}
		(void) close(fi);
		if ( usemap != 0 ) {
			*pcount = -1;
			//waitpid(dypid, NULL, 0);
			kill(dypid, SIGTERM);
		}
		return(OK);
// End of modification
		/*
		tof = 0;
		while ((n = read(fi, buf, BUFSIZ)) > 0)
			if (write(ofd, buf, n) != n) {
				(void) close(fi);
				return(REPRINT);
			}
		(void) close(fi);
		return(OK);
		*/
	}
	switch (format) {
	case 'p':	/* print file using 'pr' */
		if (IF == NULL) {	/* use output filter */
			prog = _PATH_PR;
			av[0] = "pr";
			av[1] = width;
			av[2] = length;
			av[3] = "-h";
			av[4] = *title ? title : " ";
			av[5] = 0;
			fo = ofd;
			goto start;
		}
		pipe(p);
		if ((prchild=doforkuser(DORETURN,euser)) == 0) { /* child */
			dup2(fi, 0);		/* file is stdin */
			dup2(p[1], 1);		/* pipe is stdout */
			closelog();
			for (n = 3; n < NOFILE; n++)
				(void) close(n);
			execl(_PATH_PR, "pr", width, length,
			    "-h", *title ? title : " ", 0);
			syslog(LOG_ERR, "cannot execl %s", _PATH_PR);
			exit(2);
		}
		(void) close(p[1]);		/* close output side */
		(void) close(fi);
		if (prchild < 0) {
			prchild = 0;
			(void) close(p[0]);
			return(ERROR);
		}
		fi = p[0];			/* use pipe for input */
	case 'f':	/* print plain text file */
		prog = IF;
		av[1] = width;
		av[2] = length;
		av[3] = indent;
		n = 4;
		break;
	case 'l':	/* like 'f' but pass control characters */
		prog = IF;
		av[1] = "-c";
		av[2] = width;
		av[3] = length;
		av[4] = indent;
		n = 5;
		break;
	case 'r':	/* print a fortran text file */
		prog = RF;
		av[1] = width;
		av[2] = length;
		n = 3;
		break;
	case 't':	/* print troff output */
	case 'n':	/* print ditroff output */
	case 'd':	/* print tex output */
		(void) unlink(".railmag");
		if ((fo = creat(".railmag", FILMOD)) < 0) {
			syslog(LOG_ERR, "%s: cannot create .railmag", printer);
			(void) unlink(".railmag");
		} else {
			for (n = 0; n < 4; n++) {
				if (fonts[n][0] != '/')
					(void) write(fo, _PATH_VFONT,
					    sizeof(_PATH_VFONT) - 1);
				(void) write(fo, fonts[n], strlen(fonts[n]));
				(void) write(fo, "\n", 1);
			}
			(void) close(fo);
		}
		prog = (format == 't') ? TF : (format == 'n') ? NF : DF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'c':	/* print cifplot output */
		prog = CF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'g':	/* print plot(1G) output */
		prog = GF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'v':	/* print raster output */
		prog = VF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	default:
		(void) close(fi);
		syslog(LOG_ERR, "%s: illegal format character '%c'",
			printer, format);
		return(ERROR);
	}
	if (prog == NULL) {
		(void) close(fi);
		syslog(LOG_ERR,
		   "%s: no filter found in printcap for format character '%c'",
		   printer, format);
		return(ERROR);
	}
	if ((av[0] = strrchr(prog, '/')) != NULL)
		av[0]++;
	else
		av[0] = prog;
	av[n++] = "-n";
	av[n++] = logname;
	av[n++] = "-h";
	av[n++] = fromhost;

	/* Add Jobname */
	if (jobname[0] && (jobname[0] != ' ')) {
		av[n++] = "-j";
		av[n++] = jobname;
	}

	/* Add Extended Parms */
	if (extparms[0] && (extparms[0] != ' ')) {
		av[n++] = "-z";
        av[n++] = extparms;
	}

	av[n++] = AF;
	av[n] = 0;
	fo = pfd;
	if (ofilter > 0) {		/* stop output filter */
		write(ofd, "\031\1", 2);
		while ((pid=wait3((int *)&status, WUNTRACED, 0)) > 0 && pid != ofilter);
		if (!WIFSTOPPED(status)) {
			(void) close(fi);
			syslog(LOG_WARNING,
				"%s: output filter died (retcode=%d termsig=%d)",
				printer, status.w_retcode, status.w_termsig);
			return(REPRINT);
		}
		stopped++;
	}
start:
	if ((child = doforkuser(DORETURN,euser)) == 0) {	/* child */
		dup2(fi, 0);
		dup2(fo, 1);
		n = open(tempfile, O_WRONLY|O_CREAT|O_TRUNC, 0664);
		if (n >= 0)
			dup2(n, 2);
		closelog();
		for (n = 3; n < NOFILE; n++)
			(void) close(n);
               /* Work out the maximum space we will need (plus a bit!) */
               optlen=strlen(LPOPT_EVAR)+10;
               for (n=0; fenv[n]; n++)
                       optlen += strlen(fenv[n])+1;
               optspace=malloc(optlen);

               sprintf(optspace, "%s=", LPOPT_EVAR);
               sp1=optspace + strlen(optspace);
               for (n=0; fenv[n]; n++)
               {
                       /* Skip over initial w/s */
                       for (sp2=fenv[n]; isspace(*sp2); sp2++);

                       if (*sp2)
                       {
                               /* copy across, converting to lower case */
                               for(; *sp2; sp2++, sp1++)
                                       *sp1=tolower(*sp2);

                               /* Remove terminating w/s */
                               for (; isspace(*(sp1-1)); sp1--);
                               *sp1++ = ',';
                               *sp1 = '\0';
                       }
               }

               /* Remove possible terminating comma */
               if (*(sp1-1) == ',')
                       *--sp1='\0';
	       
               putenv(optspace);

		execv(prog, av);
		syslog(LOG_ERR, "cannot execv %s", prog);
		exit(2);
	}
	(void) close(fi);
	if (child < 0)
		status.w_retcode = 100;
	else
		while ((pid = wait((int *)&status)) > 0 && pid != child)
			;
	child = 0;
	prchild = 0;
	if (stopped) {		/* restart output filter */
		if (kill(ofilter, SIGCONT) < 0) {
			syslog(LOG_ERR, "cannot restart output filter");
			exit(1);
		}
	}
	tof = 0;

	/* Copy filter output to "lf" logfile */
	if ((fp = fopen(tempfile, "r"))) {
		while (fgets(buf, sizeof(buf), fp))
			fputs(buf, stderr);
		fclose(fp);
	}

	if (!WIFEXITED(status)) {
		syslog(LOG_WARNING, "%s: filter '%c' terminated (termsig=%d)",
			printer, format, status.w_termsig);
		return(ERROR);
	}
	switch (status.w_retcode) {
	case 0:
		tof = 1;
		return(OK);
	case 1:
		return(REPRINT);
	case 2:
		return(ERROR);
	default:
		syslog(LOG_WARNING, "%s: filter '%c' exited (retcode=%d)",
			printer, format, status.w_retcode);
		return(FILTERERR);
	}
}

/*
 * Send the daemon control file (cf) and any data files.
 * Return -1 if a non-recoverable error occured, 1 if a recoverable error and
 * 0 if all is well.
 */
static int
sendit(file)
	char *file;
{
	register int i, err = OK;
	char *cp, last[BUFSIZ];
	char *dup_cfpname = NULL;
	char *final_cfp;
	FILE *dup_cfp = NULL;
	struct stat stb;

	int  ifi, fi;
	int  ifchild;
	int  sendresult;
	int  tmpmask;

	int  ii;
	int fenvcnt=0;

        fenv[0]=NULL;

    
	/*
	 * open control file
	 */
	if ((cfp = fopen(file, "r")) == NULL)
		return(OK);

	/* Name the duplicate control file something else becuase
	 * if it doesn't get deleted, weird stuff will happen. */
	dup_cfpname=strdup("lpdtempXXXXXX");
	if ((ii=mkstemp(dup_cfpname))==-1) {
		syslog(LOG_ERR, "Can't create temp cfp file");
		return(OK);
	}
	close(ii);
	tmpmask = umask(007);
	if (rename(file,dup_cfpname)==-1) {
		syslog(LOG_ERR, "Can't rename cfp file");
		return(OK);
	}
	dup_cfp=fopen(file, "w");
	umask(tmpmask);
	if (dup_cfp == NULL)
	{
		syslog(LOG_ERR, "Can't create temp cfp file %s", dup_cfpname);
		return(OK);
	}
	    
	/*
	 *      read the control file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      commands of interest are:
	 *
	 *            a-z -- "file name" name of file to print
	 *		P -- "person" user's login name
	 *              U -- "unlink" name of file to remove
	 *                    (after we print it. (Pass 2 only)).
	 */

	/* pass 0 */
	/* Henri Gomez <gomez@slib.fr> 19991104 */
	/* Pre scan to get the Z flags (RFC Filter Extension) */
	while (getline(cfp))
	  switch (line[0]) {
	  case 'E':
	    /* ignore too many options */
	    if(fenvcnt!=MAXFENV-1){
	      fenv[fenvcnt++]=strdup(line+1);
	      fenv[fenvcnt]=NULL;
	    }
	    continue;
	  }

	rewind(cfp);
	/*
	 * pass 1
	 */
	while (getline(cfp)) {
	again:
	         if (line[0] == 'S') {
		        if (dup_cfp)
			  fprintf(dup_cfp,"%s\n",line);
			cp = line+1;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fdev = i;
			cp++;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fino = i;
			continue;
		}
		if (line[0] == 'P') {
		        if (dup_cfp)
			  fprintf(dup_cfp,"%s\n",line);

			strncpy(logname, line+1, sizeof(logname)-1);
			logname[sizeof(logname) - 1] = '\0';
			if (RS) {                       /* restricted */
				if (getpwnam(logname) == (struct passwd *)0) {
					sendmail(line+1, NOACCT);
					goto pass2;
				}
			} else {			/* not restricted */
				struct passwd *dude;
				dude = getpwnam(logname);
				if (dude == NULL || dude->pw_uid == 0) {
				   /* if user dne, set euid to lp */
				   strncpy(euser, "lp", 3);
				} else {
				   strncpy(euser, logname, sizeof(logname));
				}
			}
			continue;
		}

		/* a hack */
		if (line[0] < 'a' || line[0] > 'z') {
			
		  if (dup_cfp && line[0] != 'U') {
			  if (!SH) {
				  fprintf(dup_cfp,"%s\n",line);
			  } else if (line[0] != 'J' && line[0] != 'C' && line[0] != 'L') {
				  fprintf(dup_cfp,"%s\n",line);
			  }
		  }
		  continue;
		}
		  
		/* we assume that we've hit a line dedicated to printing */
		if (line[0] >= 'a' && line[0] <= 'z') {
		        char tmpfile[20];
			
			memset(tmpfile,'\0',20);
			if (dup_cfp)
			  fprintf(dup_cfp, "%s\n", line);

		        strcpy(last, line);
			/* handle multiple copies */
                       while ((i = getline(cfp))) {
                         if (dup_cfp)
                           fprintf(dup_cfp, "%s\n", line);
				if (strcmp(last, line))
					break;
                       }
			

			/* msf - here is where we hack in IF support    */
			/*       will only run the IF, no other filters */
			/*       will be run.                           */
                       /* This hack will work for multiple files only  */
                       /* if the 'f' lines are separated by at least   */
                       /* one non-'f' line (i.e. a U or N line) due to */
                       /* the multiple copies getline() while loop     */
                       /* right above this comment.                    */
			if (IF != NULL && last[0] == 'f') {
			  
			    memcpy(tmpfile, last+1, 7);
  			    memcpy(tmpfile+7, "XXXXXX", 6);
			    tmpfile[13]='\000';
			    mktemp(tmpfile);

			    /* Nasty trick of the day, to attempt
			     * to be RFC compliant */
			    if (rename(last+1,tmpfile)==-1) {
				    syslog(LOG_ERR,"can't rename data file");
				    return(ERROR);
			    }
				
			     fi = open(tmpfile, O_RDONLY);
				
			     /* Ideally here, if the open fails, we'd
			      * just bail. However, doing this instead
			      * allows us to clean up the queue nicely. 
			      */
			     if (fi==-1 ||
				 ((stb.st_mode & S_IFMT) == S_IFLNK 
				  && fstat(fi, &stb) == 0 &&
				  (stb.st_dev != fdev || stb.st_ino != fino))
				 ) {
				     syslog(LOG_ERR, "%s: couldn't open spool file %s",
					    name, last+1);
				     fi = open("/dev/null",O_RDONLY);
			     }
				
			    ifi = open(last+1,O_WRONLY|O_CREAT|O_TRUNC, 0664);
			    if ((ifchild = doforkuser(DORETURN,euser)) == 0) { /*child*/
			      char *optspace;
			      int optlen;
			      char *sp1, *sp2;

			         dup2(fi,  0); /* input file is stdin  */
  			         dup2(ifi, 1); /* temp file is  stdout */
			         for (ii = 3; ii < NOFILE; ii++)
			            (void) close(ii);
				 /* Work out the maximum space we will need (plus a bit!) */
				 optlen=strlen(LPOPT_EVAR)+10;
				 for (ii=0; fenv[ii]; ii++)
				   optlen += strlen(fenv[ii])+1;
				 optspace=malloc(optlen);
				 
				 sprintf(optspace, "%s=", LPOPT_EVAR);
				 sp1=optspace + strlen(optspace);
				 for (ii=0; fenv[ii]; ii++)
				   {
				     /* Skip over initial w/s */
				     for (sp2=fenv[ii]; isspace(*sp2); sp2++);
				     
				     if (*sp2)
				       {
					 /* copy across, converting to lower case */
					 for(; *sp2; sp2++, sp1++)
					   *sp1=tolower(*sp2);

					 /* Remove terminating w/s */
					 for (; isspace(*(sp1-1)); sp1--);
					 *sp1++ = ',';
					 *sp1 = '\0';
				       }
				   }

				 /* Remove possible terminating comma */
				 if (*(sp1-1) == ',')
				   *--sp1='\0';
	       
				 putenv(optspace);

			         ii=execl(IF, IF, 0);
			         syslog(LOG_ERR, "cannot execv %s", IF);
			         exit(2);
			    }
			  close(ifi);
                          close(fi);
			  /* wait on it to finish */
			  signal(SIGCHLD,SIG_DFL);
			  waitpid(ifchild, NULL, 0);
			 }
		      
                        sendresult = sendfile('\3', last+1);

    		        if ( IF != NULL && last[0] == 'f')
			  {
			     if (unlink(last+1) != 0) {
				     syslog(LOG_ERR, "error unlinking %s: %d",
					    last+1,errno);
			     }
			     if (rename(tmpfile,last+1)==-1) {
				     syslog(LOG_ERR, "error renaming %s: %d",
					    tmpfile, errno);
			     }
			  }

			/* clean up our temp file */
			switch (sendresult) {
			case OK:
				if (i)
					goto again;
				break;
			case REPRINT:
				(void) fclose(cfp);
				return(REPRINT);
			case ACCESS:
			case NOSTAT:
				sendmail(logname, sendresult);
			case ERROR:
				err = ERROR;
			}
			break;
		}
	}

	if (dup_cfp)
	  {
	    fclose(dup_cfp);
	    dup_cfp = NULL;
	  }
        final_cfp=file;

	if (err == OK && sendfile('\2', final_cfp) > 0) {
		(void) fclose(cfp);
		if (dup_cfp)
		  {
		    (void) fclose(dup_cfp);
		    unlink(file);
		    rename(dup_cfpname,file);
		    free(dup_cfpname);
		  }
		return(REPRINT);
	}
pass2:
	/*
	 * pass 2
	 */
	if (dup_cfp)
	  {
	    final_cfp=dup_cfpname;
	    fclose(dup_cfp);
	    dup_cfp = NULL;
	  }
	else
	    final_cfp=file;

 	fclose(cfp);

	if ((cfp = fopen(final_cfp, "r")) == NULL) {
		if (dup_cfpname) {
		    unlink(dup_cfpname);
		    free(dup_cfpname);
		}
		return(OK);
	}

	while (getline(cfp))
		if (line[0] == 'U' && strchr(line+1, '/') == NULL)
			(void) unlinkfile(line+1, from, logname);
	/*
	 * clean-up in case another control file exists
	 */
	(void) fclose(cfp);
	unlink(file);
	if (dup_cfpname) {
	    unlink(dup_cfpname);
	    free(dup_cfpname);
	}

	/*
	 * clean-up in case another control file exists
	 */
	zerofenv();
	return(err);
}

/*
 * Send a data file to the remote machine and spool it.
 * Return positive if we should try resending.
 */
static int
sendfile(type, file)
	int type;
	char *file;
{
	register int f, i, amt;
	struct stat stb;
	char buf[BUFSIZ];
	int sizerr, resp;
	
	if (lstat(file, &stb) < 0)
		return(NOSTAT);

	if ((f = open(file, O_RDONLY)) < 0)
		return(ACCESS);
	
	/*
	 * Check to see if data file is a symbolic link. If so, it should
	 * still point to the same file or someone is trying to print something
	 * he shouldn't.
	 */
	if ((stb.st_mode & S_IFMT) == S_IFLNK && fstat(f, &stb) == 0 &&
	    (stb.st_dev != fdev || stb.st_ino != fino))
		return(BADLINK);
	if (snprintf(buf, sizeof(buf), "%c%ld %s\n", type, stb.st_size, file) >
	    sizeof(buf)-1)
	  return(ACCESS);
	amt = strlen(buf);
	for (i = 0;  ; i++) {
		if (write(pfd, buf, amt) != amt ||
		    (resp = response()) < 0 || resp == '\1') {
			(void) close(f);
			return(REPRINT);
		} else if (resp == '\0')
			break;
		if (i == 0)
			pstatus("no space on remote; waiting for queue to drain");
		if (i == 10)
			syslog(LOG_ALERT, "%s: can't send to %s; queue full",
				printer, RM);
		sleep(5 * 60);
	}
	if (i)
		pstatus("sending to %s", RM);
	sizerr = 0;
	for (i = 0; i < stb.st_size; i += BUFSIZ) {
		amt = BUFSIZ;
		if (i + amt > stb.st_size)
			amt = stb.st_size - i;
		if (sizerr == 0 && read(f, buf, amt) != amt)
			sizerr = 1;
		if (write(pfd, buf, amt) != amt) {
			(void) close(f);
			return(REPRINT);
		}
	}
	(void) close(f);
	if (sizerr) {
		syslog(LOG_INFO, "%s: %s: changed size", printer, file);
		/* tell recvjob to ignore this file */
		(void) write(pfd, "\1", 1);
		return(ERROR);
	}
	if (write(pfd, "", 1) != 1 || response())
		return(REPRINT);
	return(OK);
}

/*
 * Check to make sure there have been no errors and that both programs
 * are in sync with eachother.
 * Return non-zero if the connection was lost.
 */
static char
response()
{
	char resp;

	if (read(pfd, &resp, 1) != 1) {
		syslog(LOG_INFO, "%s: lost connection", printer);
		return(-1);
	}
	return(resp);
}

/*
 * Banner printing stuff
 */
static void
banner(name1, name2)
	char *name1, *name2;
{
	time_t tvec;

	time(&tvec);
	if (!SF && !tof)
		(void) write(ofd, FF, strlen(FF));
	if (SB) {	/* short banner only */
		if (class[0]) {
			(void) write(ofd, class, strlen(class));
			(void) write(ofd, ":", 1);
		}
		(void) write(ofd, name1, strlen(name1));
		(void) write(ofd, "  Job: ", 7);
		(void) write(ofd, name2, strlen(name2));
		(void) write(ofd, "  Date: ", 8);
		(void) write(ofd, ctime(&tvec), 24);
		(void) write(ofd, "\n", 1);
	} else {	/* normal banner */
		(void) write(ofd, "\n\n\n", 3);
		scan_out(ofd, name1, '\0');
		(void) write(ofd, "\n\n", 2);
		scan_out(ofd, name2, '\0');
		if (class[0]) {
			(void) write(ofd,"\n\n\n",3);
			scan_out(ofd, class, '\0');
		}
		(void) write(ofd, "\n\n\n\n\t\t\t\t\tJob:  ", 15);
		(void) write(ofd, name2, strlen(name2));
		(void) write(ofd, "\n\t\t\t\t\tDate: ", 12);
		(void) write(ofd, ctime(&tvec), 24);
		(void) write(ofd, "\n", 1);
	}
	if (!SF)
		(void) write(ofd, FF, strlen(FF));
	tof = 1;
}

static char *
scnline(key, p, c)
	register int key;
	register char *p;
	int c;
{
	register int scnwidth;

	for (scnwidth = WIDTH; --scnwidth;) {
		key <<= 1;
		*p++ = key & 0200 ? c : BACKGND;
	}
	return (p);
}

#define TRC(q)	(((q)-' ')&0177)

static void
scan_out(scfd, scsp, dlm)
	int scfd, dlm;
	char *scsp;
{
	register char *strp;
	register int nchrs, j;
	char outbuf[LINELEN+1], *sp, c, cc;
	int d, scnhgt;
	extern char scnkey[][HEIGHT];	/* in lpdchar.c */

	for (scnhgt = 0; scnhgt++ < HEIGHT+DROP; ) {
		strp = &outbuf[0];
		sp = scsp;
		for (nchrs = 0; ; ) {
			d = dropit(c = TRC(cc = *sp++));
			if ((!d && scnhgt > HEIGHT) || (scnhgt <= DROP && d))
				for (j = WIDTH; --j;)
					*strp++ = BACKGND;
			else
				strp = scnline(scnkey[(int)c][scnhgt-1-d], strp, cc);
			if (*sp == dlm || *sp == '\0' || nchrs++ >= PW/(WIDTH+1)-1)
				break;
			*strp++ = BACKGND;
			*strp++ = BACKGND;
		}
		while (*--strp == BACKGND && strp >= outbuf)
			;
		strp++;
		*strp++ = '\n';
		(void) write(scfd, outbuf, strp-outbuf);
	}
}

static int
dropit(c)
	int c;
{
	switch(c) {

	case TRC('_'):
	case TRC(';'):
	case TRC(','):
	case TRC('g'):
	case TRC('j'):
	case TRC('p'):
	case TRC('q'):
	case TRC('y'):
		return (DROP);

	default:
		return (0);
	}
}

/*
 * Safely (I hope) unlink a file.
 */
static void
unlinkfile(file, host, user)
	char *file;
	char *host;
	char *user;
{
	/* this hack duplicates some code but things should work better */
	/* if the file does not exist, do not worry about doing unlink */
	int ret;
	extern int errno;
	if ((strchr(file,'/') == NULL) &&
	    (strstr(file,"..") == NULL)) {
		ret = unlink(file);
		if (ret == 0)
			return;
		if (errno == 2)
			return;
		syslog(LOG_ERR,"unlink file=%s, ret=%i, errno=%i",
			file, ret, errno);
	}
	
	if (dounlink(file, host, user))
		sendmail(user, UNLINK);
}

/*
 * sendmail ---
 *   tell people about job completion
 */
static void
sendmail(user, bombed)
	char *user;
	int bombed;
{
	register int i;
	int p[2], s;
	register char *cp;
	struct stat stb;
	FILE *fp;

	if (user[0] ==  '-' || user[0] == '/' || !isprint(user[0]))
	  return;
	pipe(p);
	if ((s = dofork(DORETURN)) == 0) {		/* child */
		dup2(p[0], 0);
		closelog();
		for (i = 3; i < NOFILE; i++)
			(void) close(i);
		if ((cp = strrchr(_PATH_SENDMAIL, '/')) != NULL)
			cp++;
		else
			cp = _PATH_SENDMAIL;
		execl(_PATH_SENDMAIL, cp, "-t", 0);
		exit(0);
	} else if (s > 0) {				/* parent */
		dup2(p[1], 1);
		printf("To: %s@%s\n", user, fromhost);
		printf("Subject: %s printer job \"%s\"\n", printer,
			*jobname ? jobname : "<unknown>");
		printf("Reply-To: root@%s\n\n", host);
		printf("Your printer job ");
		if (*jobname)
			printf("(%s) ", jobname);
		switch (bombed) {
		case OK:
			printf("\ncompleted successfully\n");
			cp = "OK";
			break;
		default:
		case FATALERR:
			printf("\ncould not be printed\n");
			cp = "FATALERR";
			break;
		case NOACCT:
			printf("\ncould not be printed without an account on %s\n", host);
			cp = "NOACCT";
			break;
		case FILTERERR:
			if (stat(tempfile, &stb) < 0 || stb.st_size == 0 ||
			    (fp = fopen(tempfile, "r")) == NULL) {
				printf("\nhad some errors and may not have printed\n");
				break;
			}
			printf("\nhad the following errors and may not have printed:\n");
			while ((i = getc(fp)) != EOF)
				putchar(i);
			(void) fclose(fp);
			cp = "FILTERERR";
			break;
		case BADLINK:
			printf("\nwas not printed because it was not linked to the original file\n");
			cp = "BADLINK";
			break;
                case ACCESS:
                	printf("\nwas not printed because the file could not be accessed\n");
                        cp = "ACCESS";
                        break;
                case NOSTAT:
                	printf("\nwas not printed because the daemon could not stat the file\n");
                        cp = "NOSTAT";
                        break;
		case UNLINK:
			printf("\nwas not unlinked: permission denied\n");
			break;
		}
		fflush(stdout);
		(void) close(1);
	}
	(void) close(p[0]);
	(void) close(p[1]);
	if (s != -1) {
		wait(NULL);
		syslog(LOG_INFO,
		       "mail sent to user %s about job %s on printer %s (%s)",
		       user, *jobname ? jobname : "<unknown>", printer, cp);
	}
}

/*
 * dofork - fork with retries on failure
 */
static int
dofork(action)
	int action;
{
	register int i, pid;

	for (i = 0; i < 20; i++) {
		if ((pid = fork()) < 0) {
			sleep((unsigned)(i*i));
			continue;
		}
		/*
		 * Child should run as daemon instead of root
		 */
		if (pid == 0)
			setuid(DU);
		return(pid);
	}
	syslog(LOG_ERR, "can't fork");

	switch (action) {
	case DORETURN:
		return (-1);
	default:
		syslog(LOG_ERR, "bad action (%d) to dofork", action);
		/*FALL THRU*/
	case DOABORT:
		exit(1);
	}
	/*NOTREACHED*/
}


/*
 * doforkuser - fork with retries on failure
 *          sets up as user first for security 
 */
static int
doforkuser(action, user)
	int action;
	char *user;
{
	register int i, pid;
	struct passwd *dude;
	struct group *lpgrp;
	
	/* figure out who we wish we were */
	dude = getpwnam(user);
	if (!dude) {
	  syslog(LOG_ERR, "can't find user - can't fork");

	  switch (action) {
	    case DORETURN:
	      return (-1);
	    default:
	      syslog(LOG_ERR, "bad action (%d) to dofork", action);
	      /*FALL THRU*/
	    case DOABORT:
	      exit(1);
	  }
	}
	
	/* This shouldn't fail */
	lpgrp = getgrnam("lp");
	if (!lpgrp) {
		syslog(LOG_ERR, "can't find lp group - can't fork");
		switch (action) {
		 case DORETURN:
			return (-1);
		 default:
			syslog(LOG_ERR, "bad action (%d) to dofork", action);
		 case DOABORT:
			exit(1);
		}
	}
	
	for (i = 0; i < 20; i++) {
		if ((pid = fork()) < 0) {
			sleep((unsigned)(i*i));
			continue;
		}
		/*
		 * Child should run as user instead of root
		 * Add to lp group so it can read config file.
		 */
		if (pid == 0) {
			unsetenv("BASH_ENV");
			unsetenv("ENV");
			setenv("USER",user,1);
			setgroups(1,&lpgrp->gr_gid);
			setgid(dude->pw_gid);
			setuid(dude->pw_uid);
		}
		return(pid);
	}
	syslog(LOG_ERR, "can't fork");

	switch (action) {
	case DORETURN:
		return (-1);
	default:
		syslog(LOG_ERR, "bad action (%d) to dofork", action);
		/*FALL THRU*/
	case DOABORT:
		exit(1);
	}
	/*NOTREACHED*/
}




/*
 * Kill child processes to abort current job.
 */
static void
abortpr(signo)
	int signo;
{
// Modified by DavidYang 2001/9/25
	//mydebug("recv abort signo");
	// Send finish printing message
	// reference static var: pid
	if ( pid == getpid() )
		on_print_finish();	
// End of modification
	(void) unlink(tempfile);
	kill(0, SIGINT);
	if (ofilter > 0)
		kill(ofilter, SIGCONT);
	while (wait(NULL) > 0)
		;
	exit(0);
}

static void
init()
{
	int status;
	char *s;

	if ((status = pgetent(line, printer)) < 0) {
		syslog(LOG_ERR, "can't open printer description file");
		exit(1);
	} else if (status == 0) {
		syslog(LOG_ERR, "unknown printer: %s", printer);
		exit(1);
	}
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = _PATH_DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((ST = pgetstr("st", &bp)) == NULL)
		ST = DEFSTAT;
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = _PATH_CONSOLE;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = _PATH_DEFSPOOL;
	if ((DU = pgetnum("du")) < 0)
		DU = DEFUID;
	if ((FF = pgetstr("ff", &bp)) == NULL)
		FF = DEFFF;
	if ((PW = pgetnum("pw")) < 0)
		PW = DEFWIDTH;
	sprintf(&width[2], "%d", PW);
	if ((PL = pgetnum("pl")) < 0)
		PL = DEFLENGTH;
	sprintf(&length[2], "%d", PL);
	if ((PX = pgetnum("px")) < 0)
		PX = 0;
	sprintf(&pxwidth[2], "%d", PX);
	if ((PY = pgetnum("py")) < 0)
		PY = 0;
	sprintf(&pxlength[2], "%d", PY);
	RM = pgetstr("rm", &bp);
	if ((s = checkremote()))
		syslog(LOG_WARNING, "%s", s);

	AF = pgetstr("af", &bp);
	OF = pgetstr("of", &bp);
	IF = pgetstr("if", &bp);
	RF = pgetstr("rf", &bp);
	TF = pgetstr("tf", &bp);
	NF = pgetstr("nf", &bp);
	DF = pgetstr("df", &bp);
	GF = pgetstr("gf", &bp);
	VF = pgetstr("vf", &bp);
	CF = pgetstr("cf", &bp);
	TR = pgetstr("tr", &bp);
	RS = pgetflag("rs");
	SF = pgetflag("sf");
	SH = pgetflag("sh");
	SB = pgetflag("sb");
	HL = pgetflag("hl");
	RW = pgetflag("rw");
	BR = pgetnum("br");

/* FIX for hardware handshaking.  DJM
   Added new function for these unsigned longs
*/
	FC = pgetusgn("fc");
	FS = pgetusgn("fs");
	XC = pgetusgn("xc");
	XS = pgetusgn("xs");

	tof = !pgetflag("fo");
}

/*
 * Acquire line printer or remote connection.
 */
static void
openpr()
{
	register int i, n;
	int resp;
	char *cp;

	if (!sendtorem && *LP) {
               if ((cp = strchr(LP, '@')) != NULL) {
                       char savech;
                       struct servent *sp;
                       struct hostent *hp;
                       struct sockaddr_in sin;

                       savech = *cp;
                       *cp = '\0';
                       if ((sp = getservbyname(LP, "tcp")) != NULL) {
                               sin.sin_port = sp->s_port;
                       } else if (isdigit(*LP)) {
                               sin.sin_port = htons(atoi(LP));
                       } else {
                               syslog(LOG_ERR, "%s: bad service spec", LP);
                               exit(1);
                       }
                       *cp = savech;
                       if (inet_aton(cp+1, &sin.sin_addr))
                               ;
                       else if ((hp = gethostbyname(cp+1)) != NULL)
                               sin.sin_addr.s_addr = *(u_long *)hp->h_addr;
                       else {
                               syslog(LOG_ERR, "%s: bad host spec", cp+1);
                               exit(1);
                       }
                       if ((pfd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC)) <0){
                               syslog(LOG_ERR, "socket: %m");
                               exit(1);
                       }
#ifndef __linux__
                       sin.sin_len = sizeof sin;
#endif
                       sin.sin_family = AF_INET;
                       if (connect(pfd, (struct sockaddr*)&sin, sizeof sin) <0){
                               syslog(LOG_ERR, "connect: %m");
//                               exit(1);
                       }
               } else {
		for (i = 1; ; i = i < 32 ? i << 1 : i) {
			pfd = open(LP, RW ? O_RDWR : O_WRONLY);
			if (pfd >= 0)
				break;
			if (errno == ENOENT) {
				syslog(LOG_ERR, "%s: %m", LP);
				exit(1);
			}
			if (i == 1)
				pstatus("waiting for %s to become ready (offline ?)", printer);
			sleep(i);
		}
		if (isatty(pfd))
			setty();
		}
		pstatus("%s is ready and printing", printer);
	} else if (RM != NULL) {
		for (i = 1; ; i = i < 256 ? i << 1 : i) {
			resp = -1;
			pfd = getport(RM,0);
			if (pfd >= 0) {
				char c;
				
				(void) snprintf(line, sizeof(line), "\2%s\n", RP);
				n = strlen(line);
				if (write(pfd, line, n) == n &&
				    (resp = response()) == '\0')
					break;
#ifdef SHUT_WR
				shutdown(pfd, SHUT_WR);
#else
				shutdown(pfd, 1);
#endif
				read(pfd , &c, 1);
				(void) close(pfd);
			}
			if (i == 1) {
				if (resp < 0)
					pstatus("waiting for %s to come up", RM);
				else {
					pstatus("waiting for queue to be enabled on %s", RM);
					i = 256;
				}
			}
			sleep(i);
		}
		pstatus("sending to %s", RM);
		remote = 1;
	} else {
		syslog(LOG_ERR, "%s: no line printer device or host name",
			printer);
		exit(1);
	}
	/*
	 * Start up an output filter, if needed.
	 */
	if (!remote && OF) {
		int p[2];

		pipe(p);
		if ((ofilter = dofork(DOABORT)) == 0) {	/* child */
			dup2(p[0], 0);		/* pipe is std in */
			dup2(pfd, 1);		/* printer is std out */
			closelog();
			for (i = 3; i < NOFILE; i++)
				(void) close(i);
			if ((cp = strrchr(OF, '/')) == NULL)
				cp = OF;
			else
				cp++;
			execl(OF, cp, width, length, 0);
			syslog(LOG_ERR, "%s: %s: %m", printer, OF);
			exit(1);
		}
		(void) close(p[0]);		/* close input side */
		ofd = p[1];			/* use pipe for output */
	} else {
		ofd = pfd;
		ofilter = 0;
	}
}

struct bauds {
	int	baud;
	int	speed;
} bauds[] = {
	{50,	B50},
	{75,	B75},
	{110,	B110},
	{134,	B134},
	{150,	B150},
	{200,	B200},
	{300,	B300},
	{600,	B600},
	{1200,	B1200},
	{1800,	B1800},
	{2400,	B2400},
	{4800,	B4800},
	{9600,	B9600},
	{19200,	EXTA},
	{38400,	EXTB},
	{0,	0}
};

/*
 * setup tty lines.
 */
static void
setty()
{
	struct termios ttybuf;
	register struct bauds *bp;

	if (ioctl(pfd, TCGETS, (char *)&ttybuf) < 0) {
		syslog(LOG_ERR, "%s: ioctl(TCGETS): %m", printer);
		exit(1);
	}
	if (BR > 0) {
		for (bp = bauds; bp->baud; bp++)
			if (BR == bp->baud)
				break;
		if (!bp->baud) {
			syslog(LOG_ERR, "%s: illegal baud rate %d", printer, BR);
			exit(1);
		}
		ttybuf.c_cflag = (ttybuf.c_cflag &= ~CBAUD) | bp->speed;
	}
	ttybuf.c_cflag &= ~FC;          /* not quite right! */
	ttybuf.c_cflag |= FS;           /* not quite right! */
	if (ioctl(pfd, TCSETS, (char *)&ttybuf) < 0) {
		syslog(LOG_ERR, "%s: ioctl(TCSETS): %m", printer);
		exit(1);
	}
}

#ifdef	__STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

static void
#ifdef	__STDC__
pstatus(const char *msg, ...)
#else
pstatus(msg, va_alist)
	char *msg;
        va_dcl
#endif
{
	register int fd;
	char buf[BUFSIZ];
	va_list ap;
#ifdef __STDC__
	va_start(ap, msg);
#else
	va_start(ap);
#endif

	umask(0);
	fd = open(ST, O_WRONLY|O_CREAT, 0664);
	if (fd < 0 || flock(fd, LOCK_EX) < 0) {
		syslog(LOG_ERR, "%s: %s: %m", printer, ST);
		exit(1);
	}
	ftruncate(fd, 0);
	(void)vsnprintf(buf, sizeof(buf) - 1, msg, ap);
	va_end(ap);
	strcat(buf, "\n");
	(void) write(fd, buf, strlen(buf));
	(void) close(fd);
}


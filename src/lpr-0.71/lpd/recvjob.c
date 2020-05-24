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
 * Receive printer jobs from the network, queue them and
 * start the printer daemon.
 */

#include "lp.h"
#include "pathnames.h"
#include <sys/mount.h>

char	*sp = "";
#define ack()	(void) write(1, sp, 1);

char    tfname[FILENAMELEN];	/* tmp copy of cf before linking */
char    dfname[FILENAMELEN];	/* data files */
int	minfree;		/* keep at least minfree blocks available */

static int readfile(char *file, int size);
static int noresponse();
static int chksize(int size);
static int read_number(char *fn);
static void rcleanup();
static void frecverr(char *msg, ...);
static int readjob();
extern void printjob();

void
recvjob()
{
	struct stat stb;
	char *bp = pbuf;
	int status;

	/*
	 * Perform lookup for printer name or abbreviation
	 */
	if ((status = pgetent(line, printer)) < 0)
		frecverr("cannot open printer description file");
	else if (status == 0)
		frecverr("unknown printer %s", printer);
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = _PATH_CONSOLE;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = _PATH_DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;

	(void) close(2);			/* set up log file */
	if (open(LF, O_WRONLY|O_APPEND, 0664) < 0) {
		syslog(LOG_ERR, "%s: %m", LF);
		(void) open(_PATH_DEVNULL, O_WRONLY);
	}

	if (chdir(SD) < 0)
		frecverr("%s: %s: %m", printer, SD);
	if (stat(LO, &stb) == 0) {
		if (stb.st_mode & 010) {
			/* queue is disabled */
			putchar('\1');		/* return error code */
			exit(1);
		}
	} else if (stat(SD, &stb) < 0)
		frecverr("%s: %s: %m", printer, SD);
	minfree = 2 * read_number("minfree");	/* scale KB to 512 blocks */
	signal(SIGTERM, rcleanup);
	signal(SIGPIPE, rcleanup);

	if (readjob())
		printjob();
}

/*
 * Read printer jobs sent by lpd and copy them to the spooling directory.
 * Return the number of jobs successfully transfered.
 */
static int
readjob()
{
	register int size, nfiles;
	register char *cp;
	int gotfile = 0; /* Did we just receive a control or data file? */

	ack();
	nfiles = 0;
	for (;;) {
		/*
		 * Read a command to tell us what to do
		 */
		cp = line;
		if (gotfile && (*cp == '\0')) {
			/* The protocol says that data files should be
			   terminated by a null octet. */
			cp++;
		}
		gotfile = 0;
		do {
			if ((size = read(1, cp, 1)) != 1) {
				if (size < 0)
					frecverr("%s: Lost connection",printer);
				return(nfiles);
			}
		} while (*cp++ != '\n' && (cp - line + 1) < sizeof(line));
		if (cp - line + 1 >= sizeof line)
		  frecverr("readjob overflow");
		*--cp = '\0';
		cp = line;
		switch (*cp++) {
		case '\1':	/* cleanup because data sent was bad */
			rcleanup();
			continue;

		case '\2':	/* read cf file */
			if (! strcmp(cp, printer)) {
				syslog(LOG_DEBUG,
				       "duplicate recvjob command for %s", printer);
				/* MacOS LaserWriter 8.7 lpr client
				   sends the  printer "recvjob" command
				   twice. */
				ack();
				continue;
			}
			syslog(LOG_DEBUG, "reading cf file for %s", printer);
			gotfile = 1;
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			/*
			 * host name has been authenticated, we use our
			 * view of the host name since we may be passed
			 * something different than what gethostbyaddr()
			 * returns
			 */
			if (strchr(cp, '/'))
			  frecverr("readjob: %s: illegal path name", cp);
			strncpy(cp + 6, from, sizeof(line) + line - cp - 7);
			line[sizeof(line) - 1] = '\0';
			strncpy(tfname, cp, sizeof(tfname) - 1);
			tfname[sizeof(tfname)-1] = '\0';
			tfname[0] = 't';
			if (strchr(tfname, '/'))
				frecverr("readjob: %s: illegal path name", tfname);
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}
			if (!readfile(tfname, size)) {
				rcleanup();
				continue;
			}
			if (link(tfname, cp) < 0)
				frecverr("%s: %m", tfname);
			(void) unlink(tfname);
			tfname[0] = '\0';
			nfiles++;
			syslog(LOG_DEBUG, "finished reading cf file for %s",
			       printer);
			continue;

		case '\3':	/* read df file */
			syslog(LOG_DEBUG, "reading df file for %s", printer);
			gotfile = 1;
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}
			(void) strncpy(dfname, cp, sizeof(dfname)-1);
			dfname[sizeof(dfname)-1] = '\0';
			if (index(dfname, '/'))
				frecverr("readjob: %s: illegal path name",
					dfname);
			(void) readfile(dfname, size);
			syslog(LOG_DEBUG, "finished reading df file for %s",
			       printer);
			continue;
		}
		frecverr("protocol screwup: %s", line);
	}
}

/*
 * Read files send by lpd and copy them to the spooling directory.
 */
static int
readfile(file, size)
	char *file;
	int size;
{
	register char *cp;
	char buf[BUFSIZ];
	register int i, j, amt;
	int fd, err;

	fd = open(file, O_CREAT|O_EXCL|O_WRONLY, FILMOD);
	if (fd < 0)
		frecverr("readfile: %s: illegal path name: %m", file);
	ack();
	err = 0;
	for (i = 0; i < size; i += BUFSIZ) {
		amt = BUFSIZ;
		cp = buf;
		if (i + amt > size)
			amt = size - i;
		do {
			j = read(1, cp, amt);
			if (j <= 0)
				frecverr("Lost connection");
			amt -= j;
			cp += j;
		} while (amt > 0);
		amt = BUFSIZ;
		if (i + amt > size)
			amt = size - i;
		if (write(fd, buf, amt) != amt) {
			err++;
			break;
		}
	}
	(void) close(fd);
	if (err)
		frecverr("%s: write error", file);
	if (noresponse()) {		/* file sent had bad data in it */
		if (strchr(file, '/') == NULL)
		  (void) unlink(file);
		return(0);
	}
	ack();
	return(1);
}

static int
noresponse()
{
	char resp;

	if (read(1, &resp, 1) != 1)
		frecverr("Lost connection");
	if (resp == '\0')
		return(0);
	return(1);
}

/*
 * Check to see if there is enough space on the disk for size bytes.
 * 1 == OK, 0 == Not OK.
 */
static int
chksize(size)
	int size;
{
	return 1;
}

static int
read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	if ((fp = fopen(fn, "r")) == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}

/*
 * Remove all the files associated with the current job being transfered.
 */
static void
rcleanup()
{

	if (tfname[0] && strchr(tfname, '/') == NULL)
		(void) unlink(tfname);
	if (dfname[0] && strchr(dfname, '/') == NULL) {
		if (dfname[2] >= 'A' && dfname[0] >= 'd') {
			do {
				do
				  (void) unlink(dfname);
				while (dfname[2]-- > 'A');
				dfname[2] = 'z';
			} while (dfname[0]-- > 'd');
		}
	}
	dfname[0] = '\0';
}

#include <stdarg.h>

static void
frecverr(char *msg, ...)
{
	va_list args;
	
	va_start(args, msg);
	rcleanup();
	vsyslog(LOG_ERR, msg, args);
	putchar('\1');		/* return error code */
	va_end(args);
	exit(1);
}

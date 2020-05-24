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
 * Routines and data common to all the line printer functions.
 */

#include "lp.h"
#include <stdarg.h>
#include <string.h>

int	DU;		/* daeomon user-id */
int	MX;		/* maximum number of blocks to copy */
int	MC;		/* maximum number of copies allowed */
char	*LP;		/* line printer device name */
char	*RM;		/* remote machine name */
char	*RP;		/* remote printer name */
char	*LO;		/* lock file name */
char	*ST;		/* status file name */
char	*SD;		/* spool directory */
char	*AF;		/* accounting file */
char	*LF;		/* log file for error messages */
char	*OF;		/* name of output filter (created once) */
char	*IF;		/* name of input filter (created per job) */
char	*RF;		/* name of fortran text filter (per job) */
char	*TF;		/* name of troff filter (per job) */
char	*NF;		/* name of ditroff filter (per job) */
char	*DF;		/* name of tex filter (per job) */
char	*GF;		/* name of graph(1G) filter (per job) */
char	*VF;		/* name of vplot filter (per job) */
char	*CF;		/* name of cifplot filter (per job) */
char	*PF;		/* name of vrast filter (per job) */
char	*FF;		/* form feed string */
char	*TR;		/* trailer string to be output when Q empties */
short	SH;		/* suppress header page */
short	SB;		/* short banner instead of normal header */
short	HL;		/* print header last */
short	RW;		/* open LP for reading and writing */
short	PW;		/* page width */
short	PL;		/* page length */
short	PX;		/* page width in pixels */
short	PY;		/* page length in pixels */
short	BR;		/* baud rate if lp is a tty */
ulong	FC;		/* flags to clear if lp is a tty */
ulong	FS;		/* flags to set if lp is a tty */
ulong	XC;		/* flags to clear for local mode */
ulong	XS;		/* flags to set for local mode */
short	RS;		/* restricted to those with local accounts */

char	line[BUFSIZ];
char	pbuf[BUFSIZ/2];	/* buffer for printcap strings */
char	*bp = pbuf;	/* pointer into pbuf for pgetent() */
char	*name;		/* program name */
char	*printer;	/* printer name */
char	host[MAXHOSTNAMELEN];	/* host machine name */
char	*from = host;	/* client's machine name */
int	sendtorem;	/* are we sending to a remote? */
char   *fenv[MAXFENV]; /* Extra filter enviroment */

extern uid_t	uid, euid;

/*
 * Create a TCP connection to host "rhost" at port "rport".
 * If rport == 0, then use the printer service port.
 * Most of this code comes from rcmd.c.
 */
int
getport(rhost, rport)
	char *rhost;
	int rport;
{
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	int s, timo = 1, on = 1, lport = IPPORT_RESERVED - 1;
	int err;

	/*
	 * Get the host address and port number to connect to.
	 */
	if (rhost == NULL)
		fatal("no remote host to connect to");
	bzero((char *)&sin, sizeof(sin));
	if (inet_aton(rhost, &sin.sin_addr) == 1)
	  sin.sin_family = AF_INET;
	else {
		hp = gethostbyname(rhost);
		if (hp == NULL)
		  fatal("unknown host %s", rhost);
		bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
		sin.sin_family = hp->h_addrtype;
	}
	if (rport == 0) {
		sp = getservbyname("printer", "tcp");
		if (sp == NULL)
		  fatal("printer/tcp: unknown service");
		sin.sin_port = sp->s_port;
	} else
	  sin.sin_port = htons(rport);

	/*
	 * Try connecting to the server.
	 */
retry:
	seteuid(euid);
	s = rresvport(&lport);
	seteuid(uid);
	if (s < 0)
		return(-1);
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		err = errno;
		(void) close(s);
		errno = err;
		if (errno == EADDRINUSE) {
			lport--;
			goto retry;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		return(-1);
	}

	/* Don't bother if we get an error here.  */
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);
	return(s);
}

/*
 * Getline reads a line from the control file cfp, removes tabs, converts
 *  new-line to null and leaves it in line.
 * Returns 0 at EOF or the number of characters read.
 */
int
getline(cfp)
	FILE *cfp;
{
	register int linel = 0;
	register char *lp = line;
	register int c;

	while ((c = getc(cfp)) != '\n' && linel+1 < sizeof(line)) {
		if (c == EOF)
			return(0);
		if (c == '\t') {
			do {
				*lp++ = ' ';
				linel++;
			} while ((linel & 07) != 0 && linel+1 < sizeof(line));
			continue;
		}
		*lp++ = c;
		linel++;
	}
	*lp++ = '\0';
	return(linel);
}

/*
 * Scan the current directory and make a list of daemon files sorted by
 * creation time.
 * Return the number of entries and a pointer to the list.
 */
static int compar();

int
getq(namelist)
	struct queue *(*namelist[]);
{
	register struct direct *d;
	register struct queue *q, **queue;
	register int nitems;
	struct stat stbuf;
	DIR *dirp;
	int arraysz;
//	static int compar();

	if ((dirp = opendir(SD)) == NULL)
		return(-1);
	if (fstat(dirfd(dirp), &stbuf) < 0)
		goto errdone;

	/*
	 * Estimate the array size by taking the size of the directory file
	 * and dividing it by a multiple of the minimum size entry. 
	 */
	arraysz = (stbuf.st_size / 24);
	queue = (struct queue **)malloc(arraysz * sizeof(struct queue *));
	if (queue == NULL)
		goto errdone;

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (d->d_name[0] != 'c' || d->d_name[1] != 'f')
			continue;	/* daemon control files only */
		seteuid(euid);
		if (stat(d->d_name, &stbuf) < 0)
			continue;	/* Doesn't exist */
		seteuid(uid);
		q = (struct queue *)malloc(sizeof(struct queue));
		if (q == NULL)
			goto errdone;
		q->q_time = stbuf.st_mtime;
		strncpy(q->q_name, d->d_name, 255);
		q->q_name[255] = '\0';
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (++nitems > arraysz) {
			queue = (struct queue **)realloc((char *)queue,
				(stbuf.st_size/12) * sizeof(struct queue *));
			if (queue == NULL)
				goto errdone;
		}
		queue[nitems-1] = q;
	}
	closedir(dirp);
	if (nitems)
		qsort(queue, nitems, sizeof(struct queue *), compar);
	*namelist = queue;
	return(nitems);

errdone:
	closedir(dirp);
	return(-1);
}

/*
 * Compare modification times.
 */
static int
compar(p1, p2)
	register struct queue **p1, **p2;
{
	if ((*p1)->q_time < (*p2)->q_time)
		return(-1);
	if ((*p1)->q_time > (*p2)->q_time)
		return(1);
	return(0);
}

/*
 * Figure out whether the local machine is the same
 * as the remote machine (RM) entry (if it exists).
 */
char *
checkremote()
{
	char name[MAXHOSTNAMELEN];
	register struct hostent *hp;
	static char errbuf[128];
	char *rp, *rp_b;

	sendtorem = 0;	/* assume printer is local */
	if (RM != (char *)NULL) {
		/* get the official name of the local host */
		gethostname(name, sizeof(name));
		name[sizeof(name)-1] = '\0';
		hp = gethostbyname(name);
		if (hp == (struct hostent *) NULL) {
		    (void) snprintf(errbuf, sizeof(errbuf),
			"unable to get official name for local machine %s",
			name);
		    return errbuf;
		} else (void) strncpy(name, hp->h_name, MAXHOSTNAMELEN);

		/* get the official name of RM */
		hp = gethostbyname(RM);
		if (hp == (struct hostent *) NULL) {
		    (void) snprintf(errbuf, sizeof(errbuf),
			"unable to get official name for remote machine %s",
			RM);
		    return errbuf;
		}

		/*
		 * if the two hosts are not the same,
		 * then the printer must be remote.
		 */
		if (strcasecmp(name, hp->h_name) != 0)
			sendtorem = 1;
		else if (pgetstr("rp", &rp) > 0) {
			if (pgetent(rp_b, rp) == 0) {
				if (strcasecmp(rp_b, printer) != 0)
				  sendtorem = 1;
				free(rp_b);
			} else {
				(void) snprintf(errbuf, sizeof(errbuf),
						"can't find (local) remote printer %s",
						rp);
				free(rp);
				return errbuf;
			}
			free(rp);
		}
	}
	return NULL;
}

/*
 * Figure out whether the local machine is the same
 * as the remote machine (fromhost).
 */
int
checkfromremote(fromhost)
       char *fromhost;
{
       char name[MAXHOSTNAMELEN];
       register struct hostent *hp;
       static char errbuf[128];

       sendtorem = 0;  /* assume printer is local */
       /* get the official name of the local host */
       gethostname(name, sizeof(name));
       name[sizeof(name)-1] = '\0';
       hp = gethostbyname(name);
       if (hp == (struct hostent *) NULL) {
           (void) snprintf(errbuf, 128,
               "unable to get official name for local machine %s",
               name);
           return 1;
       } else (void) strncpy(name, hp->h_name, MAXHOSTNAMELEN);

       /* get the official name of fromhost */
       hp = gethostbyname(fromhost);
       if (hp == (struct hostent *) NULL) {
           (void) snprintf(errbuf, 128,
               "unable to get official name for remote machine %s",
               fromhost);
           return 1;
       }

       /*
        * if the two hosts are not the same,
        * then the printer must be remote.
        */
       return strcmp(name, hp->h_name) != 0;
}

void
fatal(const char *msg, ...)
{
	va_list args;
	
	va_start(args, msg);
	if (from != host)
		printf("%s: ", host);
	printf("%s: ", name);
	if (printer)
		printf("%s: ", printer);
	vprintf(msg, args);
	putchar('\n');
	va_end(args);
	exit(1);
}

/* Zero out the filter enviroment table and free up the space
*/
void zerofenv()
{
       int fenvcnt;

       for (fenvcnt=0; fenv[fenvcnt]; fenvcnt++)
               free(fenv[fenvcnt]);
       fenv[0]=NULL;
}

/* Convert a string to lower case in place
*/
void strtolower(char *s)
{
       for (; *s; s++)
               *s=tolower(*s);
}

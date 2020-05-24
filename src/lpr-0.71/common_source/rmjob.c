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
 * rmjob - remove the specified jobs from the queue.
 */

#include "lp.h"
#include "pathnames.h"

/*
 * Stuff for handling lprm specifications
 */
extern char	*user[];		/* users to process */
extern int	users;			/* # of users in user array */
extern int	requ[];			/* job number of spool entries */
extern int	requests;		/* # of spool requests */
extern char	*person;		/* name of person doing lprm */

static char	root[] = "root";
// Modified by DavidYang 2001/9/28 for NAS, administrator is the actual super user
static char	admin[] = "administrator";
// end of modification
static int	all = 0;		/* eliminate all files (root only) */
static int	cur_daemon;		/* daemon's pid */
static char	current[FILENAMELEN];	/* active control file name */

static int	iscf();
static void	process();
static void	rmremote();

extern uid_t	uid, euid;

// Added by DavidYang 2001/9/25
#include <sys/time.h>
void mydebug(char *thestr)
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
//End of addition

void
rmjob()
{
	register int i, nitems;
	int assasinated = 0;
	struct dirent **files;
	char *cp;

	if ((i = pgetent(line, printer)) < 0)
		fatal("cannot open printer description file");
	else if (i == 0)
		fatal("unknown printer");
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = _PATH_DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = _PATH_DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	RM = pgetstr("rm", &bp);
	if ((cp = checkremote()))
		printf("Warning: %s\n", cp);

	/*
	 * If the format was `lprm -' and the user isn't the super-user,
	 *  then fake things to look like he said `lprm user'.
	 */
	if (users < 0) {
		if (getuid() == 0)
			all = 1;	/* all files in local queue */
		else {
			user[0] = person;
			users = 1;
		}
	}
	if (!strcmp(person, "-all")) {
		if (from == host)
			fatal("The login name \"-all\" is reserved");
		all = 1;	/* all those from 'from' */
		person = root;
	}
	
	seteuid(euid);
	if (chdir(SD) < 0)
		fatal("cannot chdir to spool directory");
	if ((nitems = scandir(".", &files, iscf, NULL)) < 0)
		fatal("cannot access spool directory");
	seteuid(uid);

	if (nitems) {
		/*
		 * Check for an active printer daemon (in which case we
		 *  kill it if it is reading our file) then remove stuff
		 *  (after which we have to restart the daemon).
		 */
		if (lockchk(LO) && chk(current)) {
			seteuid(euid);
			assasinated = kill(cur_daemon, SIGINT) == 0;
			seteuid(uid);
			if (!assasinated)
				fatal("cannot kill printer daemon");
// Add by DavidYang 2001/10/3 wait for the previous daemon exits
			else {
				//char deb[50];
				//mydebug("wait current daemon");
				//sprintf(deb, "use euid:%d replaces uid:%d", euid, uid);
				//mydebug(deb);
				seteuid(euid);
				while(kill(cur_daemon, 0)!=-1)
					;
				seteuid(uid);
				//mydebug("wait current daemon over!");
			}
// end of addition
		}
		/*
		 * process the files
		 */
		for (i = 0; i < nitems; i++)
			process(files[i]->d_name);
	}
	rmremote();
	/*
	 * Restart the printer daemon if it was killed
	 */
	if (assasinated && !startdaemon(printer))
		fatal("cannot restart printer daemon\n");
	exit(0);
}

/*
 * Process a lock file: collect the pid of the active
 *  daemon and the file name of the active spool entry.
 * Return boolean indicating existence of a lock file.
 */
int
lockchk(s)
	char *s;
{
	register FILE *fp;
	register int i, n;

	seteuid(euid);
	if ((fp = fopen(s, "r")) == NULL) {
		if (errno == EACCES)
			fatal("can't access lock file");
		else
			return(0);
	}
	seteuid(uid);
	if (!getline(fp)) {
		(void) fclose(fp);
		return(0);		/* no daemon present */
	}
	cur_daemon = atoi(line);
	if (kill(cur_daemon, 0) < 0 && errno != EPERM) {
		(void) fclose(fp);
		return(0);		/* no daemon present */
	}
	for (i = 1; (n = fread(current, sizeof(char), sizeof(current), fp)) <= 0; i++) {
		if (i > 5) {
			n = 1;
			break;
		}
		sleep(i);
	}
	current[n-1] = '\0';
	(void) fclose(fp);
	return(1);
}

/*
 * Process a control file.
 */
void
process(file)
	char *file;
{
	FILE *cfp;
	int ret;
	char logname[32] = "";

	if (!chk(file))
		return;
	seteuid(euid);
	if ((cfp = fopen(file, "r")) == NULL)
		fatal("cannot open %s", file);
	seteuid(uid);
	while (getline(cfp)) {
		switch (line[0]) {
		case 'P':
			strncpy(logname, line+1, sizeof(logname)-1);
			logname[sizeof(logname)-1] = '\0';
			break;
		case 'U':  /* unlink associated files */
			if (strchr(line+1, '/'))
			    break;
			if (from != host)
				printf("%s: ", host);
			if (dounlink(line+1, from, logname))
				printf("cannot dequeue %s\n", line+1);
			else
				printf("%s dequeued\n", line+1);
			break;
		}
	}
	(void) fclose(cfp);
	if (from != host)
		printf("%s: ", host);
	seteuid(euid);
	ret = unlink(file);
	seteuid(uid);
	printf(ret ? "cannot dequeue %s\n" : "%s dequeued\n", file);
}

/*
 * Do the dirty work in checking
 */
int
chk(file)
	char *file;
{
	register int *r, n;
	char **u, *cp, p[64], h[257];
	FILE *cfp;

	/*
	 * Check for valid cf file name (mostly checking current).
	 */
	if (strlen(file) < 7 || file[0] != 'c' || file[1] != 'f')
		return(0);

	
	if (all && (from == host || !strcmp(from, file+6)))
	  return(1);
	
	/*
	 * get the owner's name from the control file.
	 */
	p[0] = h[0] = '\0';
	seteuid(euid);
	if ((cfp = fopen(file, "r")) == NULL)
		return(0);
	seteuid(uid);
	while (getline(cfp)) {
		if (line[0] == 'P') {
			strncpy(p, line+1, sizeof(p)-1);
			p[sizeof(p)-1] = '\0';
		} else if (line[0] == 'H') {
			strncpy(h, line+1, sizeof(h)-1);
			h[sizeof(h)-1] = '\0';
		}
	}
	(void) fclose(cfp);

	if (all && (from == host || !strcmp(from, h)))
		return(1);

	if (users == 0 && requests == 0)
		return(!strcmp(file, current) && isowner(p, file, h));
	/*
	 * Check the request list
	 */
	for (n = 0, cp = file+3; isdigit(*cp); )
		n = n * 10 + (*cp++ - '0');
	for (r = requ; r < &requ[requests]; r++)
		if (*r == n && isowner(p, file, h))
			return(1);
	/*
	 * Check to see if it's in the user list
	 */
	for (u = user; u < &user[users]; u++)
		if (!strcmp(*u, p) && isowner(p, file, h))
			return(1);
	return(0);
}

/*
 * If root is removing a file on the local machine, allow it.
 * If root is removing a file from a remote machine, only allow
 * files sent from the remote machine to be removed.
 * Normal users can only remove the file from where it was sent.
 */
int
isowner(owner, file, h)
	char *owner, *file, *h;
{
// Modified by DavidYang 2001/9/28 for NAS, administrator is the actual super user
	if ((!strcmp(person, root) || !strcmp(person, admin)) && (from == host || !strcmp(from, h)))
// end of modification
		return(1);
	if (!strcmp(person, owner) && !strcmp(from, h))
		return(1);
	if (from != host)
		printf("%s: ", host);
	printf("%s: That's a real tregedy! The Hell Permission denied\nPerson:%s\nFrom:%s\nHost:%s\n", file, person, from, host);
	return(0);
}

/*
 * Check to see if we are sending files to a remote machine. If we are,
 * then try removing files on the remote machine.
 */
void
rmremote()
{
	register char *cp;
	register int i, rem;
	char buf[BUFSIZ];

	if (!sendtorem)
		return;	/* not sending to a remote machine */

	/*
	 * Flush stdout so the user can see what has been deleted
	 * while we wait (possibly) for the connection.
	 */
	fflush(stdout);

	(void)snprintf(buf, sizeof(buf)-2, "\5%s %s", RP, all ? "-all" : person);
	cp = buf + strlen(buf);
	for (i = 0; i < users && cp-buf+1+strlen(user[i]) < sizeof buf - 2; i++) {
		cp += strlen(cp);
		*cp++ = ' ';
		strcpy(cp, user[i]);
	}
	for (i = 0; i < requests && cp-buf+10 < sizeof(buf) - 2; i++) {
		cp += strlen(cp);
		(void) sprintf(cp, " %d", requ[i]);
	}
	strcat(cp, "\n");
	rem = getport(RM,0);
	if (rem < 0) {
		if (from != host)
			printf("%s: ", host);
		printf("connection to %s is down\n", RM);
	} else {
		i = strlen(buf);
		if (write(rem, buf, i) != i)
			fatal("Lost connection");
		while ((i = read(rem, buf, sizeof(buf))) > 0)
			(void) fwrite(buf, 1, i, stdout);
		(void) close(rem);
	}
}

/*
 * Unlink a file.
 */
int
dounlink(file, host, user)
	char *file;
	char *host;
	char *user;
{
	uid_t		fsuid = geteuid();
	gid_t		fsgid = getegid();
	int		ret;

	if (file[0] != '/' && (strstr(file, "..") == NULL))
		return unlink(file);	/* unlink file in spool directory */

	if (checkfromremote(host))      /* can't unlink file on remote host! */
		return 0;

	/* Shouldn't really assume success */
	setfsuid(getuid());
	setfsgid(getgid());

	ret = unlink(file);

	setfsuid(fsuid);
	setfsgid(fsgid);

	return ret;
}

/*
 * Return 1 if the filename begins with 'cf'
 */
int
iscf(d)
	struct dirent *d;
{
	return(d->d_name[0] == 'c' && d->d_name[1] == 'f');
}

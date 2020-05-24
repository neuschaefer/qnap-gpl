/* utelnetd.c
 *
 * Simple telnet server
 *
 * Bjorn Wesen, Axis Communications AB 
 * <bjornw@axis.com>
 * 
 * Joerg Schmitz-Linneweber, Aston GmbH
 * <schmitz-linneweber@aston-technologie.de>
 *
 * Vladimir Oleynik
 * <dzo@simtreas.ru>
 *
 * Robert Schwebel, Pengutronix
 * <r.schwebel@pengutronix.de>
 * 
 *
 * This file is distributed under the GNU General Public License (GPL),
 * please see the file LICENSE for further information.
 * 
 * ---------------------------------------------------------------------------
 * (C) 2000, 2001, 2002 by the authors mentioned above
 * ---------------------------------------------------------------------------
 *
 * The telnetd manpage says it all:
 *
 *   Telnetd operates by allocating a pseudo-terminal device (see pty(4))  for
 *   a client, then creating a login process which has the slave side of the
 *   pseudo-terminal as stdin, stdout, and stderr. Telnetd manipulates the
 *   master side of the pseudo-terminal, implementing the telnet protocol and
 *   passing characters between the remote client and the login process.
 */

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#ifdef DEBUG
#define TELCMDS
#define TELOPTS
#endif
#include <arpa/telnet.h>
#include <ctype.h>
//Richard Chang 20070827 add for onlineuser
#define STATISTICS      1
#ifdef STATISTICS
#include <debug.h>
#include <qLogEngine.h>
#endif /* STATISTICS */
#include <Util.h>
#define BUFSIZE 4000

#define MIN(a,b) ((a) > (b) ? (b) : (a))

static char *loginpath = NULL;

/* shell name and arguments */

static char *argv_init[] = {NULL, NULL};

/* structure that describes a session */

struct tsession {
	struct tsession *next;
	int sockfd, ptyfd;
	int shell_pid;
	/* two circular buffers */
	char *buf1, *buf2;
	int rdidx1, wridx1, size1;
	int rdidx2, wridx2, size2;
};

#ifdef DEBUG
#define DEBUG_OUT(...) fprintf(stderr, __VA_ARGS__)
#else
static inline void DEBUG_OUT(const char *format, ...) {};
#endif

/*

   This is how the buffers are used. The arrows indicate the movement
   of data.

   +-------+     wridx1++     +------+     rdidx1++     +----------+
   |       | <--------------  | buf1 | <--------------  |          |
   |       |     size1--      +------+     size1++      |          |
   |  pty  |                                            |  socket  |
   |       |     rdidx2++     +------+     wridx2++     |          |
   |       |  --------------> | buf2 |  --------------> |          |
   +-------+     size2++      +------+     size2--      +----------+

   Each session has got two buffers.

*/

static int maxfd;

static struct tsession *sessions;
//Richard Chang 20071105 add
#define TELNET_STATE "/root/.telnet_state"
#define TELNET_USER_NAME "User Name"
#define TELNET_USER_IP "IP"
#define TELNET_USER_STATE "STATE"

#ifdef QNAP_SERVICE_BINDING
	// Constants for service binding feature.
	#define	MAX_BINDING_ADDRESS		17			// Maximum binding address.(16 ethx + 1 lo)
	#define	SZP_GET_BIND_ADDR		"/sbin/getbindaddr"
	#define	SZ_INADDR_ANY			"0.0.0.0"

	// The structure for binding address information.
	typedef	struct _T_BIND_ADDR_
	{
		int		bEnabled;
		int		cnIpv4;
		int		cnIpv6;
		char	ryszIpv4[MAX_BINDING_ADDRESS][INET_ADDRSTRLEN];
		char	ryszIpv6[MAX_BINDING_ADDRESS][INET6_ADDRSTRLEN];
	} T_BIND_ADDR, *PT_BIND_ADDR;
	
	// The flag for Hup / Ctrl-C signals
	int  g_bHup=1;
	
	/**
	 * \brief	The SIGINT signal handler. (Ctrl-C)
	 */
	static void Signal_Handler_SigHup(int idSig)
	{
		g_bHup = TRUE;
	}
	
	/**
	 * \brief	Get the service binding address.
	 * \param	ptBindAddr	The buffer to the service binding settings.
	 * \return	0 if function success, or -XXX if error.
	 */
	static int Get_Binding_Address(PT_BIND_ADDR ptBindAddr)
	{
		int  cbAddr;
		char  szLine[256];
		FILE  *pfAddr = NULL;
		struct stat  tStat;

		// Reset the inet address first.
		ptBindAddr->bEnabled = FALSE;
		ptBindAddr->cnIpv4 = ptBindAddr->cnIpv6 = 0;

		// Just report no binding address if the getbindaddr is not existed.
		if (-1 == stat(SZP_GET_BIND_ADDR, &tStat))  return 0;
		if (NULL == (pfAddr = popen(SZP_GET_BIND_ADDR " TELNET 2>/dev/null", "r")))  return 0;
		while (NULL != fgets(szLine, sizeof(szLine), pfAddr))
		{
			szLine[sizeof(szLine)-1] = '\0';
			if (0 >= (cbAddr = strlen(szLine)))  continue;
			if ('\n' == szLine[cbAddr-1])  szLine[cbAddr-1] = '\0';
			if (NULL != strchr(szLine, ':'))
			{
				strcpy(ptBindAddr->ryszIpv6[ptBindAddr->cnIpv6], szLine);
				ptBindAddr->cnIpv6 ++;
			}
			else if (NULL != strchr(szLine, '.'))
			{
				if (strstr(SZ_INADDR_ANY, szLine))
				{
					ptBindAddr->bEnabled = FALSE;
					ptBindAddr->cnIpv4 = ptBindAddr->cnIpv6 = 0;
					break;
				}
				strcpy(ptBindAddr->ryszIpv4[ptBindAddr->cnIpv4], szLine);
				ptBindAddr->cnIpv4 ++;
			}
		}
		pclose(pfAddr);
		ptBindAddr->bEnabled = ptBindAddr->cnIpv4 || ptBindAddr->cnIpv6;
		return 0;
	}


	/**
	 * \brief	Check if a socket should be accepted by the service binding address or not.
	 * \param	fdAccept	The socket FD.
	 * \param	ptBindAddr	The buffer to the service binding settings.
	 * \param	pszAddrBuf	The buffer to get the incoming address.
	 * \return	0 if function success, or -XXX if error.
	 */
	static int Check_Binding_Address(int fdAccept, PT_BIND_ADDR ptBindAddr, char *pszAddrBuf)
	{
		int  idx, iRet = 0;
		socklen_t  cbSktAddr;
		char  *pszAddr=pszAddrBuf;
		struct sockaddr_storage  tsaLocal;
		
		if (ptBindAddr->bEnabled)
		{
			cbSktAddr = sizeof(tsaLocal);
			if (-1 == getsockname(fdAccept, (struct sockaddr *)&tsaLocal, &cbSktAddr))
			{
				goto exit_check_bind_addr;
			}
			if ((AF_INET == tsaLocal.ss_family) && (0 < ptBindAddr->cnIpv4))
			{
				struct sockaddr_in *psiLocal = (struct sockaddr_in *)&tsaLocal;
				strcpy(pszAddr, inet_ntoa(psiLocal->sin_addr));
				for (idx=0; (idx<ptBindAddr->cnIpv4) && strcmp(pszAddr, ptBindAddr->ryszIpv4[idx]); idx++);
				iRet = (idx >= ptBindAddr->cnIpv4) ? -ENOTTY : 0;
			}
			else if ((AF_INET6 == tsaLocal.ss_family) && (0 < ptBindAddr->cnIpv6))
			{
				struct sockaddr_in6 *psiLocal = (struct sockaddr_in6 *)&tsaLocal;
				inet_ntop(tsaLocal.ss_family, &psiLocal->sin6_addr, pszAddr, INET6_ADDRSTRLEN);
				// Make sure the IP is real IPv6 or IPv4 expressed in IPv6
				if (!memcmp("::ffff:", pszAddr, 7))
				{
					pszAddr += 7;
					for (idx=0; (idx<ptBindAddr->cnIpv4) && strcmp(pszAddr, ptBindAddr->ryszIpv4[idx]); idx++);
					iRet = (idx >= ptBindAddr->cnIpv4) ? -ENOTTY : 0;
				}
				else
				{
					for (idx=0; (idx<ptBindAddr->cnIpv6) && strcmp(pszAddr, ptBindAddr->ryszIpv6[idx]); idx++);
					iRet = (idx >= ptBindAddr->cnIpv6) ? -ENOTTY : 0;
				}
			}
			else
			{
				iRet = -ENOTTY;
			}
		}
	exit_check_bind_addr:
		return iRet;
	}
#endif

int write_telnet_ip(int pid, char *user_IP)
{
	char telnet_pid[16] = "0";
	FILE		*fp = NULL;
	struct stat 	fconf;
	int		fstat = 0;
	
	fstat=stat(TELNET_STATE, &fconf);
	if (fstat==-1)
	{
		fp=fopen(TELNET_STATE, "w+");
		fclose(fp);
	}
	sprintf(telnet_pid, "%d", pid);
	WritePrivateProfileString(telnet_pid, TELNET_USER_IP, user_IP, TELNET_STATE); 
	return 0;
}

int get_telnet_ip(int pid, char *user_IP)
{
	char telnet_pid[16] = "0";
	char buf[32] = "";
	sprintf(telnet_pid, "%d", pid);
	GetPrivateProfileString(telnet_pid, TELNET_USER_IP, "0.0.0.0", buf, sizeof(buf), TELNET_STATE);
	strcpy(user_IP, buf);
	return 0;
}

int write_telnet_state(int pid, char *user_name,  char *user_state)
{
	char telnet_pid[16] = "0";
	FILE		*fp = NULL;
	struct stat 	fconf;
	int		fstat = 0;
	
	fstat=stat(TELNET_STATE, &fconf);
	if (fstat==-1)
	{
		fp=fopen(TELNET_STATE, "w+");
		fclose(fp);
	}
	sprintf(telnet_pid, "%d", pid);
	WritePrivateProfileString(telnet_pid, TELNET_USER_NAME, user_name, TELNET_STATE);
	WritePrivateProfileString(telnet_pid, TELNET_USER_STATE, user_state, TELNET_STATE);
	return 0;
}

int get_telnet_state(int pid, char *user_name,  char *user_state)
{
	char telnet_pid[16] = "0";
	char buf[128] = "";
	
	sprintf(telnet_pid, "%d", pid);
	GetPrivateProfileString(telnet_pid, TELNET_USER_NAME, "---", buf, sizeof(buf), TELNET_STATE);
	strcpy(user_name, buf);
	GetPrivateProfileString(telnet_pid, TELNET_USER_STATE, "---", buf, sizeof(buf), TELNET_STATE);
	strcpy(user_state, buf);
	return 0;
}

int remove_telnet_sate(int pid)
{
	char telnet_pid[16] = "0";
	char buf[128] = "";
	
	sprintf(telnet_pid, "%d", pid);
	Conf_Remove_Section(TELNET_STATE, telnet_pid);
	return 0;
}
//end
//Richard Chang 20070910 add log
static void add_logout_log(int pid)
{
	char user_IP[32]="0.0.0.0";
	char user_name[128]="---";
	char user_state[128]="---";
	
	get_telnet_ip(pid, user_IP);
	get_telnet_state(pid, user_name,  user_state);
	
	if(strcmp(user_state , "success")==0){
		SendConnToLogEngine(EVENT_TYPE_INFO, user_name, user_IP, 
			"---", CONN_SERV_TELNET, CONN_ACTION_LOGOUT,  "---");
	}
	remove_telnet_sate(pid);
}
//end
/* 
 * This code was ported from a version which was made for busybox. 
 * So we have to define some helper functions which are originally 
 * available in busybox...
 */  

void show_usage(void)
{
	printf("Usage: telnetd [-p port] [-l loginprogram] [-d]\n");
	printf("\n");
	printf("   -p port          specify the tcp port to connect to\n");
	printf("   -l loginprogram  program started by the server\n");
	printf("   -d               daemonize\n");
	printf("\n");         
	exit(1);
}

void perror_msg_and_die(char *text)
{
	fprintf(stderr,text);
	exit(1);
}

void error_msg_and_die(char *text, char *foo)
{
	perror_msg_and_die(text);
}


/* 
   Remove all IAC's from the buffer pointed to by bf (recieved IACs are ignored
   and must be removed so as to not be interpreted by the terminal).  Make an
   uninterrupted string of characters fit for the terminal.  Do this by packing
   all characters meant for the terminal sequentially towards the end of bf. 

   Return a pointer to the beginning of the characters meant for the terminal.
   and make *processed equal to the number of characters that were actually
   processed and *num_totty the number of characters that should be sent to
   the terminal.  
   
   Note - If an IAC (3 byte quantity) starts before (bf + len) but extends
   past (bf + len) then that IAC will be left unprocessed and *processed will be
   less than len.
  
   FIXME - if we mean to send 0xFF to the terminal then it will be escaped,
   what is the escape character?  We aren't handling that situation here.

  */
static char *
remove_iacs(unsigned char *bf, int len, int *processed, int *num_totty) {
    unsigned char *ptr = bf;
    unsigned char *totty = bf;
    unsigned char *end = bf + len;
   
    while (ptr < end) {
	if (*ptr != IAC) {
	    *totty++ = *ptr++;
	}
	else {
	    if ((ptr+2) < end) {
		/* the entire IAC is contained in the buffer 
		   we were asked to process. */
//		DEBUG_OUT("Ignoring IAC 0x%02x, %s, %s\n", *ptr, TELCMD(*(ptr+1)), TELOPT(*(ptr+2)));
		ptr += 3;
	    } else {
		/* only the beginning of the IAC is in the 
		   buffer we were asked to process, we can't
		   process this char. */
		break;
	    }
	}
    }

    *processed = ptr - bf;
    *num_totty = totty - bf;
    /* move the chars meant for the terminal towards the end of the 
       buffer. */
    return memmove(ptr - *num_totty, bf, *num_totty);
}


static int getpty(char *line)
{
        struct stat stb;
        int p;
        int i;
        int j;

        //p = getpt();
	p = open("/dev/ptmx",O_RDWR|O_NOCTTY);
        if (p < 0) {
//                DEBUG_OUT("getpty(): couldn't get pty\n");
                close(p);
                return -1;
        }
        if (grantpt(p)<0 || unlockpt(p)<0) {
//                DEBUG_OUT("getpty(): couldn't grant and unlock pty\n");
                close(p);
                return -1;
        }
//        DEBUG_OUT("getpty(): got pty %s\n",ptsname(p));
        strcpy(line, (const char*)ptsname(p));

        return(p);
}


static void
send_iac(struct tsession *ts, unsigned char command, int option)
{
	/* We rely on that there is space in the buffer for now.  */
	char *b = ts->buf2 + ts->rdidx2;
	*b++ = IAC;
	*b++ = command;
	*b++ = option;
	ts->rdidx2 += 3;
	ts->size2 += 3;
}


static struct tsession *
make_new_session(int sockfd)
{
	struct termios termbuf;
	int pty, pid;
	static char tty_name[32];
	struct tsession *ts = (struct tsession *)malloc(sizeof(struct tsession));
	int t1, t2;
	
	ts->buf1 = (char *)malloc(BUFSIZE);
	ts->buf2 = (char *)malloc(BUFSIZE);

	ts->sockfd = sockfd;

	ts->rdidx1 = ts->wridx1 = ts->size1 = 0;
	ts->rdidx2 = ts->wridx2 = ts->size2 = 0;

	/* Got a new connection, set up a tty and spawn a shell.  */

	pty = getpty(tty_name);

	if (pty < 0) {
		fprintf(stderr, "All network ports in use!\n");
		return 0;
	}

	if (pty > maxfd)
		maxfd = pty;

	ts->ptyfd = pty;

	/* Make the telnet client understand we will echo characters so it 
	 * should not do it locally. We don't tell the client to run linemode,
	 * because we want to handle line editing and tab completion and other
	 * stuff that requires char-by-char support.
	 */

	send_iac(ts, DO, TELOPT_ECHO);
	send_iac(ts, DO, TELOPT_LFLOW);
	send_iac(ts, WILL, TELOPT_ECHO);
	send_iac(ts, WILL, TELOPT_SGA);


	if ((pid = fork()) < 0) {
		perror("fork");
	}
	if (pid == 0) {
		/* In child, open the child's side of the tty.  */
		int i, t;

		for(i = 0; i <= maxfd; i++)
			close(i);
		/* make new process group */
		if (setsid() < 0)
			perror_msg_and_die("setsid");

		//t = open(tty_name, O_RDWR | O_NOCTTY);
		t = open(tty_name, O_RDWR);
		if (t < 0)
			perror_msg_and_die("Could not open tty");

		t1 = dup(0);
		t2 = dup(1);

		tcsetpgrp(0, getpid());
  
		/* The pseudo-terminal allocated to the client is configured to operate in
		 * cooked mode, and with XTABS CRMOD enabled (see tty(4)).
		 */

		tcgetattr(t, &termbuf);
		termbuf.c_lflag |= ECHO; /* if we use readline we dont want this */
		termbuf.c_oflag |= ONLCR|XTABS;
		termbuf.c_iflag |= ICRNL;
		termbuf.c_iflag &= ~IXOFF;
		/* termbuf.c_lflag &= ~ICANON; */
		tcsetattr(t, TCSANOW, &termbuf);

//		DEBUG_OUT("stdin, stdout, etderr: %d %d %d\n", t, t1, t2);

		/* exec shell, with correct argv and env */
		execv(loginpath, argv_init);
		
	    	/* NOT REACHED */
		perror_msg_and_die("execv");
	}

	ts->shell_pid = pid;

	return ts;
}

static void
free_session(struct tsession *ts)
{
	struct tsession *t = sessions;

	/* Unlink this telnet session from the session list.  */
	if(t == ts)
		sessions = ts->next;
	else {
		while(t->next != ts)
			t = t->next;
		t->next = ts->next;
	}
	add_logout_log(ts->shell_pid);
	free(ts->buf1);
	free(ts->buf2);

	kill(ts->shell_pid, SIGKILL);

	wait4(ts->shell_pid, NULL, 0, NULL);

	close(ts->ptyfd);
	close(ts->sockfd);

	if(ts->ptyfd == maxfd || ts->sockfd == maxfd)
		maxfd--;
	if(ts->ptyfd == maxfd || ts->sockfd == maxfd)
		maxfd--;
	
	free(ts);
}

int main(int argc, char **argv)
{
	struct sockaddr_in sa;
	int master_fd;
	fd_set rdfdset, wrfdset;
	int selret;
	int on = 1;
	int portnbr = 23;
	int c, ii;
	int daemonize = 0;
#ifdef QNAP_SERVICE_BINDING
	int iRetDbg = 0;
	T_BIND_ADDR  tbaDaemon;
	char  szIpAddr[INET6_ADDRSTRLEN];
#endif
	char buf[32], loginIP[32] = "255.255.255.255";

	/* check if user supplied a port number */

	for (;;) {
		c = getopt( argc, argv, "p:l:hd");
		if (c == EOF) break;
		switch (c) {
			case 'p':
				portnbr = atoi(optarg);
				break;
			case 'l':
				loginpath = strdup (optarg);
				break;
			case 'd':
				daemonize = 1;
				break;
			case 'h': 
			default:
				show_usage();
				exit(1);
		}
	}

	if (!loginpath) {
// modify by Kent 2003/01/30
//	  login = "/bin/login";
	  loginpath = "/bin/naslogin";
// end
	  if (access(loginpath, X_OK) < 0)
	    loginpath = "/bin/sh";
	}
	  
	if (access(loginpath, X_OK) < 0) {
		error_msg_and_die ("'%s' unavailable.", loginpath);
	}

	printf("telnetd: starting\n");
	printf("  port: %i; login program: %s\n",portnbr,loginpath);

	argv_init[0] = loginpath;
	sessions = 0;

#ifdef QNAP_SERVICE_BINDING	//// Install signal handers.
	signal(SIGHUP, Signal_Handler_SigHup);
#endif
	/* Grab a TCP socket.  */

	master_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (master_fd < 0) {
		perror("socket");
		return 1;
	}
	(void)setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/* Set it to listen to specified port.  */

	memset((void *)&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(portnbr);

	if (bind(master_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(master_fd, 1) < 0) {
		perror("listen");
		return 1;
	}

	if (daemonize) 
	{
//		DEBUG_OUT("  daemonizing\n");
		if (daemon(0, 1) < 0) perror_msg_and_die("daemon");
	}

	
	maxfd = master_fd;

	do {
		struct tsession *ts;

		FD_ZERO(&rdfdset);
		FD_ZERO(&wrfdset);

		/* select on the master socket, all telnet sockets and their
		 * ptys if there is room in their respective session buffers.
		 */

		FD_SET(master_fd, &rdfdset);

		ts = sessions;
		while (ts) {
			/* buf1 is used from socket to pty
			 * buf2 is used from pty to socket
			 */
			if (ts->size1 > 0) {
				FD_SET(ts->ptyfd, &wrfdset);  /* can write to pty */
			}
			if (ts->size1 < BUFSIZE) {
				FD_SET(ts->sockfd, &rdfdset); /* can read from socket */
			}
			if (ts->size2 > 0) {
				FD_SET(ts->sockfd, &wrfdset); /* can write to socket */
			}
			if (ts->size2 < BUFSIZE) {
				FD_SET(ts->ptyfd, &rdfdset);  /* can read from pty */
			}
			ts = ts->next;
		}

		selret = select(maxfd + 1, &rdfdset, &wrfdset, 0, 0);

		if (!selret)
			break;

		/* First check for and accept new sessions.  */
		if (FD_ISSET(master_fd, &rdfdset)) {
			int fd, salen;

			salen = sizeof(sa);	
			if ((fd = accept(master_fd, (struct sockaddr *)&sa,
					 &salen)) < 0) {
				continue;
			} else {
#ifdef QNAP_SERVICE_BINDING		//added by Smith for service binding

				if (g_bHup)
				{
					iRetDbg = Get_Binding_Address(&tbaDaemon);
					g_bHup = FALSE;
				}


				// Check if the incomming interface matched the service binding settings.
				if (0 != (iRetDbg = Check_Binding_Address(fd, &tbaDaemon, szIpAddr)))
				{
					close(fd);
					continue;
				}
#endif
				/* Create a new session and link it into
				   our active list.  */
				struct tsession *new_ts = make_new_session(fd);
				if (new_ts) {
					new_ts->next = sessions;
					sessions = new_ts;
					if (fd > maxfd)
						maxfd = fd;
					sprintf(loginIP,"%s",inet_ntoa(sa.sin_addr));
					write_telnet_ip(new_ts->shell_pid, loginIP);
				} else {
					close(fd);
				}
			}
		}

		/* Then check for data tunneling.  */

		ts = sessions;
		while (ts) { /* For all sessions...  */
			int maxlen, w, r;
			struct tsession *next = ts->next; /* in case we free ts. */

			if (ts->size1 && FD_ISSET(ts->ptyfd, &wrfdset)) {
			    int processed, num_totty;
			    char *ptr;
				/* Write to pty from buffer 1.  */
				
				maxlen = MIN(BUFSIZE - ts->wridx1,
					     ts->size1);
				ptr = remove_iacs(ts->buf1 + ts->wridx1, maxlen, 
					&processed, &num_totty);
		
				/* the difference between processed and num_totty
				   is all the iacs we removed from the stream.
				   Adjust buf1 accordingly. */
				ts->wridx1 += processed - num_totty;
				ts->size1 -= processed - num_totty;

				w = write(ts->ptyfd, ptr, num_totty);
				if (w < 0) {
					perror("write");
					free_session(ts);
					ts = next;
					continue;
				}
				ts->wridx1 += w;
				ts->size1 -= w;
				if (ts->wridx1 == BUFSIZE)
					ts->wridx1 = 0;
			}

			if (ts->size2 && FD_ISSET(ts->sockfd, &wrfdset)) {
				/* Write to socket from buffer 2.  */
				maxlen = MIN(BUFSIZE - ts->wridx2,
					     ts->size2);
				w = write(ts->sockfd, ts->buf2 + ts->wridx2, maxlen);
				if (w < 0) {
					perror("write");
					free_session(ts);
					ts = next;
					continue;
				}
				ts->wridx2 += w;
				ts->size2 -= w;
				if (ts->wridx2 == BUFSIZE)
					ts->wridx2 = 0;
			}

			if (ts->size1 < BUFSIZE && FD_ISSET(ts->sockfd, &rdfdset)) {
				/* Read from socket to buffer 1. */
				maxlen = MIN(BUFSIZE - ts->rdidx1,
					     BUFSIZE - ts->size1);
				r = read(ts->sockfd, ts->buf1 + ts->rdidx1, maxlen);
				if (!r || (r < 0 && errno != EINTR)) {
					free_session(ts);
					ts = next;
					continue;
				}
				if(!*(ts->buf1 + ts->rdidx1 + r - 1)) {
					r--;
					if(!r)
						continue;
				}
				ts->rdidx1 += r;
				ts->size1 += r;
				if (ts->rdidx1 == BUFSIZE)
					ts->rdidx1 = 0;
			}

			if (ts->size2 < BUFSIZE && FD_ISSET(ts->ptyfd, &rdfdset)) {
				/* Read from pty to buffer 2.  */
				maxlen = MIN(BUFSIZE - ts->rdidx2,
					     BUFSIZE - ts->size2);
				r = read(ts->ptyfd, ts->buf2 + ts->rdidx2, maxlen);
				if (!r || (r < 0 && errno != EINTR)) {
					free_session(ts);
					ts = next;
					continue;
				}
				for (ii=0; ii < r; ii++)
				  if (*(ts->buf2 + ts->rdidx2 + ii) == 3)
				    fprintf(stderr, "found <CTRL>-<C> in data!\n");
				ts->rdidx2 += r;
				ts->size2 += r;
				if (ts->rdidx2 == BUFSIZE)
					ts->rdidx2 = 0;
			}

			if (ts->size1 == 0) {
				ts->rdidx1 = 0;
				ts->wridx1 = 0;
			}
			if (ts->size2 == 0) {
				ts->rdidx2 = 0;
				ts->wridx2 = 0;
			}
			ts = next;
		}

	} while (1);

	return 0;
}

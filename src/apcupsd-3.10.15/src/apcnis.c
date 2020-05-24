/*
 *  apcserver.c  -- Network server for apcupsd
 */
/*
   Copyright (C) 1999-2004 Kern Sibbald

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

 */


#include "apc.h"

#ifdef HAVE_NISSERVER

#ifdef HAVE_PTHREADS
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#else
#define P(x)
#define V(x)
#endif


static char largebuf[4096];
static int stat_recs;

struct s_arg {
   UPSINFO *ups;
   int newsockfd;
};


/* forward referenced subroutines */
void *handle_client_request(void *arg);

/*
 * This routine is called by the main process to
 * track its children. On CYGWIN, the child processes
 * don't always exit when the other end of the socket
 * hangs up. Thus they remain hung on a read(). After
 * 30 seconds, we send them a SIGTERM signal, which 
 * causes them to wake up to the reality of the situation.
 */
#ifndef HAVE_PTHREADS
static void reap_children(int childpid)
{
    static int pids[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static int times[10];
    int i;
    time_t now;
    int wpid;


    time(&now);
    for (i=0; i<10; i++) {
	if (pids[i]) {
	    wpid = waitpid(pids[i], NULL, WNOHANG);	       
	    if (wpid == -1 || wpid == pids[i]) {
		pids[i] = 0;	      /* Child gone, remove from table */
	    } else if (wpid == 0 && ((now - times[i]) > 30)) {
		kill(pids[i], SIGTERM);  /* still running, kill it */
	    }
	}
    }
    /* Make another pass reaping killed programs and inserting new child */
    for (i=0; i<10; i++) {
	if (pids[i]) {
	    wpid = waitpid(pids[i], NULL, WNOHANG);	       
	    if (wpid == -1 || wpid == pids[i]) {
		pids[i] = 0;	      /* Child gone, remove from table */
	     }
	}
	if (childpid && (pids[i] == 0)) {
	    pids[i] = childpid;
	    times[i] = now;
	    childpid = 0;
	}
    }
}
#endif /* HAVE_PTHREADS */

static void status_open(UPSINFO *ups)
{
    P(mutex);
    largebuf[0] = 0;
    stat_recs = 0;
}

#define STAT_REV 1

/*
 * Send the status lines across the network one line
 * at a time (to prevent sending too large a buffer).
 *
 * Returns -1 on error or EOF
 *	    0 OK
 */
static int status_close(UPSINFO *ups, int nsockfd) 
{
    int i;
    char buf[MAXSTRING];
    char *sptr, *eptr;

    i = strlen(largebuf);
    asnprintf(buf, sizeof(buf), "APC      : %03d,%03d,%04d\n", STAT_REV, 
	      stat_recs, i);
    if (net_send(nsockfd, buf, strlen(buf)) <= 0) {
	V(mutex);
	return -1;
    }
    sptr = eptr = largebuf;
    for ( ; i > 0; i--) {
        if (*eptr == '\n') {
	    eptr++;
	    if (net_send(nsockfd, sptr, eptr - sptr) <= 0)
	       break;
	    sptr = eptr;
	} else 
	    eptr++;
    }
    if (net_send(nsockfd, NULL, 0) < 0) {
	V(mutex);
	return -1;
    }
    V(mutex);
    return 0;
}



/********************************************************************* 
 * 
 * Buffer up the status messages so that they can be sent
 * by the status_close() routine over the network.
 */
static void status_write(UPSINFO *ups, char *fmt, ...)
{
    va_list ap;
    int i;
    char buf[MAXSTRING];

    va_start(ap, fmt);
    avsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if ((i=(strlen(largebuf) + strlen(buf))) < (int)(sizeof(largebuf)-1)) {
	strcat(largebuf, buf);
	stat_recs++;
    } else
	log_event(ups, LOG_ERR,
          "apcserver.c: Status buffer overflow %d bytes\n", i-sizeof(largebuf));
}


void do_server(UPSINFO *ups)
{
   int newsockfd, sockfd, childpid;
   struct sockaddr_in cli_addr;       /* client's address */
   struct sockaddr_in serv_addr;      /* our address */
   int tlog;
   int turnon = 1;
   struct s_arg *arg;
   struct in_addr local_ip;

   init_thread_signals();

   for (tlog=0; (ups=attach_ups(ups, SHM_RDONLY)) == NULL; tlog -= 5*60 ) {
      if (tlog <= 0) {
	 tlog = 60*60; 
         log_event(ups, LOG_ERR, "apcserver: Cannot attach SYSV IPC.\n");
      }
      sleep(5*60);
   }

   local_ip.s_addr = INADDR_ANY;

    /*
     * This ifdef is a bit of a kludge, but without it, we have no inet_pton,
     * so avoid the error message.
     */
#ifdef HAVE_NAMESER_H
   if (ups->nisip[0]) {
      if (inet_pton(AF_INET, ups->nisip, &local_ip) != 1) {
         log_event(ups, LOG_WARNING, "Invalid NISIP specified: '%s'", ups->nisip);
	 local_ip.s_addr = INADDR_ANY;
      }
   }
#endif

   /*
    * Open a TCP socket  
    */
   for (tlog=0; (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0; tlog -= 5*60 ) {
      if (tlog <= 0) {
	 tlog = 60*60; 
         log_event(ups, LOG_ERR,  "apcserver: cannot open stream socket");
      }
      sleep(5*60);
   }

   /*
    * Reuse old sockets 
    */
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &turnon, sizeof(turnon)) < 0) {
      log_event(ups, LOG_WARNING, "Cannot set SO_REUSEADDR on socket: %s\n" , strerror(errno));
   }
   /* 
    * Bind our local address so that the client can send to us.
    */
   memset((char *)&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr = local_ip;
   serv_addr.sin_port = htons(ups->statusport);

   for (tlog=0; bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0; tlog -= 5*60 ) {
      if (tlog <= 0) {
	 tlog = 60*60; 
         log_event(ups, LOG_ERR, "apcserver: cannot bind port %d. ERR=%s",
	    ups->statusport, strerror(errno));
      }
      sleep(5*60);
   }
   listen(sockfd, 5);		      /* tell system we are ready */

   log_event(ups, LOG_INFO, "NIS server startup succeeded");

   for (;;) {
      /* 
       * Wait for a connection from a client process.
       */
       for (tlog=0; (newsockfd = net_accept(sockfd, &cli_addr)) < 0; tlog -= 5*60 ) {
	  if (tlog <= 0) {
	     tlog = 60*60; 
             log_event(ups, LOG_ERR,  "apcserver: accept error. ERR=%s",
		strerror(errno));
	  }
	  sleep(5*60);
       }

#ifdef HAVE_LIBWRAP
       /*
        * This function checks the incoming client and if it's not
	* allowed closes the connection.
	*/
       if (check_wrappers(argvalue, newsockfd) == FAILURE) {
	   shutdown(newsockfd, 2);
	   close(newsockfd);
	   continue;
       }
#endif

       arg = (struct s_arg *)malloc(sizeof(struct s_arg));
       arg->newsockfd = newsockfd;
       arg->ups = ups;
       childpid = 0;

#ifdef HAVE_CYGWIN
       handle_client_request(arg);
       close(newsockfd);	      /* parent process */
#else

#ifdef HAVE_PTHREADS
       {
	  pthread_t tid;
	  pthread_create(&tid, NULL, handle_client_request, arg);
       }

#else 

       /* fork to provide the response */
       for (tlog=0; (childpid = fork()) < 0; tlog -= 5*60 ) {
	  if (tlog <= 0) {
	     tlog = 60*60; 
             log_event(ups, LOG_ERR, "apcserver: fork error. ERR=%s",
		strerror(errno));
	  }
	  sleep(5*60);
       } 
       if (childpid == 0) {	      /* child process */
	  close(sockfd);	      /* close original socket */
	  handle_client_request(arg); /* process the request */
	  shutdown(newsockfd, 2);
	  close(newsockfd);
	  exit(0);
       }
       close(newsockfd);	      /* parent process */
       reap_children(childpid);
#endif /* HAVE_PTHREADS */

#endif /* HAVE_CYGWIN */

    }
}   

/* 
 * Accept requests from client.  Send output one line
 * at a time followed by a zero length transmission.
 *
 * Return when the connection is terminated or there
 * is an error.
 */

void *handle_client_request(void *arg)
{
    FILE *events_file;
    char line[MAXSTRING];
    char errmsg[]   = "Invalid command\n";
    char notavail[] = "Not available\n";
    char notrun[]   = "Apcupsd not running\n";
    int nsockfd = ((struct s_arg *)arg)->newsockfd;
    UPSINFO *ups = ((struct s_arg *)arg)->ups;

#ifdef HAVE_PTHREADS
    pthread_detach(pthread_self());
#endif
    if ((ups=attach_ups(ups, SHM_RDONLY)) == NULL) {
	net_send(nsockfd, notrun, sizeof(notrun));
	net_send(nsockfd, NULL, 0);
	free(arg);
        Error_abort0("Cannot attach SYSV IPC.\n");
    }

    for (;;) {
	/* Read command */
	if (net_recv(nsockfd, line, MAXSTRING) <= 0) {
	    break;			 /* connection terminated */
	}

        if (strncmp("status", line, 6) == 0) {
	    if (output_status(ups, nsockfd, status_open, status_write, status_close) < 0) {
		break; 
	    }

        } else if (strncmp("events", line, 6) == 0) {
	    if ((ups->eventfile[0] == 0) ||
                 ((events_file = fopen(ups->eventfile, "r")) == NULL)) {
	       net_send(nsockfd, notavail, sizeof(notavail));	  
	       if (net_send(nsockfd, NULL, 0) < 0) {
		   break;
	       }
	   } else {
	       int stat = output_events(nsockfd, events_file);
	       fclose(events_file);
	       if (stat < 0) {
		   net_send(nsockfd, notavail, sizeof(notavail));
		   net_send(nsockfd, NULL, 0);
		   break;
	       }
	   }
	 
        } else if (strncmp("rawupsinfo", line, 10) == 0) {
	    net_send(nsockfd, (char *)ups, sizeof(UPSINFO));
	    if (net_send(nsockfd, NULL, 0) < 0) {
		break;
	    }
	
        } else if (strncmp("eprominfo", line, 9) == 0) {
	    int len;
	    len = strlen(ups->eprom) + 1;
	    net_send(nsockfd, ups->eprom, len);
	    len = strlen(ups->firmrev) + 1;
	    net_send(nsockfd, ups->firmrev, len);
	    len = strlen(ups->upsmodel) + 1;
	    net_send(nsockfd, ups->upsmodel, len);

	    if (net_send(nsockfd, NULL, 0) < 0) {
		break;
	    }

	} else {
	    net_send(nsockfd, errmsg, sizeof(errmsg));
	    if (net_send(nsockfd, NULL, 0) < 0) {
		break;
	    }
	}
    }
#ifdef HAVE_PTHREADS
    shutdown(nsockfd, 2);
    close(nsockfd);
#endif
    free(arg);
    detach_ups(ups);
    return NULL;
}

#else /* HAVE_NISSERVER */

void do_server(UPSINFO *ups) {
    log_event(ups, LOG_ERR, "apcserver: code not enabled in config.\n");
    exit(1);
}

#endif /* HAVE_NISSERVER */


#ifdef HAVE_LIBWRAP
/*
 * Unfortunately this function is also used by the old network code
 * so for now compile it in anyway.
 */
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;

int check_wrappers(char *av, int newsock)
{
    struct request_info req;
    char *av0;

    av0 = strrchr(av, '/');
    if (av0) {
	av0++;			      /* strip everything before and including / */
    } else {
	av0 = av;
    }

    request_init(&req, RQ_DAEMON, av0, RQ_FILE, newsock, NULL);
    fromhost(&req);
    if (!hosts_access(&req)) {
	log_event(core_ups, LOG_WARNING,
            _("Connection from %.500s refused by tcp_wrappers."),
	    eval_client(&req));
	return FAILURE;
    }
#ifdef I_WANT_LOTS_OF_LOGGING
    log_event(core_ups, LOG_NOTICE, "connect from %.500s", eval_client(&req));
#endif
    return SUCCESS;
}

#endif /* HAVE_LIBWRAP */

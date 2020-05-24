/* libhttpd.c - HTTP protocol library
**
** Copyright ?1995,1998,1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/


#include "config.h"
#include "version.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <stdarg.h>

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

extern char* crypt( const char* key, const char* setting );

#include "libhttpd.h"
#include "mmc.h"
#include "timers.h"
#include "match.h"
#include "tdate_parse.h"

/* NAS lib */
#include <NAS.h>
#include <config.h>
#include <userManager.h>
#include <naslog.h>
#include <debug.h>
#include "cfg_system.h"

#ifdef HTTP_LIVE_STREAMING
#include "nvr.h"
#include "nvr_spec.h"
#endif

#define STATISTICS	1

/* libms for Multimedia Station authentication */
#include "ms_auth.h"
#include "ms_perm.h"
#include "ms_util.h"

#ifdef _UIV2_
#include "auth_session.h"
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifdef HTTP_LIVE_STREAMING
#include "http_stream.h"
#endif

extern char* argv0;
/* Forwards. */
static void child_reaper( ClientData client_data, struct timeval* nowP );
static int do_reap( void );
static void check_options( void );
static void free_httpd_server( httpd_server* hs );
static int initialize_listen_socket( httpd_sockaddr* saP );
static void add_response( httpd_conn* hc, char* str );
static void clear_ndelay( httpd_conn* hc );
static void send_mime( httpd_conn* hc, int status, char* title, char* encodings, char* extraheads, char* type, long long length, time_t mod );
static void send_response( httpd_conn* hc, int status, char* title, char* extraheads, char* form, char* arg );
static void send_response_tail( httpd_conn* hc );
#ifdef ERR_DIR
static int send_err_file( httpd_conn* hc, int status, char* title, char* extraheads, char* filename , char *arg);
#endif /* ERR_DIR */
#ifdef AUTH_FILE
static void send_authenticate( httpd_conn* hc, char* realm );
static int b64_decode( const char* str, unsigned char* space, int size );
static int auth_check( httpd_conn* hc, char* dirname  );
#endif /* AUTH_FILE */
static void send_dirredirect( httpd_conn* hc );
static int hexit( char c );
static void strdecode( char* to, char* from );
#ifdef GENERATE_INDEXES
static void strencode( char* to, int tosize, char* from );
#endif /* GENERATE_INDEXES */
#ifdef TILDE_MAP_1
static int tilde_map_1( httpd_conn* hc );
#endif /* TILDE_MAP_1 */
#ifdef TILDE_MAP_2
static int tilde_map_2( httpd_conn* hc );
#endif /* TILDE_MAP_2 */
static int vhost_map( httpd_conn* hc );
static char* expand_symlinks( char* path, char** restP, int no_symlink, int tildemapped );
static char* bufgets( httpd_conn* hc );
static void de_dotdot( char* file );
static void figure_mime( httpd_conn* hc );
#ifdef CGI_TIMELIMIT
static void cgi_kill2( ClientData client_data, struct timeval* nowP );
static void cgi_kill( ClientData client_data, struct timeval* nowP );
#endif /* CGI_TIMELIMIT */
#ifdef GENERATE_INDEXES
static off_t ls( httpd_conn* hc );
#endif /* GENERATE_INDEXES */
static char* build_env( char* fmt, char* arg );
#ifdef SERVER_NAME_LIST
static char* hostname_map( char* hostname );
#endif /* SERVER_NAME_LIST */
static char** make_envp( httpd_conn* hc );
static char** make_argp( httpd_conn* hc );
static void cgi_interpose_input( httpd_conn* hc, int wfd );
static void cgi_interpose_output( httpd_conn* hc, int rfd );
static void cgi_child( httpd_conn* hc );
static off_t cgi( httpd_conn* hc );
static int really_start_request( httpd_conn* hc );
static int check_referer( httpd_conn* hc );
static int really_check_referer( httpd_conn* hc );
static int sockaddr_check( httpd_sockaddr* saP );
static size_t sockaddr_len( httpd_sockaddr* saP );
static int my_snprintf( char* str, size_t size, const char* format, ... );
//Richard Chang 20070821 add log
static void add_loginfail_log(httpd_conn* hc, char *mgs);
//end

static int reap_time;

int g_checkpassword;
void kentprintf(char *str);
void kentprint(int);

extern pwdcheck g_pwdcheck[100];
char remoteuser[256];
char remoteuserip[64];
int cnt_timeout=0;
char cgi_timeout[30][128];

int Exec;

#if defined(NVR_DEBUG)

#define MAX_DEB_LOG_NUM		128

#define ACT_GET_BEGIN		1
#define ACT_GET_BEGIN2		2
#define ACT_ACCEPT		3
#define ACT_CLOSE		4
#define ACT_WAKE		5
#define ACT_WAKE_STREAM		6
#define ACT_SEND_NORM		7
#define ACT_SEND_STREAM		8
#define ACT_SEND_PTZ		9

#include "time_com.h"
#include "log.h"

static struct deb_log_st
{
    TIMESTAMP_t ts;
    int act;
    void *hc;
    int fd;
    int tid;
} __deb_log[MAX_DEB_LOG_NUM];

static int __deb_idx=0;


void deb_log( int act, void *hc, int fd )
{
    __deb_log[__deb_idx].ts=current_time_stamp();
    __deb_log[__deb_idx].act=act;
    __deb_log[__deb_idx].hc=hc;
    __deb_log[__deb_idx].fd=fd;
    __deb_log[__deb_idx].tid=vos_gettid();
    __deb_idx=(__deb_idx+MAX_DEB_LOG_NUM+1)%MAX_DEB_LOG_NUM;
}

void log_wake_up( void *hc, int fd, int is_stream )
{
    deb_log( is_stream?ACT_WAKE_STREAM:ACT_WAKE, hc, fd );    
}

void log_set_send( void *hc, int fd, int type )
{
    deb_log( type+ACT_SEND_NORM, hc, fd );    
}

static void dump_log(  FILE *file, struct deb_log_st *p, int count )
{
    char tstr[32];
    static char *actstr[]= { "get_begin", "get_begin2", "accept", "close", "wake", "wake_stream", "send", "send_stream", "send_ptz", "BUG", "" };
    int max_id=(sizeof(actstr)/sizeof(char*))-2;
    if(p->act<0 || p->act>max_id )p->act=max_id+1;
    make_time_string2( tstr, p->ts );
    if(file)
    {
        fprintf( file, "%4d %s httpd_conn_%s hc=%08X, fd=%d,tid=%d\n", count, tstr, actstr[p->act-1], p->hc, p->fd, p->tid);
    }
    else
    {
        log_Debug( "%4d %s httpd_conn_%s hc=%08X, fd=%d, tid=%d", count, tstr, actstr[p->act-1], p->hc, p->fd, p->tid);
    }
}

static void do_deb_dump( FILE *file )
{
    int count=0;
    int i, n=(__deb_idx+MAX_DEB_LOG_NUM)%MAX_DEB_LOG_NUM;
    for( i=n; i<MAX_DEB_LOG_NUM; i++ )
    {
        if(__deb_log[i].act!=0 )
            dump_log( file, &__deb_log[i], count++);
    }
    for( i=0; i<n; i++ )
    {
        dump_log( file, &__deb_log[i], count++);
    }
}

void deb_dump( void )
{
    do_deb_dump( NULL );
}

void dbg_file_dump(FILE * file)
{
    do_deb_dump( file );
}
#endif

#ifdef	QNAP_SERVICE_BINDING

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

// A flag indicating whether if receiving a HUP signal.
int  g_bHup = TRUE;

// The global buffer to cache service binding address. 
T_BIND_ADDR g_tBindAddr;

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
	if (NULL == (pfAddr = popen(SZP_GET_BIND_ADDR " WEBMAN 2>/dev/null", "r")))  return 0;
	while (NULL != fgets(szLine, sizeof(szLine), pfAddr))
	{
		szLine[sizeof(szLine)-1] = '\0';
		if (0 >= (cbAddr = strlen(szLine)))  continue;
		if ('\n' == szLine[cbAddr-1])  szLine[cbAddr-1] = '\0';
		if (NULL != strchr(szLine, ':'))
		{
			//DEBUG_PRINTF("Binding IPv6 address: %s\n", szLine);
			strcpy(ptBindAddr->ryszIpv6[ptBindAddr->cnIpv6], szLine);
			ptBindAddr->cnIpv6 ++;
		}
		else if (NULL != strchr(szLine, '.'))
		{
			if (strstr(SZ_INADDR_ANY, szLine))
			{
				//DEBUG_PRINTF("Binding IPv4 address: %s ==> Service binding is disabled!\n", szLine);
				ptBindAddr->bEnabled = FALSE;
				ptBindAddr->cnIpv4 = ptBindAddr->cnIpv6 = 0;
				break;
			}
			//DEBUG_PRINTF("Binding IPv4 address: %s\n", szLine);
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
		if(0 == pszAddrBuf[0])
		{
			if (-1 == getsockname(fdAccept, (struct sockaddr *)&tsaLocal, &cbSktAddr))
			{
				//ERROR_PRINTF("Can't get the socket name of an accepted socket!\n");
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
		else
		{
			if ((NULL != strchr(pszAddrBuf, ':')) && (0 < ptBindAddr->cnIpv6))
			{
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
			else if ((NULL != strchr(pszAddrBuf, '.')) && (0 < ptBindAddr->cnIpv4))
			{
				for (idx=0; (idx<ptBindAddr->cnIpv4) && strcmp(pszAddr, ptBindAddr->ryszIpv4[idx]); idx++);
				iRet = (idx >= ptBindAddr->cnIpv4) ? -ENOTTY : 0;
			}
		}
	}
exit_check_bind_addr:
	return iRet;
}
#endif	//QNAP_SERVICE_BINDING

//Richard Chang 20070910 add onlineuser
#ifdef STATISTICS
void add_administration_user(char* name, char* ip)
{
	USER_INFO user;
	
	strcpy(user.username, name);
	strcpy(user.ip, ip);
	strcpy(user.sharename, "Administration");
	user.app_id=shm_get_app_id("thttpd");
	user.schedule=SCHEDULE_TIMEOUT;
	user.pid=0;
	shm_add_user(&user);
}
#endif

static void
child_reaper( ClientData client_data, struct timeval* nowP )
    {
    int child_count;
    static int prev_child_count = 0;

    child_count = do_reap();

    /* Reschedule reaping, with adaptively changed time. */
    if ( child_count > prev_child_count * 3 / 2 )
	reap_time = max( reap_time / 2, MIN_REAP_TIME );
    else if ( child_count < prev_child_count * 2 / 3 )
	reap_time = min( reap_time * 5 / 4, MAX_REAP_TIME );
    (void) tmr_create(
	nowP, child_reaper, (ClientData) 0, reap_time * 1000L, 0 );
    }

static int
do_reap( void )
    {
    int child_count;
    pid_t pid;
    int status;

    /* Reap defunct children until there aren't any more. */
    for ( child_count = 0; ; ++child_count )
	{
#ifdef HAVE_WAITPID
	pid = waitpid( (pid_t) -1, &status, WNOHANG );
#else /* HAVE_WAITPID */
	pid = wait3( &status, WNOHANG, (struct rusage*) 0 );
#endif /* HAVE_WAITPID */
	if ( (int) pid == 0 )           /* none left */
	    break;
	if ( (int) pid < 0 )
	    {
	    if ( errno == EINTR )       /* because of ptrace */
		continue;
	    /* ECHILD shouldn't happen with the WNOHANG option, but with
	    ** some kernels it does anyway.  Ignore it.
	    */
	    if ( errno != ECHILD )
		syslog( LOG_ERR, "waitpid - %m" );
	    break;
	    }
	}
    return child_count;
    }


static void
check_options( void )
    {
#if defined(TILDE_MAP_1) && defined(TILDE_MAP_2)
    syslog( LOG_CRIT, "both TILDE_MAP_1 and TILDE_MAP_2 are defined" );
    exit( 1 );
#endif /* both */
    }


static void
free_httpd_server( httpd_server* hs )
    {
    if ( hs->binding_hostname != (char*) 0 )
	free( (void*) hs->binding_hostname );
    if ( hs->cwd != (char*) 0 )
	free( (void*) hs->cwd );
    if ( hs->cgi_pattern != (char*) 0 )
	free( (void*) hs->cgi_pattern );
    if ( hs->charset != (char*) 0 )
	free( (void*) hs->charset );
    if ( hs->url_pattern != (char*) 0 )
	free( (void*) hs->url_pattern );
    if ( hs->local_pattern != (char*) 0 )
	free( (void*) hs->local_pattern );
    free( (void*) hs );
    }


httpd_server*
httpd_initialize(
    char* hostname, httpd_sockaddr* sa4P, httpd_sockaddr* sa6P, int port,
    char* cgi_pattern, char* charset, char* cwd, FILE* logfp, int no_symlink,
    int vhost, char* url_pattern, char* local_pattern, int no_empty_referers )
    {
    httpd_server* hs;
    static char ghnbuf[256];
    char* cp;

    check_options();

    /* Set up child-process reaper. */
    reap_time = min( MIN_REAP_TIME * 4, MAX_REAP_TIME );
    (void) tmr_create(
	(struct timeval*) 0, child_reaper, (ClientData) 0, reap_time * 1000L,
	0 );

    hs = NEW( httpd_server, 1 );
    if ( hs == (httpd_server*) 0 )
	{
	syslog( LOG_CRIT, "out of memory" );
	return (httpd_server*) 0;
	}

    if ( hostname != (char*) 0 )
	{
	hs->binding_hostname = strdup( hostname );
	if ( hs->binding_hostname == (char*) 0 )
	    {
	    syslog( LOG_CRIT, "out of memory" );
	    return (httpd_server*) 0;
	    }
	hs->server_hostname = hs->binding_hostname;
	}
    else
	{
	hs->binding_hostname = (char*) 0;
	hs->server_hostname = (char*) 0;
	if ( gethostname( ghnbuf, sizeof(ghnbuf) ) < 0 )
	    ghnbuf[0] = '\0';
#ifdef SERVER_NAME_LIST
	if ( ghnbuf[0] != '\0' )
	    hs->server_hostname = hostname_map( ghnbuf );
#endif /* SERVER_NAME_LIST */
	if ( hs->server_hostname == (char*) 0 )
	    {
#ifdef SERVER_NAME
	    hs->server_hostname = SERVER_NAME;
#else /* SERVER_NAME */
	    if ( ghnbuf[0] != '\0' )
		hs->server_hostname = ghnbuf;
#endif /* SERVER_NAME */
	    }
	}

    hs->port = port;
    if ( cgi_pattern == (char*) 0 )
	hs->cgi_pattern = (char*) 0;
    else
	{
	/* Nuke any leading slashes. */
	if ( cgi_pattern[0] == '/' )
	    ++cgi_pattern;
	hs->cgi_pattern = strdup( cgi_pattern );
	if ( hs->cgi_pattern == (char*) 0 )
	    {
	    syslog( LOG_CRIT, "out of memory" );
	    return (httpd_server*) 0;
	    }
	/* Nuke any leading slashes in the cgi pattern. */
	while ( ( cp = strstr( hs->cgi_pattern, "|/" ) ) != (char*) 0 )
	    (void) strcpy( cp + 1, cp + 2 );
	}
    hs->charset = strdup( charset );
    hs->cwd = strdup( cwd );
    if ( hs->cwd == (char*) 0 )
	{
	syslog( LOG_CRIT, "out of memory" );
	return (httpd_server*) 0;
	}
    if ( url_pattern == (char*) 0 )
	hs->url_pattern = (char*) 0;
    else
	{
	hs->url_pattern = strdup( url_pattern );
	if ( hs->url_pattern == (char*) 0 )
	    {
	    syslog( LOG_CRIT, "out of memory" );
	    return (httpd_server*) 0;
	    }
	}
    if ( local_pattern == (char*) 0 )
	hs->local_pattern = (char*) 0;
    else
	{
	hs->local_pattern = strdup( local_pattern );
	if ( hs->local_pattern == (char*) 0 )
	    {
	    syslog( LOG_CRIT, "out of memory" );
	    return (httpd_server*) 0;
	    }
	}
    hs->logfp = (FILE*) 0;
    httpd_set_logfp( hs, logfp );
    hs->no_symlink = no_symlink;
    hs->vhost = vhost;
    hs->no_empty_referers = no_empty_referers;

    /* Initialize listen sockets.  Try v6 first because of a Linux peculiarity;
    ** unlike other systems, it has magical v6 sockets that also listen for v4,
    ** but if you bind a v4 socket first then the v6 bind fails.
    */
    if ( sa6P == (httpd_sockaddr*) 0 )
	hs->listen6_fd = -1;
    else
	hs->listen6_fd = initialize_listen_socket( sa6P );
    if ( sa4P == (httpd_sockaddr*) 0 )
	hs->listen4_fd = -1;
    else
	hs->listen4_fd = initialize_listen_socket( sa4P );
    /* If we didn't get any valid sockets, fail. */
    if ( hs->listen4_fd == -1 && hs->listen6_fd == -1 )
	{
	free_httpd_server( hs );
	return (httpd_server*) 0;
	}

    /* Done initializing. */
    if ( hs->binding_hostname == (char*) 0 )
	syslog( LOG_INFO, "%s starting on port %d", SERVER_SOFTWARE, hs->port );
    else
	syslog(
	    LOG_INFO, "%s starting on %.80s, port %d", SERVER_SOFTWARE,
	    httpd_ntoa( hs->listen4_fd != -1 ? sa4P : sa6P ), hs->port );
    return hs;
    }


static int
initialize_listen_socket( httpd_sockaddr* saP )
    {
    int listen_fd;
    int on, flags;

    /* Check sockaddr. */
    if ( ! sockaddr_check( saP ) )
	{
	syslog( LOG_CRIT, "unknown sockaddr family on listen socket" );
	return -1;
	}
    /* Create socket. */
    listen_fd = socket( saP->sa.sa_family, SOCK_STREAM, 0 );
    if ( listen_fd < 0 )
	{
	syslog( LOG_CRIT, "socket - %m" );
	return -1;
	}
    (void) fcntl( listen_fd, F_SETFD, 1 );
    /* Allow reuse of local addresses. */
    on = 1;
    if ( setsockopt(
	     listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on,
	     sizeof(on) ) < 0 )
	syslog( LOG_CRIT, "setsockopt SO_REUSEADDR - %m" );
    /* Bind to it. */
    if ( bind( listen_fd, &saP->sa, sockaddr_len( saP ) ) < 0 )
	{
	syslog(
	    LOG_CRIT, "bind %.80s - %m", httpd_ntoa( saP ) );
	(void) close( listen_fd );
	return -1;
	}
    /* Set the listen file descriptor to no-delay mode. */
    flags = fcntl( listen_fd, F_GETFL, 0 );
    if ( flags == -1 )
	{
	syslog( LOG_CRIT, "fcntl F_GETFL - %m" );
	(void) close( listen_fd );
	return -1;
	}
    if ( fcntl( listen_fd, F_SETFL, flags | O_NDELAY ) < 0 )
	{
	syslog( LOG_CRIT, "fcntl O_NDELAY - %m" );
	(void) close( listen_fd );
	return -1;
	}
    /* Start a listen going. */
    if ( listen( listen_fd, LISTEN_BACKLOG ) < 0 )
	{
	syslog( LOG_CRIT, "listen - %m" );
	(void) close( listen_fd );
	return -1;
	}
    return listen_fd;
    }


void
httpd_set_logfp( httpd_server* hs, FILE* logfp )
    {
    if ( hs->logfp != (FILE*) 0 )
	(void) fclose( hs->logfp );
    hs->logfp = logfp;
    }


void
httpd_terminate( httpd_server* hs )
    {
    if ( hs->listen4_fd != -1 )
	(void) close( hs->listen4_fd );
    if ( hs->listen6_fd != -1 )
	(void) close( hs->listen6_fd );
    if ( hs->logfp != (FILE*) 0 )
	(void) fclose( hs->logfp );
    free_httpd_server( hs );
    }


/* Conditional macro to allow two alternate forms for use in the built-in
** error pages.  If EXPLICIT_ERROR_PAGES is defined, the second and more
** explicit error form is used; otherwise, the first and more generic
** form is used.
*/
#ifdef EXPLICIT_ERROR_PAGES
#define ERROR_FORM(a,b) b
#else /* EXPLICIT_ERROR_PAGES */
#define ERROR_FORM(a,b) a
#endif /* EXPLICIT_ERROR_PAGES */


static char* ok200title = "OK";
static char* ok206title = "Partial Content";

static char* err302title = "Found";
static char* err302form = "The actual URL is '%.80s'.\n";

static char* err304title = "Not Modified";

char* httpd_err400title = "Bad Request";
char* httpd_err400form =
    "Your request has bad syntax or is inherently impossible to satisfy.\n";

#ifdef AUTH_FILE
static char* err401title = "Unauthorized";
static char* err401form =
//    "Authorization required for the URL '%.80s'.\n";
//	"Authorization fail.\n";
	"<SCRIPT Language=\"JavaScript\">document.write(ERR401_FORM)</script>";
#endif /* AUTH_FILE */

static char* err403title = "Forbidden";
static char* err403form =
    "You do not have permission to get URL %.80s from this server.\n";

static char* err404title = "Not Found";
//static char* err404form =
//    "The requested URL '%.80s' was not found on this server.\n";
//Weak point fix: modified by KenChen@QNAP 
static char* err404form =
    "The requested resource %.80s was not found on this server.\n";

char* httpd_err408title = "Request Timeout";
char* httpd_err408form =
    "No request appeared within a reasonable time period.\n";

static char* err500title = "Internal Error";
static char* err500form =
    "There was an unusual problem serving the requested URL '%.80s'.\n";

static char* err501title = "Not Implemented";
static char* err501form =
    "The requested method '%.80s' is not implemented by this server.\n";

char* httpd_err503title = "Service Temporarily Overloaded";
char* httpd_err503form =
    "The requested URL '%.80s' is temporarily overloaded.  Please try again later.\n";

char* httpd_err504title = "Execute Error";
char* httpd_err504form = "You can't execute file %s\n";

char* httpd_err999title = "Web Filemanager";
//char* httpd_err999form = "Web filemanager is not enable.\n";
char* httpd_err999form = "<SCRIPT Language=\"JavaScript\">document.write(ERR999_FORM)</script>";

/* Append a string to the buffer waiting to be sent as response. */
static void
add_response( httpd_conn* hc, char* str )
    {
    int len;

    len = strlen( str );
    httpd_realloc_str( &hc->response, &hc->maxresponse, hc->responselen + len );
    (void) memcpy( &(hc->response[hc->responselen]), str, len );
    hc->responselen += len;
    }

/* Send the buffered response. */
void
httpd_write_response( httpd_conn* hc )
    {
    /* First turn off NDELAY mode. */
    clear_ndelay( hc );
    /* And send it, if necessary. */
    if ( hc->responselen > 0 )
	{
	(void) write( hc->conn_fd, hc->response, hc->responselen );
	hc->responselen = 0;
	}
    }

// add by Kent 2003/02/07
/* Set NDELAY mode on a socket. */
void
httpd_set_ndelay( int fd )
    {
    int flags, newflags;

    flags = fcntl( fd, F_GETFL, 0 );
    if ( flags != -1 )
        {
        newflags = flags | (int) O_NDELAY;
        if ( newflags != flags )
            (void) fcntl( fd, F_SETFL, newflags );
        }
    }
/* Clear NDELAY mode on a socket. */
void
httpd_clear_ndelay( int fd )
    {
    int flags, newflags;

    flags = fcntl( fd, F_GETFL, 0 );
    if ( flags != -1 )
        {
        newflags = flags & ~ (int) O_NDELAY;
        if ( newflags != flags )
            (void) fcntl( fd, F_SETFL, newflags );
        }
    }
// end

static void
clear_ndelay( httpd_conn* hc )
    {
    int flags, newflags;

    flags = fcntl( hc->conn_fd, F_GETFL, 0 );
    if ( flags != -1 )
	{
	newflags = flags & ~ (int) O_NDELAY;
	if ( newflags != flags )
	    (void) fcntl( hc->conn_fd, F_SETFL, newflags );
	}
    }


static void
send_mime( httpd_conn* hc, int status, char* title, char* encodings, char* extraheads, char* type, long long length, time_t mod )
    {
    time_t now;
    const char* rfc1123fmt = "%a, %d %b %Y %H:%M:%S GMT";
    char nowbuf[100];
    char modbuf[100];
    char fixed_type[500];
    char buf[1000];
    int partial_content;

    hc->status = status;
    hc->bytes = length;
    if ( hc->mime_flag )
	{
	if ( status == 200 && hc->got_range &&
	     ( hc->end_byte_loc >= hc->init_byte_loc ) &&
	     ( ( hc->end_byte_loc != length - 1 ) ||
	       ( hc->init_byte_loc != 0 ) ) &&
	     ( hc->range_if == (time_t) -1 ||
	       hc->range_if == hc->sb.st_mtime ) )
	    {
	    partial_content = 1;
	    hc->status = status = 206;
	    title = ok206title;
	    }
	else
	    partial_content = 0;

	now = time( (time_t*) 0 );
	if ( mod == (time_t) 0 )
	    mod = now;
	(void) strftime( nowbuf, sizeof(nowbuf), rfc1123fmt, gmtime( &now ) );
	(void) strftime( modbuf, sizeof(modbuf), rfc1123fmt, gmtime( &mod ) );
//=======================================================================
//	add by Kent Tsai by 2001/03/19
//	this section is control the charset
//=======================================================================
//	if (strstr(hc->encodedurl, "indexcht.html"))

//	if (strstr(hc->encodedurl, "/cgi-bin/data_cht/"))
//		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "big5");
//	else
//	if (strstr(hc->encodedurl, "&lang=cht"))
/* Marked by KenChen 20061114
	if (strstr(hc->cookie, "nas_lang=TCH"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "big5");
	else
	if (strstr(hc->cookie, "nas_lang=JPN"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "Shift_JIS");
	else
	if (strstr(hc->cookie, "nas_lang=ENG"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "iso8859-1");
	else
	if (strstr(hc->cookie, "nas_lang=SCH"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "gb2312");
	else
	if (strstr(hc->cookie, "nas_lang=KOR"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "KS_C_5601-1987");
	else
	if (strstr(hc->cookie, "nas_lang=RUS"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "windows-1251");
	else
        if (	strstr(hc->cookie, "nas_lang=FRE") || strstr(hc->cookie, "nas_lang=GER") ||
		strstr(hc->cookie, "nas_lang=SPA") || strstr(hc->cookie, "nas_lang=ITA") ||
		strstr(hc->cookie, "nas_lang=POR"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "windows-1252");
	else
	(void) my_snprintf(
	    fixed_type, sizeof(fixed_type), type, hc->hs->charset );
*/
//---Added by KenChen 20061114---//
	if (strstr(hc->cookie, "nas_lang"))
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, "UTF-8");
	else
		(void) my_snprintf(fixed_type, sizeof(fixed_type), type, hc->hs->charset );
//---end here---//
	if (hc->encodedurl)
	{
		if (strstr(hc->encodedurl, "/cgi-bin/Qdownload/"))//Jeffrey added 2005,11,30
			(void) my_snprintf( fixed_type, sizeof(fixed_type), type, "UTF-8" );

	}

//	(void) my_snprintf( fixed_type, sizeof(fixed_type), type, "UTF-8" );//Shone modified 2005,09,22

	(void) my_snprintf( buf, sizeof(buf),
	    "%.20s %d %s\r\nServer: %s\r\nContent-type: %s\r\nDate: %s\r\nLast-modified: %s\r\nAccept-Ranges: bytes\r\nConnection: close\r\n",
	    hc->protocol, status, title, SERVER_SOFTWARE, fixed_type, nowbuf,
	    modbuf );

	add_response( hc, buf );
	if ( encodings[0] != '\0' )
	    {
	    (void) my_snprintf( buf, sizeof(buf),
		"Content-encoding: %s\r\n", encodings );
	    add_response( hc, buf );
	    }
	if ( partial_content )
	    {
	    (void) my_snprintf( buf, sizeof(buf),
		"Content-range: bytes %lld-%lld/%lld\r\nContent-length: %lld\r\n",
		hc->init_byte_loc, hc->end_byte_loc, length, ( hc->end_byte_loc - hc->init_byte_loc + 1 ) );
	    add_response( hc, buf );
	    }
	else if ( length >= 0 )
	    {
	    (void) my_snprintf( buf, sizeof(buf),
		"Content-length: %lld\r\n", length );
	    add_response( hc, buf );
	    }
	if ( extraheads[0] != '\0' )
	    add_response( hc, extraheads );
	add_response( hc, "\r\n" );
	}
    }


void
httpd_realloc_str( char** strP, int* maxsizeP, int size )
    {
    if ( *maxsizeP == 0 )
	{
	*maxsizeP = MAX( 200, size );   /* arbitrary */
	*strP = NEW( char, *maxsizeP + 1 );
	}
    else if ( size > *maxsizeP )
	{
	*maxsizeP = MAX( *maxsizeP * 2, size * 5 / 4 );
	*strP = RENEW( *strP, char, *maxsizeP + 1 );
	}
    else
	return;
    if ( *strP == (char*) 0 )
	{
	syslog( LOG_ERR, "out of memory" );
	exit( 1 );
	}
    }

//====================================================
//	lock_pwd : when you logout you will lock your pwd to fail
//====================================================
void lock_pwd(char *username)
{
	int i;

#ifdef _DEBUG_LOCK
	FILE *fptr;
	fptr=fopen("/var/debug.lock", "a+");
	fprintf(fptr, "lock user->%s:\n", username);
	fclose(fptr);
#endif
	for (i=0;i<MAX_CONNECT;i++)
	{
		if (!strcmp(g_pwdcheck[i].name, username))
		{
			g_pwdcheck[i].bUsed=1;
			g_pwdcheck[i].bCheck=1;
			return;
		}
	}

	for (i=0;i<MAX_CONNECT;i++)
	{
		if (g_pwdcheck[i].bUsed==0)
		{
			strcpy(g_pwdcheck[i].name, username);
			g_pwdcheck[i].bUsed=1;
			g_pwdcheck[i].bCheck=1;
			return;
		}
	}
}

int get_lock_pwd(char *username)
{
	int i;

#ifdef _DEBUG_LOCK
	FILE *fptr;
	fptr=fopen("/var/debug.lock", "a+");
	fprintf(fptr, "getlock->%s:\n", username);
	fclose(fptr);
#endif
	for (i=0;i<MAX_CONNECT;i++)
	{
		if (!strcmp(username, g_pwdcheck[i].name))
		{
			return g_pwdcheck[i].bCheck;
		}
	}
	return 0;
}

void unlock_pwd(char *username)
{
	int i;

#ifdef _DEBUG_LOCK
	FILE *fptr;
	fptr=fopen("/var/debug.lock", "a+");
	fprintf(fptr, "unlock->%s:\n", username);
	fclose(fptr);
#endif
	for (i=0;i<MAX_CONNECT;i++)
	{
		if (!strcmp(g_pwdcheck[i].name, username))
		{
			g_pwdcheck[i].bUsed=0;
			g_pwdcheck[i].bCheck=0;
		}
	}
}

static void
send_response( httpd_conn* hc, int status, char* title, char* extraheads, char* form, char* arg )
    {
    char buf[1000];
    int i;

hc->response[0]=0x0;
    send_mime( hc, status, title, "", extraheads, "text/html", -1, 0 );
    (void) my_snprintf( buf, sizeof(buf),
	"<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H2>%d %s</H2>\n",
	status, title, status, title );
    add_response( hc, buf );
    (void) my_snprintf( buf, sizeof(buf),
	form, arg );
if (status==401)
{
	unlock_pwd(remoteuser);
//	g_checkpassword=0;
}
    add_response( hc, buf );
    if ( match( "**MSIE**", hc->useragent ) )
	{
	int n;
	add_response( hc, "<!--\n" );
	for ( n = 0; n < 6; ++n )
	    add_response( hc, "Padding so that MSIE deigns to show this error instead of its own canned one.\n");
	add_response( hc, "-->\n" );
	}
    send_response_tail( hc );
    }


static void
send_response_tail( httpd_conn* hc )
    {
    char buf[1000];

//============================================
//	mark by Kent Tsai 2001/03/05
//	bacause we don't need to show thttpd tail
//============================================
/*
    (void) my_snprintf( buf, sizeof(buf),
	"<HR>\n<ADDRESS><A HREF=\"%s\">%s</A></ADDRESS>\n</BODY></HTML>\n",
	SERVER_ADDRESS, SERVER_SOFTWARE );
    add_response( hc, buf );
*/
    }


void
httpd_send_err( httpd_conn* hc, int status, char* title, char* extraheads, char* form, char* arg )
    {
#ifdef ERR_DIR

    char filename[1000], errhtml[100];

/*        if (strstr(hc->cookie, "nas_lang=ENG"))
                strcpy(errhtml, "error_eng.html");
        else
        if (strstr(hc->cookie, "nas_lang=JPN"))
                strcpy(errhtml, "error_jpn.html");
        else
        if (strstr(hc->cookie, "nas_lang=TCH"))
                strcpy(errhtml, "error_cht.html");
        else
                strcpy(errhtml, "error_eng.html");*/
	strcpy(errhtml, "error_thttpd.html");

    /* Try virtual host error page. */
    if ( hc->hs->vhost && hc->hostdir[0] != '\0' )
	{
//	(void) my_snprintf( filename, sizeof(filename),
//	    "%s/%s/err%d.html", hc->hostdir, ERR_DIR, status );

	//(void) my_snprintf(filename, sizeof(filename), "%s/%s/%s", hc->hostdir, ERR_DIR, errhtml);
	(void) my_snprintf(filename, sizeof(filename), "/tmp/%s", errhtml); //use /tmp/error_thttpd.html instead, 2012/Jun/13
	if ( send_err_file( hc, status, title, extraheads, filename , arg) )
	    return;
	}

    /* Try server-wide error page. */
//    (void) my_snprintf( filename, sizeof(filename),
//	"%s/err%d.html", ERR_DIR, status );
	//(void) my_snprintf( filename, sizeof(filename), "%s/%s", ERR_DIR, errhtml);
	(void) my_snprintf( filename, sizeof(filename), "/tmp/%s", errhtml); //use /tmp/error_thttpd.html instead, 2012/Jun/13

//netscape error(URL include space char)
    if ( send_err_file( hc, status, title, extraheads, filename, arg ) )
	return;

    /* Fall back on built-in error page. */
    send_response( hc, status, title, extraheads, form, arg );

#else /* ERR_DIR */

    send_response( hc, status, title, extraheads, form, arg );
#endif /* ERR_DIR */
    }



//====================================================================
//	this function modify by KEnt Tsai by 2001/03/07
//	use error.html to show error html
//===================================================================
#ifdef ERR_DIR
static int
send_err_file( httpd_conn* hc, int status, char* title, char* extraheads, char* filename, char *arg)
    {
    FILE* fp;
    char buf[1000], *ptr;
    int r;
    int i;
	char    color_101r_1[]={"<td width=\"97%\" bgcolor=\"#678DBF\">"};
	char    color_101r_2[]={"<td bgcolor=\"#678DBF\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
	char    color_101rw_1[]={"<td width=\"97%\" bgcolor=\"#678DBF\">"};
	char    color_101rw_2[]={"<td bgcolor=\"#678DBF\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
	char	color_2108r_1[]={"<td width=\"97%\" bgcolor=\"#764C90\">"};
	char	color_2108r_2[]={"<td bgcolor=\"#764C90\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
	char    color_2108rw_1[]={"<td width=\"97%\" bgcolor=\"#764C90\">"};
	char    color_2108rw_2[]={"<td bgcolor=\"#764C90\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
        char    color_104r_1[]={"<td width=\"97%\" bgcolor=\"#0095C7\">"};
        char    color_104r_2[]={"<td bgcolor=\"#0095C7\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
        char    color_4000_1[]={"<td width=\"97%\" bgcolor=\"#6666FF\">"};
        char    color_4000_2[]={"<td bgcolor=\"#6666FF\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
        char    color_4020_1[]={"<td width=\"97%\" bgcolor=\"#009889\">"};
        char    color_4020_2[]={"<td bgcolor=\"#009889\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
        char    color_2000_1[]={"<td width=\"97%\" bgcolor=\"#004682\">"};
        char    color_2000_2[]={"<td bgcolor=\"#004682\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};
	char	color_4100_1[]={"<td width=\"97%\" bgcolor=\"#8D4541\">"};
	char	color_4100_2[]={"<td bgcolor=\"#8D4541\" height=\"77\" valign=\"bottom\"><img src=\"/v2_menu/images/b4a.gif\"></td>"};


    fp = fopen( filename, "r" );
    if ( fp == (FILE*) 0 )
	return 0;
    send_mime( hc, status, title, "", extraheads, "text/html", -1, 0 );
    for (;;)
	{
	buf[0]=0x0;
	fgets(buf, 1000, fp);
	if (buf[0]==0x0)
		break;
//	r = fread( buf, 1, sizeof(buf) - 1, fp );
//	if ( r <= 0 )
//	    break;
//	buf[r] = '\0';
	//==============================================
#ifdef _NAS
        if (strstr(buf, "HEADER_BG1"))
        {
                char    model[256];

                Get_Model_Name(model, 256);

                if (!strcmp(model, "NAS-101R"))
                        ptr=color_101r_1;
                else
                if (!strcmp(model, "NAS-101RW"))
                        ptr=color_101rw_1;
                else
                if (!strcmp(model, "NAS-2108R"))
                        ptr=color_2108r_1;
                else
		if (!strcmp(model, "NAS-2108RW"))
			ptr=color_2108rw_1;
		else
                if (!strcmp(model, "NAS-104R") || !strcmp(model, "NAS-4300"))
                        ptr=color_104r_1;
                else
                if (!strcmp(model, "NAS-4000"))
                        ptr=color_4000_1;
		else
		if (!strcmp(model, "NAS-4020") || !strcmp(model, "NAS-4030"))
			ptr=color_4020_1;
		else
		if (!strcmp(model, "NAS-2000") || !strcmp(model, "ND-22000"))
			ptr=color_2000_1;
		else
		if (!strcmp(model, "NAS-4100") || !strcmp(model, "ND-41000") || !strcmp(model, "NAS-4120"))
			ptr=color_4100_1;
                else
                        ptr=color_4000_1;
		my_snprintf(buf, sizeof(buf), ptr, arg);
        }
        else
        if (strstr(buf, "HEADER_BG2"))
        {
                char    model[256];

                Get_Model_Name(model, 256);
                if (!strcmp(model, "NAS-101R"))
                        ptr=color_101r_2;
                else
                if (!strcmp(model, "NAS-101RW"))
                        ptr=color_101rw_2;
                else
                if (!strcmp(model, "NAS-2108R"))
                        ptr=color_2108r_2;
                else
                if (!strcmp(model, "NAS-2108RW"))
                        ptr=color_2108rw_2;
                else
                if (!strcmp(model, "NAS-104R") || !strcmp(model, "NAS-4300"))
                        ptr=color_104r_2;
                else
                if (!strcmp(model, "NAS-4000"))
                        ptr=color_4000_2;
		else
		if (!strcmp(model, "NAS-4020") || !strcmp(model, "NAS-4030"))
			ptr=color_4020_2;
		else
		if (!strcmp(model, "NAS-2000") || !strcmp(model, "ND-22000"))
			ptr=color_2000_2;
		else
		if (!strcmp(model, "NAS-4100") || !strcmp(model, "ND-41000") || !strcmp(model, "NAS-4120"))
			ptr=color_4100_2;
                else
                        ptr=color_4000_2;
                my_snprintf(buf, sizeof(buf), ptr, arg);
        }
	else
	if (strstr(buf, "#charset#"))
	{
		char	charset[512];
		/*ptr=hc->cookie;
		
		if (!strcmp(ptr, "nas_lang=TCH"))
			strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\">\n");
		else
		if (!strcmp(ptr, "nas_lang=ENG"))
			strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n");
		else
		if (!strcmp(ptr, "nas_lang=JPN"))
			strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">\n");
		else
		if (!strcmp(ptr, "nas_lang=SCH"))
			strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\">\n");
		else
		if (	!strcmp(ptr, "nas_lang=FRE") || !strcmp(ptr, "nas_lang=GER") ||
			!strcmp(ptr, "nas_lang=SPA") || !strcmp(ptr, "nas_lang=ITA") ||
			!strcmp(ptr, "nas_lang=POR") || !strcmp(ptr, "nas_lang=DUT") ||
			!strcmp(ptr, "nas_lang=NOR") || !strcmp(ptr, "nas_lang=FIN") ||
			!strcmp(ptr, "nas_lang=SWE") || !strcmp(ptr, "nas_lang=DAN")	)
			strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\">\n");
		else
		if (!strcmp(ptr, "nas_lang=KOR"))
			strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=KS_C_5601-1987\">\n");
		else
		if (!strcmp(ptr, "nas_lang=RUS"))
			strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1251\">\n");*/

		strcpy(charset, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");//Shone modified 2005,09,22
		my_snprintf(buf, sizeof(buf), charset, arg);
	}
        else
#endif
	if (strstr(buf, "#error#"))
	{
		switch (status)
		{
			case 302:
				ptr=err302form;
				break;
			case 400:
				ptr=httpd_err400form;
				break;
			case 401:
				ptr=err401form;
				unlock_pwd(remoteuser);
//				g_checkpassword=0;
				break;
			case 403:
				ptr=err403form;
				break;
			case 404:
				ptr=err404form;
				break;
			case 408:
				ptr=httpd_err408form;
				break;
			case 500:
				ptr=err500form;
				break;
			case 501:
				ptr=err501form;
				break;
			case 503:
				ptr=httpd_err503form;
				break;
			case 999:
				ptr=httpd_err999form;
				break;
		}
		//Vulnerability: fixed by KenChen@QNAP 2012/May/18
		if(strcasestr(arg,"script"))
			my_snprintf(buf, sizeof(buf), ptr, "");
		else
			my_snprintf(buf, sizeof(buf), ptr, arg);
	}
	//==============================================
	add_response( hc, buf );
	}
    (void) fclose( fp );

#ifdef ERR_APPEND_SERVER_INFO
    send_response_tail( hc );
#endif /* ERR_APPEND_SERVER_INFO */

    return 1;
    }
#endif /* ERR_DIR */


#ifdef AUTH_FILE

static void
send_authenticate( httpd_conn* hc, char* realm )
    {
    static char* header;
    static int maxheader = 0;
    static char headstr[] = "WWW-Authenticate: Basic realm=\"";

    httpd_realloc_str(
	&header, &maxheader, sizeof(headstr) + strlen( realm ) + 3 );
    (void) my_snprintf( header, maxheader, "%s%s\"\r\n", headstr, realm );
    httpd_send_err( hc, 401, err401title, header, err401form, hc->encodedurl );
    /* If the request was a POST then there might still be data to be read,
    ** so we need to do a lingering close.
    */
    if ( hc->method == METHOD_POST )
	hc->should_linger = 1;
    }

/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int
b64_decode( const char* str, unsigned char* space, int size )
    {
    const char* cp;
    int space_idx, phase;
    int d, prev_d = 0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[(int) *cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
// modify by Kent 2003/05/21
// change size to size-1
		if ( space_idx < size-1 )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size-1 )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size-1 )
// end
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
    }


void DEBUG_connecttion(httpd_conn* hc)
{
	FILE *fptr;

	fptr=fopen("/var/thttpd.tmp", "w");
	fprintf(fptr, "server->binding_hostname=[%s]\n", hc->hs->binding_hostname);
	fprintf(fptr, "server->server_hostname=[%s]\n", hc->hs->server_hostname);
	fprintf(fptr, "server->cgi_pattern=[%s]\n", hc->hs->cgi_pattern);
	fprintf(fptr, "server->charset=[%s]\n", hc->hs->charset);
	fprintf(fptr, "server->cwd=[%s]\n", hc->hs->cwd);
	fprintf(fptr, "server->url_pattern=[%s]\n", hc->hs->url_pattern);
	fprintf(fptr, "server->local_pattern=[%s]\n", hc->hs->local_pattern);



	fprintf(fptr, "read_buf=[%s]\n", hc->read_buf);
	fprintf(fptr, "method=[%d]\n", hc->method);
	fprintf(fptr, "status=[%d]\n", hc->status);
	fprintf(fptr, "encodedurl=[%s]\n", hc->encodedurl);
	fprintf(fptr, "decodedurl=[%s]\n", hc->decodedurl);
	fprintf(fptr, "protocol=[%s]\n", hc->protocol);
	fprintf(fptr, "origfilename=[%s]\n", hc->origfilename);
	fprintf(fptr, "expnfilename=[%s]\n", hc->expnfilename);
	fprintf(fptr, "encodings=[%s]\n", hc->encodings);
	fprintf(fptr, "pathinfo=[%s]\n", hc->pathinfo);
	fprintf(fptr, "query=[%s]\n", hc->query);
	fprintf(fptr, "referer=[%s]\n", hc->referer);
	fprintf(fptr, "useragent=[%s]\n", hc->useragent);
	fprintf(fptr, "accept=[%s]\n", hc->accept);
	fprintf(fptr, "accepte=[%s]\n", hc->accepte);
	fprintf(fptr, "cookie=[%s]\n", hc->cookie);
	fprintf(fptr, "contenttype=[%s]\n", hc->contenttype);
	fprintf(fptr, "reqhost=[%s]\n", hc->reqhost);
	fprintf(fptr, "hdrhost=[%s]\n", hc->hdrhost);
	fprintf(fptr, "hostdir=[%s]\n", hc->hostdir);
	fprintf(fptr, "authrization=[%s]\n", hc->authorization);
	fprintf(fptr, "remoteuser=[%s]\n", hc->remoteuser);
	fprintf(fptr, "response=[%s]\n", hc->response);
	fprintf(fptr, "mime_flag=[%d]\n", hc->mime_flag);
	fprintf(fptr, "conn_fd=[%d]\n", hc->conn_fd);
	fprintf(fptr, "file_address=[%s]\n", hc->file_address);
    fprintf(fptr, "got_range=[%d]\n", hc->got_range);
    fprintf(fptr, "init_byte_loc=[%lld]\n", hc->init_byte_loc);
    fprintf(fptr, "end_byte_loc=[%lld]\n", hc->end_byte_loc);
    fprintf(fptr, "keep_alive=[%d]\n", hc->keep_alive);
    fprintf(fptr, "hostname=[%s]\n", hc->hostname);

	fprintf(fptr, "\n");
	fclose(fptr);
}

void kentprint(int tmp)
{
	FILE *fptr;

	fptr=fopen("/var/thttpd.tmp", "a+");
	fprintf(fptr, "%d:\n", tmp);
	fclose(fptr);
}

void kentprintf(char *str)
{
        FILE *fptr;

        fptr=fopen("/var/thttpd.tmp", "a+");
        fprintf(fptr, "%s:\n", str);
        fclose(fptr);
}

static int
auth_check( httpd_conn* hc, char* dirname  )
    {
    static char* authpath;
    static int maxauthpath = 0;
    struct stat sb;
    char authinfo[500] = {0};
    char* authpass;
    int l;
    FILE* fp;
    char line[500];
    char* cryp;
    static char* prevauthpath;
    static int maxprevauthpath = 0;
    static time_t prevmtime;
    static char* prevuser;
    static int maxprevuser = 0;
    static char* prevcryp;
    static int maxprevcryp = 0;
    int ret;
    int isnameovermaxlength = 0;
	
	/* MultiMedia Station auth check */
	int get_auth = 0;
	char* tmpurl = NULL;
	char* tmp = NULL;
	//Richard Chang 20070903 add check user in download file
	int check_user =0;
	char check_share[256] ="";
	char* tmp2 = NULL;
	char* tmp3 = NULL;
	//Richard Chang 20070910 add onlineuser
	SHARE_NAME_LIST         *share_name_list;
	int                     g_share_num;                    //total share folder number
	char addr[64];

#ifdef STATISTICS
	int i, check_admin=0;
	char administation_list[13][32]={"cgi-bin/inidex.cgi",
				"cgi-bin/quick/quick.cgi",
				"cgi-bin/server/",
				"cgi-bin/network/",
				"cgi-bin/device.cgi",
				"cgi-bin/security.cgi",
				"cgi-bin/share.cgi",
				"cgi-bin/misc/",
				"cgi-bin/ups_setting/",
				"cgi-bin/logs.cgi",
				"cgi-bin/connlogs.cgi",
				"cgi-bin/onlineuser.cgi",
				"cgi-bin/sysinfo.cgi"};
#endif
    /* Construct auth filename. */
    httpd_realloc_str(
	&authpath, &maxauthpath, strlen( dirname ) + 1 + sizeof(AUTH_FILE) );
    (void) my_snprintf( authpath, maxauthpath,
	"%s/%s", dirname, AUTH_FILE );

	/* Check if we get authorization string from browser */
	if ( hc->authorization[0] == '\0' || strncmp( hc->authorization, "Basic ", 6 ) != 0 )
		get_auth = 0;
	else
		get_auth = 1;

	// for CMS live report
	if (strstr(hc->encodedurl, "cgi-bin/getsysinfo.cgi")&& !strcmp(httpd_ntoa(&hc->client_addr),"127.0.0.1"))
	{
		return 0;
	}

#ifdef _UIV2_
	// anonymous login on local display for ROD
	if (strncmp(hc->encodedurl, "/cgi-bin/notify.cgi", 19) == 0 && strcmp(hc->authorization, "Basic mjptzqnap209Opo6bc6p2qdtPQ==") == 0) {
                const char szSpecial[] = {0x9a, 0x3a, 0x6d, 0xce, 0xa9, 0xda, 0xa7, 0x6d, 0x3d, 0x0};
                strcpy(hc->remoteuser, szSpecial);
                return 0;
        }
	// Hugo Liao: for ptz control from local
	// Bruce Lan: Add some cgi check for anonymous user by local display
	if ( (strncmp(hc->encodedurl, "/cgi-bin/qplay.cgi", 18) == 0 
        || strncmp(hc->encodedurl, "/cgi-bin/ptz.cgi", 16)==0 
        || strncmp(hc->encodedurl, "/cgi-bin/getstream.cgi", 22)==0
        || strncmp(hc->encodedurl, "/cgi-bin/get_init_info.cgi", 26)==0
        || strncmp(hc->encodedurl, "/cgi-bin/q_status.cgi", 21)==0
        || strncmp(hc->encodedurl, "/cgi-bin/qstatusex.cgi", 22)==0 
        || strncmp(hc->encodedurl, "/cgi-bin/qcapabilities.cgi", 26)==0 
        || strncmp(hc->encodedurl, "/cgi-bin/getmultistreaminfo.cgi", 31)==0 
        || strncmp(hc->encodedurl, "/cgi-bin/param.cgi", 18)==0) 
        && (strcmp(hc->authorization, "Basic mjptzqnap2009")==0 
        || strcmp(hc->authorization, "Basic mjptzqnap209Opo6bc6p2qdtPQ==") == 0))
		return 0;
	// End
	if (Is_Stunnel_File_Flag_Enabled()) {
		if (Is_WEB_Force_SSL_File_Flag_Enabled()) {
			strncpy(addr, httpd_ntoa(&hc->client_addr), 64);
//			if(IS_IPv6_Address(addr)){
//				if(!strncasecmp(addr, "::ffff:", strlen("::ffff:"))){
//					strcpy(addr, addr+strlen("::ffff:"));
//				}
//			}
			if (strstr(addr, "127.0.0.1") != (char*) 0) { // is local client
				//is local client
			}
			else if(hc->source_ip[0] == '\0'){      //is not send by stunnel
    			char redir[128];
				int port;
				// only for HTTP 1.1
				port = Get_Profile_Integer("Stunnel","Port",443);
				if (port == 443) {
					snprintf(redir, 128, "Location: https://%s", hc->hdrhost);
				} else {
					snprintf(redir, 128, "Location: https://%s:%d", hc->hdrhost, port);
				}
				send_response( hc, 302, err302title, redir, err302form, "/" );
				return -403;
			}
		}
	}

	if ((strstr(hc->decodedurl, "/cgi-bin/filemanager/share/")) == (hc->decodedurl)) {
		if (tmp=strstr(hc->decodedurl, "sid=")) {
			if (auth_get_session(tmp+sizeof("sid=")-1, AUTH_TYPE_FILMANAGER, NULL) == 0) {
				return 0;
			}
			if (auth_get_session(tmp+sizeof("sid=")-1, AUTH_TYPE_SYSTEM, NULL) == 0) {
				return 0;
			}
		}
		//Hide permission error and url
		//httpd_send_err(
        //              hc, 403, err403title, "",
        //              ERROR_FORM( err403form, "You are not granted to browse the requested URL '%.80s' .\n" ),
        //              hc->encodedurl );
		httpd_send_err( hc, 404, err404title, "", err404form, "" );
		return -403;
	}
	
	if ((strstr(hc->decodedurl, "/cgi-bin/download/")) == (hc->decodedurl)) {
		if (tmp=strstr(hc->decodedurl, "sid=")) {
			if (auth_get_session(tmp+sizeof("sid=")-1, AUTH_TYPE_SYSTEM, NULL) == 0) {
				return 0;
			}
		}
		//Hide permission error and url
		//httpd_send_err(
        //              hc, 403, err403title, "",
        //              ERROR_FORM( err403form, "You are not granted to browse the requested URL '%.80s' .\n" ),
        //              hc->encodedurl );
		httpd_send_err( hc, 404, err404title, "", err404form, "" );
		return -403;
	}

	if (
// Hugo: this cgi support NAS_SID login
#ifndef NSS_UI3
		strstr(hc->encodedurl, "cgi-bin/aevmgt_main.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/aevt_buzzer.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/aevt_cfg_apply.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/aevt_cfg_delete.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/aevt_email.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/aevt_evt_delete.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/aevt_evt_schedule.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/aevt_sms.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/alarmoutput_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/email_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/ms_agg_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/recording_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/buzzer_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/logicin_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/ms_alarm_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/sms_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/camctrl_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/logicout_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/ms_motion_cgi.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/cam_status.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_cam_cap.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_act_list.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_act_list_by_type.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_evt_tree.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_msas_list.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/getliveimage.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/ipcam_test.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/protocol_mgmt.cgi")==0 &&
#endif
		strstr(hc->encodedurl, "cgi-bin/camera_info.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/rtc_test.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/vgetstream.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/mobileparam.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/_enter.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/nvrlogs.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/camera_view.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/getmultistreaminfo.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/advancedsetting.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/camsetting.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/_playback.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin/diagnosis.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/enter.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_ext_info.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_init_info.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_log_info.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/get_pb_auth.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/qstatusex.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/demomode.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/ld_activex.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/notify.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/setcact.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/qcapabilities.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/mrec.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/putaudiostream.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/qcmd.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/getparam.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/param.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/emap_show.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/emap_ex.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/nvrlog_xml.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/playback.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/q_status.cgi")==0 &&
//		strstr(hc->encodedurl, "cgi-bin/rec_file.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/rec_stor.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/record")==0 &&
		strstr(hc->encodedurl, "cgi-bin/recsetting.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/setschedule.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/test_system.cgi")==0 &&
        strstr(hc->encodedurl, "cgi-bin/qvrpro_system.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/total_space.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/hwtest")==0 &&
		strstr(hc->encodedurl, "cgi-bin/burnin")==0 &&
		strstr(hc->encodedurl, "cgi-bin/ptz.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/customize")==0 &&
                strstr(hc->encodedurl, "cgi-bin/cfg_sys_debug.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/failover_management.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/firmwareupgrade.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/gauth.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/getnvrinfo.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/foui.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/qlive.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/qplay.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/setcmsevt.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/setipsec.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/sms_main.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/sms_setting.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/recover.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/set_mxpg_fnum.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/restart.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/qadmst.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/nvrlog_xml.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/nvrlogs.cgi")==0 &&
//                strstr(hc->encodedurl, "cgi-bin/nas_ext_cfg.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/not_support_browser.cgi")==0 &&
//              strstr(hc->encodedurl, "cgi-bin/logical_input.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/key_management.cgi")==0 &&
                strstr(hc->encodedurl, "cgi-bin/recovery_management.cgi")==0 &&
		strstr(hc->encodedurl, "cgi-bin/Qdownload/DL_Config_bt.cgi") == 0 &&
		strstr(hc->encodedurl, "cgi-bin/Qdownload/DL_Task_Option.cgi") == 0 &&
		strstr(hc->encodedurl, "cgi-bin/Qdownload/Get_DL_List.cgi") == 0 &&
		strstr(hc->encodedurl, "cgi-bin/http_stream_start.cgi") == 0 &&
		strstr(hc->encodedurl, "cgi-bin/http_stream_stop.cgi") == 0 &&
		strstr(hc->encodedurl, "cgi-bin/getstream.cgi") == 0
		)
		return 0;

#ifdef NSS_UI3
		// Hugo: recording_cgi.cgi can confused with "cgi-bin/record", so process here
		if (strncmp(hc->encodedurl, "/cgi-bin/recording_cgi.cgi", 25) == 0)
			return 0;
#endif
		if (strstr(hc->encodedurl, "cgi-bin/http_stream_start.cgi") || strstr(hc->encodedurl, "cgi-bin/http_stream_stop.cgi"))
		{
			if (strcmp(hc->source_ip, "127.0.0.1") == 0)
				return 0;
			httpd_send_err(
				hc, 403, err403title, "",
				ERROR_FORM( err403form, "The requested URL '%.80s' is only permitted for administration group.\n" ),
				hc->encodedurl );
			return -403;
		}
#endif
    /* Does this directory have an auth file? */
//modify by Kent Tsai 2001/02/05
//    if ( stat( authpath, &sb ) < 0 )
	/* Nope, let the request go through. */
//	return 0;

    /* Does this request contain basic authorization info? */

	//=======================================================
	//	add by Kent Tsai 2001/02/14
	//	if you use filemanager.cgi to logout
	//	then clear auth
	//=======================================================
/*	if (strstr(hc->encodedurl, "logout=yes"))
	{
		lock_pwd(hc->remoteuser);
//		g_checkpassword=1;
		strcpy(hc->authorization, "B");
		return 0;
	}*/

//kentprintf(hc->encodedurl);
//kentprintf(hc->decodedurl);

	/* add by Kent 2001/07/23 */
	/* user can't execute file under /cgi-bin/filemanager/share folder */
//	if (strstr(hc->encodedurl, "cgi-bin/filemanager/share")==0)
//		return 0;
	
#ifdef _UIV2_
	if (strstr(hc->encodedurl, "treeview.js")!=0 ||
		strstr(hc->encodedurl, "/html/")!=0)
		return 0;
#endif

	if ((tmp2=strstr(hc->decodedurl, "cgi-bin/filemanager/share"))==0 &&
		strstr(hc->encodedurl, "cgi-bin/filemanager/filemanager.cgi?")==0 && 
		strstr(hc->encodedurl, "cgi-bin/filemanager/function.cgi")==0 && 
		strstr(hc->encodedurl, "cgi-bin")==0
		)
		return 0;
	//Richard Chang 20070903 add check user in download file
	if (tmp2 != NULL){
		if(strstr(hc->encodedurl, "cgi-bin/filemanager/function.cgi?")==0 &&
		   strstr(hc->encodedurl, "&func=")==0 && strstr(hc->encodedurl, "?folder=/")==0 
		   && strstr(hc->encodedurl, "?upload=/")==0 
		){
			check_user = 1;
		}
	}
//Richard Chang 20070910 add onlineuser
#ifdef STATISTICS
	for(i=0;i<13;i++){
		if (strstr(hc->encodedurl, administation_list[i])!=0){
			check_admin = 1;
			break;
		}
	}
#endif
		
    /* Decode it. */
    l = b64_decode( &(hc->authorization[6]), authinfo, sizeof(authinfo)-1 );
    authinfo[l] = '\0';
	
	//===========================================================
	// add by Kent Tsai 2001/02/02
	// the format of authinfo is "user:passwd"
	//================================================================
    /* Split into user and password. */
    authpass = strchr( authinfo, ':' );
    if ( authpass == (char*) 0 ) {
		/* No colon?  Bogus auth info. */
		send_authenticate( hc, dirname );
		return -1;
	}
    *authpass++ = '\0';

	// prevent username too long
	// Hugo modify at 2014/11/17: for NVR/QVR base 64 authentication cgi with AD user long name
//	if (strlen(authinfo) > 32+1) {
	if (strlen(authinfo) > 128+1) {
		send_authenticate( hc, dirname );
		return -1;
	}

	// block guest access by Jeff on 2013/12/2
	if(!strcmp(authinfo, "guest"))
 	{
 		send_authenticate( hc, dirname );
 		return -1;
	}
	if (strlen(authpass)>64) {
		send_authenticate( hc, dirname );
		return -1;
	}
	
//	authinfo[32+1]=0x0;
//    authpass[16]=0x0;
	// end
	
	//=======================================================
	//      if you use filemanager.cgi to logout
	//      then clear auth
	//=======================================================
#ifdef _NAS
	if (strstr(hc->encodedurl, "logout=yes")) {
		lock_pwd(authinfo);
		strcpy(hc->authorization, "B");
		Get_Exact_NAS_User_Name(authinfo);
		shm_del_user(remoteuser, remoteuserip, shm_get_app_id("thttpd"));
		SendConnToLogEngine(EVENT_TYPE_INFO, remoteuser, remoteuserip, 
			"---", (is_https() == 0) ? CONN_SERV_HTTP : CONN_SERV_HTTPS, CONN_ACTION_LOGOUT,  "---");
		return 0;
	}
#endif
	//=========================================================
	//add by Kent Tsai 2001/02/06
	//copy the username into remoteuser
	//because filemanager.cgi want to know who is online
	//so I use cgi environment vaiable "REMOTE_USER" to save this user
	//=========================================================================

	strcpy(hc->remoteuser, authinfo);
#ifdef _NAS
	Get_Exact_NAS_User_Name(authinfo);

	//Richard Chang 20070903 add check user in download file
	if(check_user == 1){
		if (Check_NAS_User_Password(authinfo, authpass) == SUCCESS) {
			g_share_num=Get_NAS_User_Accessible_Share_List_Ex(authinfo, &share_name_list);
			for (i=0;i<g_share_num;i++){
				GetPrivateProfileString(share_name_list[i].share_name, "path", "", check_share, sizeof(check_share), SAMBA_CONFIG_PATH);
				if(strstr(hc->decodedurl, check_share)!=0){
					strcpy(check_share, share_name_list[i].share_name);
					check_user = 3;
					break;
				}
			}
			
			if (g_share_num > 0 && share_name_list != NULL){
				Release_List((void*)share_name_list);
			}
			
			if(check_user == 3){
				unlock_pwd(authinfo);
				return 0;
			}else{
				send_authenticate(hc, dirname);
				return -1;
			}
		}
		else {
			send_authenticate(hc, dirname);
			return -1;
		}
	}
//Richard Chang 20070910 add onlineuser
#ifdef STATISTICS
	if(check_admin == 1)
	{
		if(hc->source_ip[0] != '\0'){
			add_administration_user(authinfo, hc->source_ip);
		}
		else{
			add_administration_user(authinfo, httpd_ntoa(&hc->client_addr));
		}
	}
#endif
	if (Check_NAS_User_Password(authinfo, authpass) == SUCCESS) {
		unlock_pwd(authinfo);
		return 0;
	}
	else {
		send_authenticate(hc, dirname);
		add_loginfail_log(hc, "f7"); //Richard add 20070821
		return -1;
	}
#endif
#ifndef _NAS
	return 0;
#endif
    /* See if we have a cached entry and can use it. */
    if ( maxprevauthpath != 0 &&
	 strcmp( authpath, prevauthpath ) == 0 &&
	 sb.st_mtime == prevmtime &&
	 strcmp( authinfo, prevuser ) == 0 )
	{
	/* Yes.  Check against the cached encrypted password. */
	if ( strcmp( crypt( authpass, prevcryp ), prevcryp ) == 0 )
	    {
	    /* Ok! */
	    httpd_realloc_str(
		&hc->remoteuser, &hc->maxremoteuser, strlen( authinfo ) );
	    (void) strcpy( hc->remoteuser, authinfo );
	    return 0;
	    }
	else
	    {
	    /* No. */
	    send_authenticate( hc, dirname );
	    return -1;
	    }
	}

    /* Open the password file. */
//modify by Kent Tsai
//use root directory .htpasswd to check password
strcpy(authpath, "./.htpasswd");

    fp = fopen( authpath, "r" );
    if ( fp == (FILE*) 0 )
	{
	/* The file exists but we can't open it?  Disallow access. */
	syslog(
	    LOG_ERR, "%.80s auth file %.80s could not be opened - %m",
	    httpd_ntoa( &hc->client_addr ), authpath );
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "The requested URL '%.80s' is protected by an authentication file, but the authentication file cannot be opened.\n" ),
	    hc->encodedurl );
	return -1;
	}

    /* Read it. */
    while ( fgets( line, sizeof(line), fp ) != (char*) 0 )
	{
	/* Nuke newline. */
	l = strlen( line );
	if ( line[l - 1] == '\n' )
	    line[l - 1] = '\0';
	/* Split into user and encrypted password. */
	cryp = strchr( line, ':' );
	if ( cryp == (char*) 0 )
	    continue;
	*cryp++ = '\0';
	/* Is this the right user? */
	if ( strcmp( line, authinfo ) == 0 )
	    {
	    /* Yes. */
	    (void) fclose( fp );
	    /* So is the password right? */
	    if ( strcmp( crypt( authpass, cryp ), cryp ) == 0 )
		{
		/* Ok! */
		httpd_realloc_str(
		    &hc->remoteuser, &hc->maxremoteuser, strlen( line ) );
		(void) strcpy( hc->remoteuser, line );
		/* And cache this user's info for next time. */
		httpd_realloc_str(
		    &prevauthpath, &maxprevauthpath, strlen( authpath ) );
		(void) strcpy( prevauthpath, authpath );
		prevmtime = sb.st_mtime;
		httpd_realloc_str(
		    &prevuser, &maxprevuser, strlen( authinfo ) );
		(void) strcpy( prevuser, authinfo );
		httpd_realloc_str( &prevcryp, &maxprevcryp, strlen( cryp ) );
		(void) strcpy( prevcryp, cryp );
		return 0;
		}
	    else
		{
		/* No. */
		send_authenticate( hc, dirname );
		return -1;
		}
	    }
	}

    /* Didn't find that user.  Access denied. */
    (void) fclose( fp );
    send_authenticate( hc, dirname );
    return -1;
    }
#endif /* AUTH_FILE */


static void
send_dirredirect( httpd_conn* hc )
    {
    static char* location;
    static char* header;
    static int maxlocation = 0, maxheader = 0;
    static char headstr[] = "Location: ";

    httpd_realloc_str( &location, &maxlocation, strlen( hc->encodedurl ) + 1 );
    (void) my_snprintf( location, maxlocation,
	"%s/", hc->encodedurl );
    httpd_realloc_str(
	&header, &maxheader, sizeof(headstr) + strlen( location ) );
    (void) my_snprintf( header, maxheader,
	"%s%s\r\n", headstr, location );
    //send_response( hc, 302, err302title, header, err302form, location ); // vulnerablity fixed. KenChen@QNAP 2013/Jun/22
    send_response( hc, 302, err302title, header, err302form, "Not found." );
    }


char*
httpd_method_str( int method )
    {
    switch ( method )
	{
	case METHOD_GET: return "GET";
	case METHOD_HEAD: return "HEAD";
	case METHOD_POST: return "POST";
	default: return (char*) 0;
	}
    }


static int
hexit( char c )
    {
    if ( c >= '0' && c <= '9' )
	return c - '0';
    if ( c >= 'a' && c <= 'f' )
	return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
	return c - 'A' + 10;
    return 0;           /* shouldn't happen, we're guarded by isxdigit() */
    }


/* Copies and decodes a string.  It's ok for from and to to be the
** same string.
*/
static void
strdecode( char* to, char* from )
    {
    for ( ; *from != '\0'; ++to, ++from )
	{
	if ( from[0] == '%' && isxdigit( from[1] ) && isxdigit( from[2] ) )
	    {
	    *to = hexit( from[1] ) * 16 + hexit( from[2] );
	    from += 2;
	    }
	else
	    *to = *from;
	}
    *to = '\0';
    }


#ifdef GENERATE_INDEXES
/* Copies and encodes a string. */
static void
strencode( char* to, int tosize, char* from )
    {
    int tolen;

    for ( tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from )
	{
	if ( isalnum(*from) || strchr( "/_.-~", *from ) != (char*) 0 )
	    {
	    *to = *from;
	    ++to;
	    ++tolen;
	    }
	else
	    {
	    (void) sprintf( to, "%%%02x", (int) *from & 0xff );
	    to += 3;
	    tolen += 3;
	    }
	}
    *to = '\0';
    }
#endif /* GENERATE_INDEXES */


#ifdef TILDE_MAP_1
/* Map a ~username/whatever URL into <prefix>/username. */
static int
tilde_map_1( httpd_conn* hc )
    {
    static char* temp;
    static int maxtemp = 0;
    int len;
    static char* prefix = TILDE_MAP_1;

    len = strlen( hc->expnfilename ) - 1;
    httpd_realloc_str( &temp, &maxtemp, len );
    (void) strcpy( temp, &hc->expnfilename[1] );
    httpd_realloc_str(
	&hc->expnfilename, &hc->maxexpnfilename, strlen( prefix ) + 1 + len );
    (void) strcpy( hc->expnfilename, prefix );
    if ( prefix[0] != '\0' )
	(void) strcat( hc->expnfilename, "/" );
    (void) strcat( hc->expnfilename, temp );
    return 1;
    }
#endif /* TILDE_MAP_1 */

#ifdef TILDE_MAP_2
/* Map a ~username/whatever URL into <user's homedir>/<postfix>. */
static int
tilde_map_2( httpd_conn* hc )
    {
    static char* temp;
    static int maxtemp = 0;
    static char* postfix = TILDE_MAP_2;
    char* cp;
    struct passwd* pw;
    char* alt;
    char* rest;

    /* Get the username. */
    httpd_realloc_str( &temp, &maxtemp, strlen( hc->expnfilename ) - 1 );
    (void) strcpy( temp, &hc->expnfilename[1] );
    cp = strchr( temp, '/' );
    if ( cp != (char*) 0 )
	*cp++ = '\0';
    else
	cp = "";

    /* Get the passwd entry. */
    pw = getpwnam( temp );
    if ( pw == (struct passwd*) 0 )
	return 0;

    /* Set up altdir. */
    httpd_realloc_str(
	&hc->altdir, &hc->maxaltdir,
	strlen( pw->pw_dir ) + 1 + strlen( postfix ) );
    (void) strcpy( hc->altdir, pw->pw_dir );
    if ( postfix[0] != '\0' )
	{
	(void) strcat( hc->altdir, "/" );
	(void) strcat( hc->altdir, postfix );
	}
    alt = expand_symlinks( hc->altdir, &rest, 0, 1 );
    if ( rest[0] != '\0' )
	return 0;
    httpd_realloc_str( &hc->altdir, &hc->maxaltdir, strlen( alt ) );
    (void) strcpy( hc->altdir, alt );

    /* And the filename becomes altdir plus the post-~ part of the original. */
    httpd_realloc_str(
	&hc->expnfilename, &hc->maxexpnfilename,
	strlen( hc->altdir ) + 1 + strlen( cp ) );
    (void) my_snprintf( hc->expnfilename, hc->maxexpnfilename,
	"%s/%s", hc->altdir, cp );

    /* For this type of tilde mapping, we want to defeat vhost mapping. */
    hc->tildemapped = 1;

    return 1;
    }
#endif /* TILDE_MAP_2 */


/* Virtual host mapping. */
static int
vhost_map( httpd_conn* hc )
    {
    httpd_sockaddr sa;
    int sz;
    static char* tempfilename;
    static int maxtempfilename = 0;
    char* cp1;
    int len;
#ifdef VHOST_DIRLEVELS
    int i;
    char* cp2;
#endif /* VHOST_DIRLEVELS */

    /* Figure out the virtual hostname. */
    if ( hc->reqhost[0] != '\0' )
	hc->hostname = hc->reqhost;
    else if ( hc->hdrhost[0] != '\0' )
	hc->hostname = hc->hdrhost;
    else
	{
	sz = sizeof(sa);
	if ( getsockname( hc->conn_fd, &sa.sa, &sz ) < 0 )
	    {
	    syslog( LOG_ERR, "getsockname - %m" );
	    return 0;
	    }
	hc->hostname = httpd_ntoa( &sa );
	}
    /* Pound it to lower case. */
    for ( cp1 = hc->hostname; *cp1 != '\0'; ++cp1 )
	if ( isupper( *cp1 ) )
	    *cp1 = tolower( *cp1 );

    if ( hc->tildemapped )
	return 1;

    /* Figure out the host directory. */
#ifdef VHOST_DIRLEVELS
    httpd_realloc_str(
	&hc->hostdir, &hc->maxhostdir,
	strlen( hc->hostname ) + 2 * VHOST_DIRLEVELS );
    if ( strncmp( hc->hostname, "www.", 4 ) == 0 )
	cp1 = &hc->hostname[4];
    else
	cp1 = hc->hostname;
    for ( cp2 = hc->hostdir, i = 0; i < VHOST_DIRLEVELS; ++i )
	{
	*cp2++ = *cp1++;
	*cp2++ = '/';
	}
    (void) strcpy( cp2, hc->hostname );
#else /* VHOST_DIRLEVELS */
    httpd_realloc_str( &hc->hostdir, &hc->maxhostdir, strlen( hc->hostname ) );
    (void) strcpy( hc->hostdir, hc->hostname );
#endif /* VHOST_DIRLEVELS */

    /* Prepend hostdir to the filename. */
    len = strlen( hc->expnfilename );
    httpd_realloc_str( &tempfilename, &maxtempfilename, len );
    (void) strcpy( tempfilename, hc->expnfilename );
    httpd_realloc_str(
	&hc->expnfilename, &hc->maxexpnfilename,
	strlen( hc->hostdir ) + 1 + len );
    (void) strcpy( hc->expnfilename, hc->hostdir );
    (void) strcat( hc->expnfilename, "/" );
    (void) strcat( hc->expnfilename, tempfilename );
    return 1;
    }


/* Expands all symlinks in the given filename, eliding ..'s and leading /'s.
** Returns the expanded path (pointer to static string), or (char*) 0 on
** errors.  Also returns, in the string pointed to by restP, any trailing
** parts of the path that don't exist.
**
** This is a fairly nice little routine.  It handles any size filenames
** without excessive mallocs.
*/
static char*
expand_symlinks( char* path, char** restP, int no_symlink, int tildemapped )
    {
    static char* checked;
    static char* rest;
    char link[5000];
    static int maxchecked = 0, maxrest = 0;
    int checkedlen, restlen, linklen, prevcheckedlen, prevrestlen, nlinks, i;
    char* r;
    char* cp1;
    char* cp2;

    if ( no_symlink )
	{
	/* If we are chrooted, we can actually skip the symlink-expansion,
	** since it's impossible to get out of the tree.  However, we still
	** need to do the pathinfo check, and the existing symlink expansion
	** code is a pretty reasonable way to do this.  So, what we do is
	** a single stat() of the whole filename - if it exists, then we
	** return it as is with nothing in restP.  If it doesn't exist, we
	** fall through to the existing code.
	**
	** One side-effect of this is that users can't symlink to central
	** approved CGIs any more.  The workaround is to use the central
	** URL for the CGI instead of a local symlinked one.
	*/
	struct stat sb;
	if ( stat( path, &sb ) != -1 )
	    {
	    httpd_realloc_str( &checked, &maxchecked, strlen( path ) );
	    (void) strcpy( checked, path );
	    httpd_realloc_str( &rest, &maxrest, 0 );
	    rest[0] = '\0';
	    *restP = rest;
	    return checked;
	    }
	}

    /* Start out with nothing in checked and the whole filename in rest. */
    httpd_realloc_str( &checked, &maxchecked, 1 );
    checked[0] = '\0';
    checkedlen = 0;
    restlen = strlen( path );
    httpd_realloc_str( &rest, &maxrest, restlen );
    (void) strcpy( rest, path );
    if ( rest[restlen - 1] == '/' )
	rest[--restlen] = '\0';         /* trim trailing slash */
    if ( ! tildemapped )
	/* Remove any leading slashes. */
	while ( rest[0] == '/' )
	    {
	    (void) strcpy( rest, &(rest[1]) );
	    --restlen;
	    }
    r = rest;
    nlinks = 0;

    /* While there are still components to check... */
    while ( restlen > 0 )
	{
	/* Save current checkedlen in case we get a symlink.  Save current
	** restlen in case we get a non-existant component.
	*/
	prevcheckedlen = checkedlen;
	prevrestlen = restlen;

	/* Grab one component from r and transfer it to checked. */
	cp1 = strchr( r, '/' );
	if ( cp1 != (char*) 0 )
	    {
	    i = cp1 - r;
	    if ( i == 0 )
		{
		/* Special case for absolute paths. */
		httpd_realloc_str( &checked, &maxchecked, checkedlen + 1 );
		(void) strncpy( &checked[checkedlen], r, 1 );
		checkedlen += 1;
		}
	    else if ( strncmp( r, "..", MAX( i, 2 ) ) == 0 )
		{
		/* Ignore ..'s that go above the start of the path. */
		if ( checkedlen != 0 )
		    {
		    cp2 = strrchr( checked, '/' );
		    if ( cp2 == (char*) 0 )
			checkedlen = 0;
		    else if ( cp2 == checked )
			checkedlen = 1;
		    else
			checkedlen = cp2 - checked;
		    }
		}
	    else
		{
		httpd_realloc_str( &checked, &maxchecked, checkedlen + 1 + i );
		if ( checkedlen > 0 && checked[checkedlen-1] != '/' )
		    checked[checkedlen++] = '/';
		(void) strncpy( &checked[checkedlen], r, i );
		checkedlen += i;
		}
	    checked[checkedlen] = '\0';
	    r += i + 1;
	    restlen -= i + 1;
	    }
	else
	    {
	    /* No slashes remaining, r is all one component. */
	    if ( strcmp( r, ".." ) == 0 )
		{
		/* Ignore ..'s that go above the start of the path. */
		if ( checkedlen != 0 )
		    {
		    cp2 = strrchr( checked, '/' );
		    if ( cp2 == (char*) 0 )
			checkedlen = 0;
		    else if ( cp2 == checked )
			checkedlen = 1;
		    else
			checkedlen = cp2 - checked;
		    checked[checkedlen] = '\0';
		    }
		}
	    else
		{
		httpd_realloc_str(
		    &checked, &maxchecked, checkedlen + 1 + restlen );
		if ( checkedlen > 0 && checked[checkedlen-1] != '/' )
		    checked[checkedlen++] = '/';
		(void) strcpy( &checked[checkedlen], r );
		checkedlen += restlen;
		}
	    r += restlen;
	    restlen = 0;
	    }

	/* Try reading the current filename as a symlink */
	if ( checked[0] == '\0' )
	    continue;
	linklen = readlink( checked, link, sizeof(link) );
	if ( linklen == -1 )
	    {
	    if ( errno == EINVAL )
		continue;               /* not a symlink */
	    if ( errno == EACCES || errno == ENOENT || errno == ENOTDIR )
		{
		/* That last component was bogus.  Restore and return. */
		*restP = r - ( prevrestlen - restlen );
		if ( prevcheckedlen == 0 )
		    (void) strcpy( checked, "." );
		else
		    checked[prevcheckedlen] = '\0';
		return checked;
		}
	    syslog( LOG_ERR, "readlink %.80s - %m", checked );
	    return (char*) 0;
	    }
	++nlinks;
	if ( nlinks > MAX_LINKS )
	    {
	    syslog( LOG_ERR, "too many symlinks in %.80s", path );
	    return (char*) 0;
	    }
	link[linklen] = '\0';
	if ( link[linklen - 1] == '/' )
	    link[--linklen] = '\0';     /* trim trailing slash */

	/* Insert the link contents in front of the rest of the filename. */
	if ( restlen != 0 )
	    {
	    (void) strcpy( rest, r );
	    httpd_realloc_str( &rest, &maxrest, restlen + linklen + 1 );
	    for ( i = restlen; i >= 0; --i )
		rest[i + linklen + 1] = rest[i];
	    (void) strcpy( rest, link );
	    rest[linklen] = '/';
	    restlen += linklen + 1;
	    r = rest;
	    }
	else
	    {
	    /* There's nothing left in the filename, so the link contents
	    ** becomes the rest.
	    */
	    httpd_realloc_str( &rest, &maxrest, linklen );
	    (void) strcpy( rest, link );
	    restlen = linklen;
	    r = rest;
	    }

	if ( rest[0] == '/' )
	    {
	    /* There must have been an absolute symlink - zero out checked. */
	    checked[0] = '\0';
	    checkedlen = 0;
	    }
	else
	    {
	    /* Re-check this component. */
	    checkedlen = prevcheckedlen;
	    checked[checkedlen] = '\0';
	    }
	}

    /* Ok. */
    *restP = r;
    if ( checked[0] == '\0' )
	(void) strcpy( checked, "." );
    return checked;
    }


int
httpd_get_conn( httpd_server* hs, int listen_fd, httpd_conn* hc )
    {
    httpd_sockaddr sa;
    int sz;

#if defined(NVR_DEBUG)
    deb_log( ACT_GET_BEGIN, hc, -1 );
    deb_log( ACT_GET_BEGIN2, hc, hc->conn_fd );
#endif
    if ( ! hc->initialized )
	{
	hc->encodedurl = 0; //Fix bug#34667
	hc->read_size = 0;
	//httpd_realloc_str( &hc->read_buf, &hc->read_size, 500 ); /* mark by Nick */
	httpd_realloc_str( &hc->read_buf, &hc->read_size, 1460 ); /* add by Nick */
	hc->maxdecodedurl =
	    hc->maxorigfilename = hc->maxexpnfilename = hc->maxencodings =
	    hc->maxpathinfo = hc->maxquery = hc->maxaccept =
	    hc->maxaccepte = hc->maxreqhost = hc->maxhostdir =
	    hc->maxremoteuser = hc->maxresponse = 0;
#ifdef TILDE_MAP_2
	hc->maxaltdir = 0;
#endif /* TILDE_MAP_2 */
	httpd_realloc_str( &hc->decodedurl, &hc->maxdecodedurl, 1 );
	httpd_realloc_str( &hc->origfilename, &hc->maxorigfilename, 1 );
	httpd_realloc_str( &hc->expnfilename, &hc->maxexpnfilename, 0 );
	httpd_realloc_str( &hc->encodings, &hc->maxencodings, 0 );
	httpd_realloc_str( &hc->pathinfo, &hc->maxpathinfo, 0 );
	httpd_realloc_str( &hc->query, &hc->maxquery, 0 );
	httpd_realloc_str( &hc->accept, &hc->maxaccept, 0 );
	httpd_realloc_str( &hc->accepte, &hc->maxaccepte, 0 );
	httpd_realloc_str( &hc->reqhost, &hc->maxreqhost, 0 );
	httpd_realloc_str( &hc->hostdir, &hc->maxhostdir, 0 );
	httpd_realloc_str( &hc->remoteuser, &hc->maxremoteuser, 0 );
	httpd_realloc_str( &hc->response, &hc->maxresponse, 0 );
#ifdef TILDE_MAP_2
	httpd_realloc_str( &hc->altdir, &hc->maxaltdir, 0 );
#endif /* TILDE_MAP_2 */
	hc->initialized = 1;
	}

    /* Accept the new connection. */
    sz = sizeof(sa);
    hc->conn_fd = accept( listen_fd, &sa.sa, &sz );
#if defined(NVR_DEBUG)
    deb_log( ACT_ACCEPT, hc, hc->conn_fd );
#endif
    if ( hc->conn_fd < 0 )
	{
	if ( errno == EWOULDBLOCK )
	    return GC_NO_MORE;
	syslog( LOG_ERR, "accept - %m" );
	return GC_FAIL;
	}
    if ( ! sockaddr_check( &sa ) )
	{
	syslog( LOG_ERR, "unknown sockaddr family" );
	return GC_FAIL;
	}

#ifdef	QNAP_SERVICE_BINDING
	// Check service binding here!
	{
		char  szAddrBuf[INET_ADDRSTRLEN + INET6_ADDRSTRLEN]={0};
		if (g_bHup)
		{
			Get_Binding_Address(&g_tBindAddr);
			g_bHup = FALSE;
		}
		if (g_tBindAddr.bEnabled && (0 != Check_Binding_Address(hc->conn_fd, &g_tBindAddr, szAddrBuf)))
		{
			close(hc->conn_fd);
			hc->conn_fd = -1;
			return GC_FAIL;
		}
	}
#endif	//QNAP_SERVICE_BINDING
	
    (void) fcntl( hc->conn_fd, F_SETFD, 1 );
    hc->hs = hs;
    memset( &hc->client_addr, 0, sizeof(hc->client_addr) );
    memcpy( &hc->client_addr, &sa, sockaddr_len( &sa ) );
    hc->read_idx = 0;
    hc->checked_idx = 0;
    hc->checked_state = CHST_FIRSTWORD;
    hc->protocol = "HTTP/1.0";
    hc->mime_flag = 1;
    hc->should_linger = 0;
    hc->responselen = 0;
    hc->file_address = (char*) 0;
    hc->decodedurl[0] = hc->origfilename[0] = hc->expnfilename[0] =
	hc->encodings[0] = hc->pathinfo[0] = hc->query[0] = hc->accept[0] =
	hc->accepte[0] = hc->reqhost[0] = hc->hostdir[0] = hc->remoteuser[0] =
	hc->response[0] = '\0';
#ifdef TILDE_MAP_2
    hc->altdir[0] = '\0';
#endif /* TILDE_MAP_2 */
    hc->referer = "";
    hc->useragent = "";
    hc->cookie = "";
    hc->contenttype = "";
    hc->hdrhost = "";
    hc->authorization = "";
    hc->if_modified_since = (time_t) -1;
    hc->range_if = (time_t) -1;
    hc->contentlength = -1;
    hc->got_range = 0;
    hc->tildemapped = 0;
    hc->init_byte_loc = 0;
    hc->end_byte_loc = -1;
    hc->keep_alive = 0;
    hc->hostname = (char*) 0;
    hc->accept_language = "";
    hc->x_apache_proxy = "";
    memset(hc->source_ip, 0, sizeof(hc->source_ip));
    return GC_OK;
    }


/* Checks hc->read_buf to see whether a complete request has been read so far;
** either the first line has two words (an HTTP/0.9 request), or the first
** line has three words and there's a blank line present.
**
** hc->read_idx is how much has been read in; hc->checked_idx is how much we
** have checked so far; and hc->checked_state is the current state of the
** finite state machine.
*/
int
httpd_got_request( httpd_conn* hc )
    {
    char c;

    for ( ; hc->checked_idx < hc->read_idx; ++hc->checked_idx )
	{
	c = hc->read_buf[hc->checked_idx];
	switch ( hc->checked_state )
	    {
	    case CHST_FIRSTWORD:
	    switch ( c )
		{
		case ' ': case '\t':
		hc->checked_state = CHST_FIRSTWS;
		break;
		case '\n': case '\r':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		}
	    break;
	    case CHST_FIRSTWS:
	    switch ( c )
		{
		case ' ': case '\t':
		break;
		case '\n': case '\r':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		default:
		hc->checked_state = CHST_SECONDWORD;
		break;
		}
	    break;
	    case CHST_SECONDWORD:
	    switch ( c )
		{
		case ' ': case '\t':
		hc->checked_state = CHST_SECONDWS;
		break;
		case '\n': case '\r':
		/* The first line has only two words - an HTTP/0.9 request. */
		return GR_GOT_REQUEST;
		}
	    break;
	    case CHST_SECONDWS:
	    switch ( c )
		{
		case ' ': case '\t':
		break;
		case '\n': case '\r':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		default:
		hc->checked_state = CHST_THIRDWORD;
		break;
		}
	    break;
	    case CHST_THIRDWORD:
	    switch ( c )
		{
		case ' ': case '\t':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		case '\n':
		hc->checked_state = CHST_LF;
		break;
		case '\r':
		hc->checked_state = CHST_CR;
		break;
		}
	    break;
	    case CHST_LINE:
	    switch ( c )
		{
		case '\n':
		hc->checked_state = CHST_LF;
		break;
		case '\r':
		hc->checked_state = CHST_CR;
		break;
		}
	    break;
	    case CHST_LF:
	    switch ( c )
		{
		case '\n':
		/* Two newlines in a row - a blank line - end of request. */
		return GR_GOT_REQUEST;
		case '\r':
		hc->checked_state = CHST_CR;
		break;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_CR:
	    switch ( c )
		{
		case '\n':
		hc->checked_state = CHST_CRLF;
		break;
		case '\r':
		/* Two returns in a row - end of request. */
		return GR_GOT_REQUEST;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_CRLF:
	    switch ( c )
		{
		case '\n':
		/* Two newlines in a row - end of request. */
		return GR_GOT_REQUEST;
		case '\r':
		hc->checked_state = CHST_CRLFCR;
		break;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_CRLFCR:
	    switch ( c )
		{
		case '\n': case '\r':
		/* Two CRLFs or two CRs in a row - end of request. */
		return GR_GOT_REQUEST;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_BOGUS:
	    return GR_BAD_REQUEST;
	    }
	}
    return GR_NO_REQUEST;
    }


int
httpd_parse_request( httpd_conn* hc )
    {
    char* buf;
    char* method_str;
    char* url;
    char* protocol;
    char* reqhost;
    char* eol;
    char* cp;
    char* pi;
    char authinfo[500], *authpass;
    char local_addr[INET_ADDRSTRLEN + INET6_ADDRSTRLEN]={0};
    int l;

    hc->checked_idx = 0;
    hc->mime_flag = 0;
    hc->one_one = 0;
//2010-01-29 Richard Chang add for stunnel send source ip
//2012-02-08 Jeff Chang modified for https service binding.
	if(hc->read_buf[0] == '['){
		int read_buf_len = 0;
		hc->checked_idx = strcspn(hc->read_buf, "GP");
		cp = &hc->read_buf[1];
		read_buf_len = strcspn(cp, ",");
		memset(hc->source_ip, 0, sizeof(hc->source_ip));
		strncpy(hc->source_ip, cp, read_buf_len);
		if(NULL != (cp = strchr(cp, ','))) cp++;
		read_buf_len = strcspn(cp, "]");
		strncpy(local_addr, cp, read_buf_len);
		if (g_tBindAddr.bEnabled && (0 != Check_Binding_Address(0, &g_tBindAddr, local_addr)))
		{
			return -1;
		}
		cp = &hc->read_buf[hc->checked_idx];
		read_buf_len = strcspn(cp, " \t\n\r" );
		strncpy(hc->read_buf, cp, read_buf_len);
		hc->read_buf[read_buf_len] = '\0';
	}
///////////////////////////
    method_str = bufgets( hc );
    url = strpbrk( method_str, " \t\n\r" );
    if ( url == (char*) 0 )
	{
	httpd_send_err( hc, 400, httpd_err400title, "", httpd_err400form, "" );
	return -1;
	}
    *url++ = '\0';
    url += strspn( url, " \t\n\r" );
    protocol = strpbrk( url, " \t\n\r" );
    if ( protocol == (char*) 0 )
	protocol = "HTTP/0.9";
    else
	{
	*protocol++ = '\0';
	protocol += strspn( protocol, " \t\n\r" );
	if ( *protocol != '\0' )
	    {
	    hc->mime_flag = 1;
	    eol = strpbrk( protocol, " \t\n\r" );
	    if ( eol != (char*) 0 )
		*eol = '\0';
	    if ( strcasecmp( protocol, "HTTP/1.0" ) != 0 )
		hc->one_one = 1;
	    }
	}
    /* Check for HTTP/1.1 absolute URL. */
    if ( strncasecmp( url, "http://", 7 ) == 0 )
	{
	if ( ! hc->one_one )
	    {
	    httpd_send_err( hc, 400, httpd_err400title, "", httpd_err400form, "" );
	    return -1;
	    }
	reqhost = url + 7;
	url = strchr( reqhost, '/' );
	if ( url == (char*) 0 )
	    {
	    httpd_send_err( hc, 400, httpd_err400title, "", httpd_err400form, "" );
	    return -1;
	    }
	*url = '\0';
	httpd_realloc_str( &hc->reqhost, &hc->maxreqhost, strlen( reqhost ) );
	(void) strcpy( hc->reqhost, reqhost );
	*url = '/';
	}

    if ( strcasecmp( method_str, httpd_method_str( METHOD_GET ) ) == 0 )
	hc->method = METHOD_GET;
    else if ( strcasecmp( method_str, httpd_method_str( METHOD_HEAD ) ) == 0 )
	hc->method = METHOD_HEAD;
    else if ( strcasecmp( method_str, httpd_method_str( METHOD_POST ) ) == 0 )
	hc->method = METHOD_POST;
    else
	{
	httpd_send_err( hc, 501, err501title, "", err501form, method_str );
	return -1;
	}

    hc->encodedurl = url;

    // add by albert
    // if Is_WFM_Port_Enabled() is TRUE then 
    // only wfm_thttpd could access /cgi-bin/filemanager/
    buf = strrchr(argv0, '/');
    buf++;
    if( !strncmp(hc->encodedurl, "/cgi-bin/filemanager", 20) ) {
	if ( !strncmp(buf, "wfm_thttpd", strlen(buf)))
	{
		hc->encodedurl += 20;
	} 
/*	//The function, WFM_Port is not included in QTS
	else if ( !strncmp(buf, "_thttpd_", strlen(buf)) && 
						Is_WFM_Port_Enabled())
	{
	httpd_send_err( hc, 400, httpd_err400title, "", httpd_err400form, "" );
	return -1;
	}
*/
   }

// Meiji 00-12-28 mark the changes...
    /*-- add by Nick Start --*/
//    if (strcmp(hc->encodedurl,"/")==0)  strcpy(hc->encodedurl,"/index.cgi");
    /*-- add by Nick End   --*/

    httpd_realloc_str(
	&hc->decodedurl, &hc->maxdecodedurl, strlen( hc->encodedurl ) );
    strdecode( hc->decodedurl, hc->encodedurl );

    de_dotdot( hc->decodedurl );
    if ( hc->decodedurl[0] != '/' || hc->decodedurl[1] == '/' ||
	 ( hc->decodedurl[1] == '.' && hc->decodedurl[2] == '.' &&
	   ( hc->decodedurl[3] == '\0' || hc->decodedurl[3] == '/' ) ) )
	{
	httpd_send_err( hc, 400, httpd_err400title, "", httpd_err400form, "" );
	return -1;
	}

    hc->protocol = protocol;

    if ( hc->decodedurl[0] != '/' )
	{
	httpd_send_err( hc, 400, httpd_err400title, "", httpd_err400form, "" );
	return -1;
	}
    httpd_realloc_str(
	&hc->origfilename, &hc->maxorigfilename, strlen( hc->decodedurl ) );
    (void) strcpy( hc->origfilename, &hc->decodedurl[1] );
    /* Special case for top-level URL. */
    if ( hc->origfilename[0] == '\0' )
	(void) strcpy( hc->origfilename, "." );

    /* Extract query string from encoded URL. */
    cp = strchr( hc->encodedurl, '?' );
    if ( cp != (char*) 0 )
	{
	++cp;
	httpd_realloc_str( &hc->query, &hc->maxquery, strlen( cp ) );
	(void) strcpy( hc->query, cp );
	}
    /* And remove query from filename. */
    cp = strchr( hc->origfilename, '?' );
    if ( cp != (char*) 0 )
	*cp = '\0';

    if ( hc->mime_flag )
	{
	/* Read the MIME headers. */
	while ( ( buf = bufgets( hc ) ) != (char*) 0 )
	    {
	    if ( buf[0] == '\0' )
		break;
	    if ( strncasecmp( buf, "Referer:", 8 ) == 0 )
		{
		cp = &buf[8];
		cp += strspn( cp, " \t" );
		hc->referer = cp;
		}
	    else if ( strncasecmp( buf, "User-Agent:", 11 ) == 0 )
		{
		cp = &buf[11];
		cp += strspn( cp, " \t" );
		hc->useragent = cp;
		}
	    else if ( strncasecmp( buf, "Host:", 5 ) == 0 )
		{
		cp = &buf[5];
		cp += strspn( cp, " \t" );
		hc->hdrhost = cp;
		cp = strchr( hc->hdrhost, ']' );
		
		if ( cp != (char*) 0) { // IPv6
			*(cp+1) = '\0';
		} else {
			cp = strchr( hc->hdrhost, ':' );
			if ( cp != (char*) 0 )
				*cp = '\0';
		}
		}
	    else if ( strncasecmp( buf, "Accept:", 7 ) == 0 )
		{
		cp = &buf[7];
		cp += strspn( cp, " \t" );
		if ( hc->accept[0] != '\0' )
		    {
		    if ( strlen( hc->accept ) > 5000 )
			{
			syslog(
			    LOG_ERR, "%.80s way too much Accept: data",
			    httpd_ntoa( &hc->client_addr ) );
			continue;
			}
		    httpd_realloc_str(
			&hc->accept, &hc->maxaccept,
			strlen( hc->accept ) + 2 + strlen( cp ) );
		    (void) strcat( hc->accept, ", " );
		    }
		else
		    httpd_realloc_str(
			&hc->accept, &hc->maxaccept, strlen( cp ) );
		(void) strcat( hc->accept, cp );
		}
	    else if ( strncasecmp( buf, "Accept-Encoding:", 16 ) == 0 )
		{
		cp = &buf[16];
		cp += strspn( cp, " \t" );
		if ( hc->accepte[0] != '\0' )
		    {
		    if ( strlen( hc->accepte ) > 5000 )
			{
			syslog(
			    LOG_ERR, "%.80s way too much Accept-Encoding: data",
			    httpd_ntoa( &hc->client_addr ) );
			continue;
			}
		    httpd_realloc_str(
			&hc->accepte, &hc->maxaccepte,
			strlen( hc->accepte ) + 2 + strlen( cp ) );
		    (void) strcat( hc->accepte, ", " );
		    }
		else
		    httpd_realloc_str(
			&hc->accepte, &hc->maxaccepte, strlen( cp ) );
		(void) strcpy( hc->accepte, cp );
		}
	    else if ( strncasecmp( buf, "If-Modified-Since:", 18 ) == 0 )
		{
		cp = &buf[18];
		hc->if_modified_since = tdate_parse( cp );
		if ( hc->if_modified_since == (time_t) -1 )
		    syslog( LOG_DEBUG, "unparsable time: %.80s", cp );
		}
	    else if ( strncasecmp( buf, "Cookie:", 7 ) == 0 )
		{
		cp = &buf[7];
		cp += strspn( cp, " \t" );
		hc->cookie = cp;
		}
	    else if ( strncasecmp( buf, "Range:", 6 ) == 0 )
		{
		/* Only support %d- and %d-%d, not %d-%d,%d-%d or -%d. */
		if ( strchr( buf, ',' ) == (char*) 0 )
		    {
		    char* cp_dash;
		    cp = strpbrk( buf, "=" );
		    if ( cp != (char*) 0 )
			{
			cp_dash = strchr( cp + 1, '-' );
			if ( cp_dash != (char*) 0 && cp_dash != cp + 1 )
			    {
			    *cp_dash = '\0';
			    hc->got_range = 1;
			    hc->init_byte_loc = atoll( cp + 1 );
			    if ( cp_dash[1] )
				hc->end_byte_loc = atoll( cp_dash + 1 );
			    }
			}
		    }
		}
	    else if ( strncasecmp( buf, "Range-If:", 9 ) == 0 ||
		      strncasecmp( buf, "If-Range:", 9 ) == 0 )
		{
		cp = &buf[9];
		hc->range_if = tdate_parse( cp );
		if ( hc->range_if == (time_t) -1 )
		    syslog( LOG_DEBUG, "unparsable time: %.80s", cp );
		}
	    else if ( strncasecmp( buf, "Content-Type:", 13 ) == 0 )
		{
		cp = &buf[13];
		cp += strspn( cp, " \t" );
		hc->contenttype = cp;
		}
	    else if ( strncasecmp( buf, "Content-Length:", 15 ) == 0 )
		{
		cp = &buf[15];
		//hc->contentlength = atol( cp );
		hc->contentlength = atoll( cp ); //KenChen@QNAP
		}
	    else if ( strncasecmp( buf, "Authorization:", 14 ) == 0 )
		{
		cp = &buf[14];
		cp += strspn( cp, " \t" );
		//============================================================
		//	add by Kent Tsai 2001/02/14
		//
		//==============================================================
		/* Decode it. */
		l = b64_decode( &(cp[6]), authinfo, sizeof(authinfo) );
		authinfo[l] = '\0';

        //===========================================================
        // add by Kent Tsai 2001/02/02
        // the format of authinfo is "user:passwd"
        //================================================================
		/* Split into user and password. */
		authpass = strchr( authinfo, ':' );
		if (authpass == NULL) continue;
		*authpass++=0x0;
		strcpy(remoteuser, authinfo);
//		strcpy(remoteuserip, hc->hdrhost);
		if(hc->source_ip[0] != '\0'){
			strncpy(remoteuserip, hc->source_ip, 64);
		}
		else{
			strncpy(remoteuserip, httpd_ntoa( &hc->client_addr ), 64);
		}

		if (get_lock_pwd(authinfo))
//		if (g_checkpassword==1)
		{
			strcpy(cp, "B");
		}
		hc->authorization = cp;
		}
	    else if ( strncasecmp( buf, "Connection:", 11 ) == 0 )
		{
		cp = &buf[11];
		cp += strspn( cp, " \t" );
		if ( strcasecmp( cp, "keep-alive" ) == 0 )
		    hc->keep_alive = 1;
		}
		else if ( strncasecmp( buf, "Accept-Language:", 16 ) == 0 ) //add this for mobile language by RichardChang 2012/Oct/05 
		{
		cp = &buf[16];
		cp += strspn( cp, " \t" );
		hc->accept_language = cp;
		}
		else if ( strncasecmp( buf, "X-Apache-Proxy:", 15 ) == 0 )
		{
            cp = &buf[16];
            cp += strspn( cp, " \t" );
            hc->x_apache_proxy = cp;
		}
		else if ( strncasecmp( buf, "X-Forwarded-For:", 16 ) == 0 ) { //add this for apache ssl by RichardChang 2013/Apr/30
			cp = &buf[16];
			cp += strspn( cp, " \t" );
			memset(hc->source_ip, 0, sizeof(hc->source_ip));
			strncpy(hc->source_ip, cp, 64);
			cp = strchr( hc->source_ip, ',' );
			if ( cp != (char*) 0) {
				*(cp) = '\0';
			}
			strncpy(remoteuserip, hc->source_ip, 64);
		}
		else if ( strncasecmp( buf, "X-Forwarded-Host:", 17 ) == 0 ) { //add this for apache ssl by RichardChang 2013/May/15
			cp = &buf[17];
			cp += strspn( cp, " \t" );
			hc->hdrhost = cp;
			cp = strchr( hc->hdrhost, ']' );
			if ( cp != (char*) 0) { // IPv6
				*(cp+1) = '\0';
			} else {
				cp = strchr( hc->hdrhost, ':' );
				if ( cp != (char*) 0 )
					*cp = '\0';
			}
			cp = strchr( hc->hdrhost, ',' );
			if ( cp != (char*) 0) {
				*(cp) = '\0';
			}
		}
#ifdef LOG_UNKNOWN_HEADERS
	    else if ( strncasecmp( buf, "Accept-Charset:", 15 ) == 0 ||
		      strncasecmp( buf, "Agent:", 6 ) == 0 ||
		      strncasecmp( buf, "Cache-Control:", 14 ) == 0 ||
		      strncasecmp( buf, "Cache-Info:", 11 ) == 0 ||
		      strncasecmp( buf, "Charge-To:", 10 ) == 0 ||
		      strncasecmp( buf, "Client-ip:", 10 ) == 0 ||
		      strncasecmp( buf, "Date:", 5 ) == 0 ||
		      strncasecmp( buf, "Extension:", 10 ) == 0 ||
		      strncasecmp( buf, "Forwarded:", 10 ) == 0 ||
		      strncasecmp( buf, "From:", 5 ) == 0 ||
		      strncasecmp( buf, "HTTP-Version:", 13 ) == 0 ||
		      strncasecmp( buf, "Max-Forwards:", 13 ) == 0 ||
		      strncasecmp( buf, "Message-ID:", 11 ) == 0 ||
		      strncasecmp( buf, "MIME-Version:", 13 ) == 0 ||
		      strncasecmp( buf, "Negotiate:", 10 ) == 0 ||
		      strncasecmp( buf, "Pragma:", 7 ) == 0 ||
		      strncasecmp( buf, "Proxy-agent:", 12 ) == 0 ||
		      strncasecmp( buf, "Proxy-Connection:", 17 ) == 0 ||
		      strncasecmp( buf, "Security-Scheme:", 16 ) == 0 ||
		      strncasecmp( buf, "Session-ID:", 11 ) == 0 ||
		      strncasecmp( buf, "UA-color:", 9 ) == 0 ||
		      strncasecmp( buf, "UA-CPU:", 7 ) == 0 ||
		      strncasecmp( buf, "UA-Disp:", 8 ) == 0 ||
		      strncasecmp( buf, "UA-OS:", 6 ) == 0 ||
		      strncasecmp( buf, "UA-pixels:", 10 ) == 0 ||
		      strncasecmp( buf, "User:", 5 ) == 0 ||
		      strncasecmp( buf, "Via:", 4 ) == 0 ||
		      strncasecmp( buf, "X-", 2 ) == 0 )
		; /* ignore */
	    else
		syslog( LOG_DEBUG, "unknown request header: %.80s", buf );
#endif /* LOG_UNKNOWN_HEADERS */
	    }
	}

    if ( hc->one_one )
	{
	/* Check that HTTP/1.1 requests specify a host, as required. */
	if ( hc->reqhost[0] == '\0' && hc->hdrhost[0] == '\0' )
	    {
	    httpd_send_err( hc, 400, httpd_err400title, "", httpd_err400form, "" );
	    return -1;
	    }

	/* If the client wants to do keep-alives, it might also be doing
	** pipelining.  There's no way for us to tell.  Since we don't
	** implement keep-alives yet, if we close such a connection there
	** might be unread pipelined requests waiting.  So, we have to
	** do a lingering close.
	*/
	if ( hc->keep_alive )
	    hc->should_linger = 1;
	}

    /* Ok, the request has been parsed.  Now we resolve stuff that
    ** may require the entire request.
    */

    /* Copy original filename to expanded filename. */
    httpd_realloc_str(
	&hc->expnfilename, &hc->maxexpnfilename, strlen( hc->origfilename ) );
    (void) strcpy( hc->expnfilename, hc->origfilename );

    /* Tilde mapping. */
    if ( hc->expnfilename[0] == '~' )
	{
#ifdef TILDE_MAP_1
	if ( ! tilde_map_1( hc ) )
	    {
//	    httpd_send_err( hc, 404, err404title, "", err404form, hc->encodedurl );
//Weak point fix: modified by KenChen@QNAP 
	    httpd_send_err( hc, 404, err404title, "", err404form, "" );
	    return -1;
	    }
#endif /* TILDE_MAP_1 */
#ifdef TILDE_MAP_2
	if ( ! tilde_map_2( hc ) )
	    {
	    //httpd_send_err( hc, 404, err404title, "", err404form, hc->encodedurl );
//Weak point fix: modified by KenChen@QNAP 
	    httpd_send_err( hc, 404, err404title, "", err404form, "" );
	    return -1;
	    }
#endif /* TILDE_MAP_2 */
	}

    /* Virtual host mapping. */
    if ( hc->hs->vhost )
	if ( ! vhost_map( hc ) )
	    {
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    return -1;
	    }

    /* Expand all symbolic links in the filename.  This also gives us
    ** any trailing non-existing components, for pathinfo.
    */
    cp = expand_symlinks( hc->expnfilename, &pi, hc->hs->no_symlink, hc->tildemapped );
    if ( cp == (char*) 0 )
	{
	httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	return -1;
	}
    httpd_realloc_str( &hc->expnfilename, &hc->maxexpnfilename, strlen( cp ) );
    (void) strcpy( hc->expnfilename, cp );
    httpd_realloc_str( &hc->pathinfo, &hc->maxpathinfo, strlen( pi ) );
    (void) strcpy( hc->pathinfo, pi );

    /* Remove pathinfo stuff from the original filename too. */
    if ( hc->pathinfo[0] != '\0' )
	{
	int i;
	i = strlen( hc->origfilename ) - strlen( hc->pathinfo );
	if ( i > 0 && strcmp( &hc->origfilename[i], hc->pathinfo ) == 0 )
	    hc->origfilename[i - 1] = '\0';
	}

    /* If the expanded filename is an absolute path, check that it's still
    ** within the current directory or the alternate directory.
    */
    if ( hc->expnfilename[0] == '/' )
	{
	if ( strncmp(
		 hc->expnfilename, hc->hs->cwd, strlen( hc->hs->cwd ) ) == 0 )
	    {
	    /* Elide the current directory. */
	    (void) strcpy(
		hc->expnfilename, &hc->expnfilename[strlen( hc->hs->cwd )] );
	    }
#ifdef TILDE_MAP_2
	else if ( hc->altdir[0] != '\0' &&
		  ( strncmp(
		       hc->expnfilename, hc->altdir,
		       strlen( hc->altdir ) ) == 0 &&
		    ( hc->expnfilename[strlen( hc->altdir )] == '\0' ||
		      hc->expnfilename[strlen( hc->altdir )] == '/' ) ) )
	    {}
#endif /* TILDE_MAP_2 */
	else
	    {
//===============================================================
//	modify by Kent 2001/06/13
//	to prevent 403 error
//===============================================================
//kentprintf(hc->encodedurl);
return 0;
	    syslog(
		LOG_NOTICE, "%.80s URL \"%.80s\" goes outside the web tree",
		httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	    httpd_send_err(
		hc, 403, err403title, "",
		ERROR_FORM( err403form, "The requested URL '%.80s' resolves to a file outside the permitted web server directory tree.\n" ),
		hc->encodedurl );
	    return -1;
	    }
	}
    return 0;
    }


static char*
bufgets( httpd_conn* hc )
    {
    int i;
    char c;

    for ( i = hc->checked_idx; hc->checked_idx < hc->read_idx; ++hc->checked_idx )
	{
	c = hc->read_buf[hc->checked_idx];
	if ( c == '\n' || c == '\r' )
	    {
	    hc->read_buf[hc->checked_idx] = '\0';
	    ++hc->checked_idx;
	    if ( c == '\r' && hc->checked_idx < hc->read_idx &&
		 hc->read_buf[hc->checked_idx] == '\n' )
		{
		hc->read_buf[hc->checked_idx] = '\0';
		++hc->checked_idx;
		}
	    return &(hc->read_buf[i]);
	    }
	}
    return (char*) 0;
    }


static void
de_dotdot( char* file )
    {
    char* cp;
    char* cp2;
    int l;

    /* Collapse any multiple / sequences. */
    while ( ( cp = strstr( file, "//") ) != (char*) 0 )
	{
	for ( cp2 = cp + 2; *cp2 == '/'; ++cp2 )
	    continue;
	(void) strcpy( cp + 1, cp2 );
	}

    /* Elide any xxx/../ sequences. */
    while ( ( cp = strstr( file, "/../" ) ) != (char*) 0 )
	{
	for ( cp2 = cp - 1; cp2 >= file && *cp2 != '/'; --cp2 )
	    continue;
	if ( cp2 < file )
	    break;
	(void) strcpy( cp2, cp + 3 );
	}

    /* Also elide any xxx/.. at the end. */
    while ( ( l = strlen( file ) ) > 3 &&
	    strcmp( ( cp = file + l - 3 ), "/.." ) == 0 )
	{
	for ( cp2 = cp - 1; cp2 >= file && *cp2 != '/'; --cp2 )
	    continue;
	if ( cp2 < file )
	    break;
	*cp2 = '\0';
	}
    }


void
httpd_close_conn( httpd_conn* hc, struct timeval* nowP )
    {
    if ( hc->file_address != (char*) 0 )
	{
	mmc_unmap( hc->file_address, nowP, &(hc->sb) );
	hc->file_address = (char*) 0;
	}
    if ( hc->conn_fd >= 0 )
	{
	(void) close( hc->conn_fd );
	hc->conn_fd = -1;
#if defined(NVR_DEBUG)
    deb_log( ACT_CLOSE, hc, -1 );
#endif
	}
    }

void
httpd_destroy_conn( httpd_conn* hc )
    {
    if ( hc->initialized )
	{
	free( (void*) hc->read_buf );
	free( (void*) hc->decodedurl );
	free( (void*) hc->origfilename );
	free( (void*) hc->expnfilename );
	free( (void*) hc->encodings );
	free( (void*) hc->pathinfo );
	free( (void*) hc->query );
	free( (void*) hc->accept );
	free( (void*) hc->accepte );
	free( (void*) hc->reqhost );
	free( (void*) hc->hostdir );
	free( (void*) hc->remoteuser );
	free( (void*) hc->response );
#ifdef TILDE_MAP_2
	free( (void*) hc->altdir );
#endif /* TILDE_MAP_2 */
	hc->initialized = 0;
	}
    }


/* Figures out MIME encodings and type based on the filename.  Multiple
** encodings are separated by semicolons.
*/
static void
figure_mime( httpd_conn* hc )
    {
    int i, j, k, l;
    int got_enc;
    struct table {
	char* ext;
	char* val;
	};
    static struct table enc_tab[] = {
#include "mime_encodings.h"
	};
    static struct table typ_tab[] = {
#include "mime_types.h"
	};

    /* Look at the extensions on hc->expnfilename from the back forwards. */
    i = strlen( hc->expnfilename );
    for (;;)
	{
	j = i;
	for (;;)
	    {
	    --i;
	    if ( i <= 0 )
		{
		/* No extensions left. */
		hc->type = "text/plain; charset=%s";
		return;
		}
	    if ( hc->expnfilename[i] == '.' )
		break;
	    }
	/* Found an extension. */
	got_enc = 0;
	for ( k = 0; k < sizeof(enc_tab)/sizeof(*enc_tab); ++k )
	    {
	    l = strlen( enc_tab[k].ext );
	    if ( l == j - i - 1 &&
		 strncasecmp( &hc->expnfilename[i+1], enc_tab[k].ext, l ) == 0 )
		{
		httpd_realloc_str(
		    &hc->encodings, &hc->maxencodings,
		    strlen( enc_tab[k].val ) + 1 );
		if ( hc->encodings[0] != '\0' )
		    (void) strcat( hc->encodings, ";" );
		(void) strcat( hc->encodings, enc_tab[k].val );
		got_enc = 1;
		}
	    }
	if ( ! got_enc )
	    {
	    /* No encoding extension found - time to try type extensions. */
	    for ( k = 0; k < sizeof(typ_tab)/sizeof(*typ_tab); ++k )
		{
		l = strlen( typ_tab[k].ext );
		if ( l == j - i - 1 &&
		     strncasecmp(
			 &hc->expnfilename[i+1], typ_tab[k].ext, l ) == 0 )
		    {
		    hc->type = typ_tab[k].val;
		    return;
		    }
		}
	    /* No recognized type extension found - return default. */
	    hc->type = "text/plain; charset=%s";
	    return;
	    }
	}
    }


#ifdef CGI_TIMELIMIT
static void
cgi_kill2( ClientData client_data, struct timeval* nowP )
    {
    pid_t pid;

    /* Before trying to kill the CGI process, reap any zombie processes.
    ** That may get rid of the CGI process.
    */
    (void) do_reap();

    pid = (pid_t) client_data.i;
    if ( kill( pid, SIGKILL ) == 0 )
	syslog( LOG_ERR, "hard-killed CGI process %d", pid );
    }

static int get_parent_pid( int pid, char *name )
{
	FILE *file;
	char fname[64], buf[1024], *p, *ptr;
	if( pid==1 )return -1;
	sprintf( fname, "/proc/%d/stat", pid );
	file=fopen( fname, "r");
	if(!file)return -2;
	if(!fgets( buf, 1024-1, file))
	{
		fclose(file);
		return -3;
	}
	p=strtok_r( buf, " ", &ptr); // pid
	if(p)
	{
		p=strtok_r( NULL, " ", &ptr); // process name
		if( p )
		{
			if( name )
				strcpy(name, p);
			p=strtok_r( NULL, " ", &ptr); // state
			if(p)
			{
				p=strtok_r( NULL, " ", &ptr); // parent pid
				if(p)
				{
					pid=atoi(p);
					fclose(file);
					return pid;
				}
			}
		}
	}
	fclose(file);
	return -4;
}
static void
cgi_kill( ClientData client_data, struct timeval* nowP )
    {
    pid_t pid, ppid;

    /* Before trying to kill the CGI process, reap any zombie processes.
    ** That may get rid of the CGI process.
    */
    (void) do_reap();

    pid = (pid_t) client_data.i;
    ppid=get_parent_pid( pid, NULL );
    if( ppid!=getpid())return;
    if ( kill( pid, SIGINT ) == 0 )
	{
	syslog( LOG_ERR, "killed CGI process %d", pid );
	/* In case this isn't enough, schedule an uncatchable kill. */
	(void) tmr_create( nowP, cgi_kill2, client_data, 5 * 1000L, 0 );
	}
    }
#endif /* CGI_TIMELIMIT */


#ifdef GENERATE_INDEXES

/* qsort comparison routine - declared old-style on purpose, for portability. */
static int
name_compare( a, b )
    char** a;
    char** b;
    {
    return strcmp( *a, *b );
    }


static off_t
ls( httpd_conn* hc )
    {
    DIR* dirp;
    struct dirent* de;
    int namlen;
    static int maxnames = 0;
    int nnames;
    static char* names;
    static char** nameptrs;
    static char* name;
    static int maxname = 0;
    static char* rname;
    static int maxrname = 0;
    static char* encrname;
    static int maxencrname = 0;
    FILE* fp;
    int i, r;
    struct stat sb;
    struct stat lsb;
    char modestr[20];
    char* linkprefix;
    char link[MAXPATHLEN];
    int linklen;
    char* fileclass;
    time_t now;
    char* timestr;
    ClientData client_data;

    dirp = opendir( hc->expnfilename );
    if ( dirp == (DIR*) 0 )
	{
	syslog( LOG_ERR, "opendir %.80s - %m", hc->expnfilename );
	//httpd_send_err( hc, 404, err404title, "", err404form, hc->encodedurl );
//Weak point fix: modified by KenChen@QNAP 
	    httpd_send_err( hc, 404, err404title, "", err404form, "" );
	return -1;
	}

    send_mime( hc, 200, ok200title, "", "", "text/html", -1, hc->sb.st_mtime );
    hc->bytes = 0;
    if ( hc->method == METHOD_HEAD )
	closedir( dirp );
    else if ( hc->method == METHOD_GET )
	{
	httpd_write_response( hc );
	r = fork( );
	if ( r < 0 )
	    {
	    syslog( LOG_ERR, "fork - %m" );
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    return -1;
	    }
	if ( r == 0 )
	    {
	    /* Child process. */

#ifdef CGI_NICE
	    /* Set priority. */
	    (void) nice( CGI_NICE );
#endif /* CGI_NICE */

	    /* Open a stdio stream so that we can use fprintf, which is more
	    ** efficient than a bunch of separate write()s.  We don't have
	    ** to worry about double closes or file descriptor leaks cause
	    ** we're in a subprocess.
	    */
	    fp = fdopen( hc->conn_fd, "w" );
	    if ( fp == (FILE*) 0 )
		{
		syslog( LOG_ERR, "fdopen - %m" );
		httpd_send_err(
		    hc, 500, err500title, "", err500form, hc->encodedurl );
		closedir( dirp );
		exit( 1 );
		}

	    (void) fprintf( fp, "<HTML><HEAD><TITLE>Index of %.80s</TITLE></HEAD>\n" \
		"<BODY BGCOLOR=\"#99cc99\">\n<H2>Index of %.80s</H2>\n" \
		"<PRE>\nmode  links  bytes  last-changed  name\n<HR>", hc->encodedurl, hc->encodedurl );

	    /* Read in names. */
	    nnames = 0;
	    while ( ( de = readdir( dirp ) ) != 0 )     /* dirent or direct */
		{
		if ( nnames >= maxnames )
		    {
		    if ( maxnames == 0 )
			{
			maxnames = 100;
			names = NEW( char, maxnames * MAXPATHLEN );
			nameptrs = NEW( char*, maxnames );
			}
		    else
			{
			maxnames *= 2;
			names = RENEW( names, char, maxnames * MAXPATHLEN );
			nameptrs = RENEW( nameptrs, char*, maxnames );
			}
		    if ( names == (char*) 0  || nameptrs == (char**) 0 )
			{
			syslog( LOG_ERR, "out of memory" );
			exit( 1 );
			}
		    for ( i = 0; i < maxnames; ++i )
			nameptrs[i] = &names[i * MAXPATHLEN];
		    }
		namlen = NAMLEN(de);
		(void) strncpy( nameptrs[nnames], de->d_name, namlen );
		nameptrs[nnames][namlen] = '\0';
		++nnames;
		}
	    closedir( dirp );

	    /* Sort the names. */
	    qsort( nameptrs, nnames, sizeof(*nameptrs), name_compare );

	    /* Generate output. */
	    for ( i = 0; i < nnames; ++i )
		{
		httpd_realloc_str(
		    &name, &maxname,
		    strlen( hc->expnfilename ) + 1 + strlen( nameptrs[i] ) );
		httpd_realloc_str(
		    &rname, &maxrname,
		    strlen( hc->origfilename ) + 1 + strlen( nameptrs[i] ) );
		if ( hc->expnfilename[0] == '\0' ||
		     strcmp( hc->expnfilename, "." ) == 0 )
		    {
		    (void) strcpy( name, nameptrs[i] );
		    (void) strcpy( rname, nameptrs[i] );
		    }
		else
		    {
		    (void) my_snprintf( name, maxname,
			"%s/%s", hc->expnfilename, nameptrs[i] );
		    if ( strcmp( hc->origfilename, "." ) == 0 )
			(void) my_snprintf( rname, maxrname,
			    "%s", nameptrs[i] );
		    else
			(void) my_snprintf( rname, maxrname,
			    "%s%s", hc->origfilename, nameptrs[i] );
		    }
		httpd_realloc_str(
		    &encrname, &maxencrname, 3 * strlen( rname ) + 1 );
		strencode( encrname, maxencrname, rname );

		if ( stat( name, &sb ) < 0 || lstat( name, &lsb ) < 0 )
		    continue;

		linkprefix = "";
		link[0] = '\0';
		/* Break down mode word.  First the file type. */
		switch ( lsb.st_mode & S_IFMT )
		    {
		    case S_IFIFO:  modestr[0] = 'p'; break;
		    case S_IFCHR:  modestr[0] = 'c'; break;
		    case S_IFDIR:  modestr[0] = 'd'; break;
		    case S_IFBLK:  modestr[0] = 'b'; break;
		    case S_IFREG:  modestr[0] = '-'; break;
		    case S_IFSOCK: modestr[0] = 's'; break;
		    case S_IFLNK:  modestr[0] = 'l';
		    linklen = readlink( name, link, sizeof(link) );
		    if ( linklen != -1 )
			{
			link[linklen] = '\0';
			linkprefix = " -> ";
			}
		    break;
		    default:       modestr[0] = '?'; break;
		    }
		/* Now the world permissions.  Owner and group permissions
		** are not of interest to web clients.
		*/
		modestr[1] = ( lsb.st_mode & S_IROTH ) ? 'r' : '-';
		modestr[2] = ( lsb.st_mode & S_IWOTH ) ? 'w' : '-';
		modestr[3] = ( lsb.st_mode & S_IXOTH ) ? 'x' : '-';
		modestr[4] = '\0';

		/* We also leave out the owner and group name, they are
		** also not of interest to web clients.  Plus if we're
		** running under chroot(), they would require a copy
		** of /etc/passwd and /etc/group, which we want to avoid.
		*/

		/* Get time string. */
		now = time( (time_t*) 0 );
		timestr = ctime( &lsb.st_mtime );
		timestr[ 0] = timestr[ 4];
		timestr[ 1] = timestr[ 5];
		timestr[ 2] = timestr[ 6];
		timestr[ 3] = ' ';
		timestr[ 4] = timestr[ 8];
		timestr[ 5] = timestr[ 9];
		timestr[ 6] = ' ';
		if ( now - lsb.st_mtime > 60*60*24*182 )        /* 1/2 year */
		    {
		    timestr[ 7] = ' ';
		    timestr[ 8] = timestr[20];
		    timestr[ 9] = timestr[21];
		    timestr[10] = timestr[22];
		    timestr[11] = timestr[23];
		    }
		else
		    {
		    timestr[ 7] = timestr[11];
		    timestr[ 8] = timestr[12];
		    timestr[ 9] = ':';
		    timestr[10] = timestr[14];
		    timestr[11] = timestr[15];
		    }
		timestr[12] = '\0';

		/* The ls -F file class. */
		switch ( sb.st_mode & S_IFMT )
		    {
		    case S_IFDIR:  fileclass = "/"; break;
		    case S_IFSOCK: fileclass = "="; break;
		    case S_IFLNK:  fileclass = "@"; break;
		    default:
		    fileclass = ( sb.st_mode & S_IXOTH ) ? "*" : "";
		    break;
		    }

		/* And print. */
		(void)  fprintf( fp,
		   "%s %3ld  %8ld  %s  <A HREF=\"/%.500s%s\">%.80s</A>%s%s%s\n",
		    modestr, (long) lsb.st_nlink, (long) lsb.st_size, timestr,
		    encrname, S_ISDIR(sb.st_mode) ? "/" : "",
		    nameptrs[i], linkprefix, link, fileclass );
		}

	    (void) fprintf( fp, "</PRE></BODY></HTML>\n" );
	    (void) fclose( fp );
	    exit( 0 );
	    }

	/* Parent process. */
	closedir( dirp );
	syslog( LOG_INFO, "spawned indexing process %d for directory '%.200s'", r, hc->expnfilename );
#ifdef CGI_TIMELIMIT
	/* Schedule a kill for the child process, in case it runs too long */
	client_data.i = r;
	(void) tmr_create(
	    (struct timeval*) 0, cgi_kill, client_data, CGI_TIMELIMIT * 1000L,
	    0 );
#endif /* CGI_TIMELIMIT */
	hc->status = 200;
	hc->bytes = CGI_BYTECOUNT;
	hc->should_linger = 0;
	}
    else
	{
	httpd_send_err(
	    hc, 501, err501title, "", err501form, httpd_method_str( hc->method ) );
	return -1;
	}

    return 0;
    }

#endif /* GENERATE_INDEXES */


static char*
build_env( char* fmt, char* arg )
    {
    char* cp;
    int size;
    static char* buf;
    static int maxbuf = 0;
//Vulnerability (CVE-2014-6271): patch by KenChen@QNAP 2014/09/26
	if(arg && !strncmp("() {", arg, 4)){
		syslog( LOG_ERR, "invalid argument" );
		exit( 1 );
	}
//end here 2014/09/26
    size = strlen( fmt ) + strlen( arg );
    if ( size > maxbuf )
	httpd_realloc_str( &buf, &maxbuf, size );
    (void) my_snprintf( buf, maxbuf,
	fmt, arg );
    cp = strdup( buf );
    if ( cp == (char*) 0 )
	{
	syslog( LOG_ERR, "out of memory" );
	exit( 1 );
	}
    return cp;
    }


#ifdef SERVER_NAME_LIST
static char*
hostname_map( char* hostname )
    {
    int len, n;
    static char* list[] = { SERVER_NAME_LIST };

    len = strlen( hostname );
    for ( n = sizeof(list) / sizeof(*list) - 1; n >= 0; --n )
	if ( strncasecmp( hostname, list[n], len ) == 0 )
	    if ( list[n][len] == '/' )  /* check in case of a substring match */
		return &list[n][len + 1];
    return (char*) 0;
    }
#endif /* SERVER_NAME_LIST */

#ifdef QTS_HA
static int
check_cluster(char *query_string, char *idx_str)
{
	int idx = 0;
	char *delim="&";
	char *pch;
	pch = strtok(query_string,delim);
	while (pch != NULL){
		if(!strcmp(pch,"ha_cluster=clusterA")){
			sprintf(idx_str,"1");
			idx =1;
			break;
		}
		else if (!strcmp(pch,"ha_cluster=clusterB")){
			sprintf(idx_str,"2");
			idx=2;
			break;
		}else
			idx = 0;
		pch = strtok(NULL, delim);
	}

	return idx;
}
#endif

/* Set up environment variables. Be real careful here to avoid
** letting malicious clients overrun a buffer.  We don't have
** to worry about freeing stuff since we're a sub-process.
*/
static char**
make_envp( httpd_conn* hc )
    {
    static char* envp[50];
    int envn;
    char* cp;
    char buf[256];

#ifdef QTS_HA
	int cluster_idx = 0;
	char idx_str[2];
#endif

    envn = 0;
    envp[envn++] = build_env( "PATH=%s", CGI_PATH );
#ifdef CGI_LD_LIBRARY_PATH
    envp[envn++] = build_env( "LD_LIBRARY_PATH=%s", CGI_LD_LIBRARY_PATH );
#endif /* CGI_LD_LIBRARY_PATH */
    envp[envn++] = build_env( "SERVER_SOFTWARE=%s", SERVER_SOFTWARE );
    /* If vhosting, use that server-name here. */
    if ( hc->hs->vhost && hc->hostname != (char*) 0 )
	cp = hc->hostname;
    else
	cp = hc->hs->server_hostname;
    if ( cp != (char*) 0 )
	envp[envn++] = build_env( "SERVER_NAME=%s", cp );
    envp[envn++] = "GATEWAY_INTERFACE=CGI/1.1";
    envp[envn++] = build_env("SERVER_PROTOCOL=%s", hc->protocol);
    (void) my_snprintf( buf, sizeof(buf),
	"%d", hc->hs->port );
    envp[envn++] = build_env( "SERVER_PORT=%s", buf );
    envp[envn++] = build_env(
	"REQUEST_METHOD=%s", httpd_method_str( hc->method ) );
    if ( hc->pathinfo[0] != '\0' )
	{
	char* cp2;
	int l;
	envp[envn++] = build_env( "PATH_INFO=/%s", hc->pathinfo );
	l = strlen( hc->hs->cwd ) + strlen( hc->pathinfo ) + 1;
	cp2 = NEW( char, l );
	if ( cp2 != (char*) 0 )
	    {
	    (void) my_snprintf( cp2, l,
		"%s%s", hc->hs->cwd, hc->pathinfo );
	    envp[envn++] = build_env( "PATH_TRANSLATED=%s", cp2 );
	    }
	}
    envp[envn++] = build_env( "SCRIPT_NAME=/%s", hc->origfilename );
    if ( hc->query[0] != '\0')
    //QUERY_STRING URLdecoding, modified by Ken Chen 2005/09/22
	envp[envn++] = build_env( "QUERY_STRING=%s", hc->query );
#ifdef QTS_HA
	if (check_cluster(hc->query, idx_str)){
		envp[envn++] = build_env("HA_CLUSTER=%s", idx_str );
	}
#endif
	//envp[envn++] = build_env( "QUERY_STRING=%s", URLdecode_to_UTF8(hc->query) );
	//2010-01-29 Richard Chang add for stunnel send source ip
	if(hc->source_ip[0] != '\0'){
		envp[envn++] = build_env("REMOTE_ADDR=%s", hc->source_ip);
	}
	else{
    envp[envn++] = build_env(
	"REMOTE_ADDR=%s", httpd_ntoa( &hc->client_addr ) );
	}
    if ( hc->referer[0] != '\0' )
	envp[envn++] = build_env( "HTTP_REFERER=%s", hc->referer );
    if ( hc->useragent[0] != '\0' )
	envp[envn++] = build_env( "HTTP_USER_AGENT=%s", hc->useragent );
    if ( hc->accept[0] != '\0' )
	envp[envn++] = build_env( "HTTP_ACCEPT=%s", hc->accept );
    if ( hc->accepte[0] != '\0' )
	envp[envn++] = build_env( "HTTP_ACCEPT_ENCODING=%s", hc->accepte );
    if ( hc->cookie[0] != '\0' )
	envp[envn++] = build_env( "HTTP_COOKIE=%s", hc->cookie );
    if ( hc->contenttype[0] != '\0' )
	envp[envn++] = build_env( "CONTENT_TYPE=%s", hc->contenttype );
    if ( hc->hdrhost[0] != '\0' )
	envp[envn++] = build_env( "HTTP_HOST=%s", hc->hdrhost );
    if ( hc->contentlength != -1 )
	{
	(void) my_snprintf( buf, sizeof(buf),
	    "%lld", (long long) hc->contentlength ); //KenChen@QNAP long => long long
	envp[envn++] = build_env( "CONTENT_LENGTH=%s", buf );
	}
    if ( hc->remoteuser[0] != '\0' )
	envp[envn++] = build_env( "REMOTE_USER=%s", hc->remoteuser );
    if ( hc->authorization[0] == '\0' )
	envp[envn++] = build_env( "AUTH_TYPE=%s", "Basic" );
    envp[envn++] = build_env( "REMOTE_PWD=%s", hc->authorization );
	/* We only support Basic auth at the moment. */
    if ( getenv( "TZ" ) != (char*) 0 )
	envp[envn++] = build_env( "TZ=%s", getenv( "TZ" ) );
    envp[envn++] = build_env( "CGI_PATTERN=%s", hc->hs->cgi_pattern );

	// peterlai add for WFM2 download function of safari
    if ( hc->got_range == 1) {
		if(hc->end_byte_loc>0)
			(void) my_snprintf( buf, sizeof(buf), "bytes=%lld-%lld", hc->init_byte_loc, hc->end_byte_loc);
		else
			(void) my_snprintf( buf, sizeof(buf), "bytes=%lld-", hc->init_byte_loc, hc->end_byte_loc);
		envp[envn++] = build_env( "HTTP_RANGE=Range: %s", buf);
	}
	if ( hc->accept_language[0] != '\0' )
	envp[envn++] = build_env( "ACCEPT_LANGUAGE=%s", hc->accept_language );
    if ( hc->x_apache_proxy[0] != '\0' )
        envp[envn++] = build_env( "X_APACHE_PROXY=%s", hc->x_apache_proxy );
//===Add IF_MODIFIED_SINCE 20130204 KenChen ===
	if ( hc->if_modified_since > 0 ){
		(void) my_snprintf( buf, sizeof(buf), "%lu", (unsigned long)hc->if_modified_since);
		envp[envn++] = build_env( "IF_MODIFIED_SINCE=%s",  buf);
	}
//===
#ifdef RECYCLE_EX
    if (strstr(hc->encodedurl, "/cgi-bin/filemanager/utilRequest.cgi")){
	if(Is_Recycle_Bin_File_Flag_Enabled()){
	    //FILE* fp;
	    //fp = fopen("/tmp/aix", "a+");
	    //fprintf(fp, "%s\n", hc->encodedurl);

	    //fclose(fp);
	    envp[envn++] = build_env( "LD_PRELOAD=%s", "/usr/local/lib/libtrash.so" );
	}
    }
#endif
    
    envp[envn] = (char*) 0;
    return envp;
    }


/* Set up argument vector.  Again, we don't have to worry about freeing stuff
** since we're a sub-process.  This gets done after make_envp() because we
** scribble on hc->query.
*/
static char**
make_argp( httpd_conn* hc )
    {
    char** argp;
    int argn;
    char* cp1;
    char* cp2;

    /* By allocating an arg slot for every character in the query, plus
    ** one for the filename and one for the NULL, we are guaranteed to
    ** have enough.  We could actually use strlen/2.
    */
    argp = NEW( char*, strlen( hc->query ) + 2 );
    if ( argp == (char**) 0 )
	return (char**) 0;

    argp[0] = strrchr( hc->expnfilename, '/' );
    if ( argp[0] != (char*) 0 )
	++argp[0];
    else
	argp[0] = hc->expnfilename;

    argn = 1;
    /* According to the CGI spec at http://hoohoo.ncsa.uiuc.edu/cgi/cl.html,
    ** "The server should search the query information for a non-encoded =
    ** character to determine if the command line is to be used, if it finds
    ** one, the command line is not to be used."
    */
    if ( strchr( hc->query, '=' ) == (char*) 0 )
	{
	for ( cp1 = cp2 = hc->query; *cp2 != '\0'; ++cp2 )
	    {
	    if ( *cp2 == '+' )
		{
		*cp2 = '\0';
		strdecode( cp1, cp1 );
		argp[argn++] = cp1;
		cp1 = cp2 + 1;
		}
	    }
	if ( cp2 != cp1 )
	    {
	    strdecode( cp1, cp1 );
	    argp[argn++] = cp1;
	    }
	}

    argp[argn] = (char*) 0;
    return argp;
    }

// Kevin 2001-07-31: Update from v2.21b source code...
/* Special hack to deal with broken browsers that send a LF or CRLF
** after POST data, causing TCP resets - we just read and discard up
** to 2 bytes.  Unfortunately this doesn't fix the problem for CGIs
** which avoid the interposer process due to their POST data being
** short.  Creating an interposer process for all POST CGIs is
** unacceptably expensive.  The eventual fix will come when interposing
** gets integrated into the main loop as a tasklet instead of a process.
*/
static void
post_post_garbage_hack( httpd_conn* hc )
    {
    char buf[2];
// Kevin 2001-07-31: Patch from Sascha Schumann <sascha@schumann.cx>...
//    fcntl(hc->conn_fd, F_SETFL, O_NONBLOCK);
//    (void) read( hc->conn_fd, buf, 2 );
// Kent 2003/02/07
	httpd_set_ndelay( hc->conn_fd );
	(void) read( hc->conn_fd, buf, sizeof(buf));
// end
/*
    int r;

    r = recv( hc->conn_fd, buf, sizeof(buf), MSG_PEEK );
    if ( r > 0 )
        (void) read( hc->conn_fd, buf, r );
*/
    }
// Kevin 2001-07-31: End of patch
// Kevin 2001-07-31: End of update


/* This routine is used only for POST requests.  It reads the data
** from the request and sends it to the child process.  The only reason
** we need to do it this way instead of just letting the child read
** directly is that we have already read part of the data into our
** buffer.
*/
static void
cgi_interpose_input( httpd_conn* hc, int wfd )
    {
    //int c, r, flags; //KenChen@QNAP
    int r, flags;
    long long c;
    char buf[1024];

    c = hc->read_idx - hc->checked_idx;
    if ( c > 0 )
	{
// modify by Kent 2003/02/07
	if ( httpd_write_fully( wfd, &(hc->read_buf[hc->checked_idx]), c ) != c )
//	if ( write( wfd, &(hc->read_buf[hc->checked_idx]), c ) != c )
// end
	    return;
	}
    while ( c < hc->contentlength )
	{
	r = read( hc->conn_fd, buf, MIN( sizeof(buf), hc->contentlength - c ) );
	if ( r == 0 )
// modify by Kent 2003/02/07
	    return;
//	    sleep( 1 );
// end
	else if ( r < 0 )
	    {
	    if ( errno == EAGAIN )
		sleep( 1 );
	    else
		return;
	    }
	else
	    {
// modify by Kent 2003/02/07
	    if ( httpd_write_fully( wfd, buf, r ) != r )
//	    if ( write( wfd, buf, r ) != r )
// end
		return;
	    c += r;
	    }
	}

/* add by Kent 2001/12/03			*/
/* prevent the name of object too long...	*/
      if (	strstr(hc->encodedurl, "cgi-bin/filemanager/filemanager.cgi") || 
		strstr(hc->encodedurl, "cgi-bin/filemanager/function.cgi"))
	 post_post_garbage_hack( hc );
    }


/* This routine is used for parsed-header CGIs.  The idea here is that the
** CGI can return special headers such as "Status:" and "Location:" which
** change the return status of the response.  Since the return status has to
** be the very first line written out, we have to accumulate all the headers
** and check for the special ones before writing the status.  Then we write
** out the saved headers and proceed to echo the rest of the response.
*/
static void
cgi_interpose_output( httpd_conn* hc, int rfd )
    {
    int r;
    char buf[1024];
    int headers_size, headers_len;
    char* headers;
    char* br;
    int status;
    char* title;
    char* cp;

    /* Slurp in all headers. */
    headers_size = 0;
    httpd_realloc_str( &headers, &headers_size, 500 );
    headers_len = 0;
    for (;;)
	{
	r = read( rfd, buf, sizeof(buf) );
	if ( r <= 0 )
	    {
	    br = &(headers[headers_len]);
	    break;
	    }
	httpd_realloc_str( &headers, &headers_size, headers_len + r );
	(void) memcpy( &(headers[headers_len]), buf, r );
	headers_len += r;
	headers[headers_len] = '\0';
	if ( ( br = strstr( headers, "\r\n\r\n" ) ) != (char*) 0 ||
	     ( br = strstr( headers, "\n\n" ) ) != (char*) 0 )
	    break;
	}

    /* Figure out the status. */
    status = 200;
    if ( ( cp = strstr( headers, "Status:" ) ) != (char*) 0 &&
	 cp < br &&
	 ( cp == headers || *(cp-1) == '\n' ) )
	{
	cp += 7;
	cp += strspn( cp, " \t" );
	status = atoi( cp );
	}
    if ( ( cp = strstr( headers, "Location:" ) ) != (char*) 0 &&
	 cp < br &&
	 ( cp == headers || *(cp-1) == '\n' ) )
	status = 302;

	// for Partial Content status 206
    if ( ( cp = strstr( headers, "206 Partial Content" ) ) != (char*) 0) {
	status = 206;
	}

    /* Write the status line. */
    switch ( status )
	{
	case 200: title = ok200title; break;
	case 206: title = ok206title; break;
	case 302: title = err302title; break;
	case 304: title = err304title; break;
	case 400: title = httpd_err400title; break;
#ifdef AUTH_FILE
	case 401: title = err401title; break;
#endif /* AUTH_FILE */
	case 403: title = err403title; break;
	case 404: title = err404title; break;
	case 408: title = httpd_err408title; break;
	case 500: title = err500title; break;
	case 501: title = err501title; break;
	case 503: title = httpd_err503title; break;
	case 999: title = httpd_err999title; break;
	default: title = "Something"; break;
	}
    (void) my_snprintf( buf, sizeof(buf), "HTTP/1.1 %d %s\r\n", status, title );
    (void) write( hc->conn_fd, buf, strlen( buf ) );

    /* Write the saved headers. */
    (void) write( hc->conn_fd, headers, headers_len );

    /* Echo the rest of the output. */
    for (;;)
	{
	r = read( rfd, buf, sizeof(buf) );
	if ( r <= 0 )
	    return;
	if ( write( hc->conn_fd, buf, r ) != r )
	    return;
	}
    }


/* CGI child process. */
static void
cgi_child( httpd_conn* hc )
    {
    int r;
    char** argp;
    char** envp;
    char* binary;
    char* directory;

#ifdef HTTP_LIVE_STREAMING
    http_stream_conn_close_all();
#endif
    /* Unset close-on-exec flag for this socket.  This actually shouldn't
    ** be necessary, according to POSIX a dup()'d file descriptor does
    ** *not* inherit the close-on-exec flag, its flag is always clear.
    ** However, Linux messes this up and does copy the flag to the
    ** dup()'d descriptor, so we have to clear it.  This could be
    ** ifdeffed for Linux only.
    */
    
    
    (void) fcntl( hc->conn_fd, F_SETFD, 0 );

    /* Close the syslog descriptor so that the CGI program can't
    ** mess with it.  All other open descriptors should be either
    ** the listen socket(s), sockets from accept(), or the file-logging
    ** fd, and all of those are set to close-on-exec, so we don't
    ** have to close anything else.
    */
    closelog();

    /* If the socket happens to be using one of the stdin/stdout/stderr
    ** descriptors, move it to another descriptor so that the dup2 calls
    ** below don't screw things up.
    */
    
    
    if ( hc->conn_fd == STDIN_FILENO || hc->conn_fd == STDOUT_FILENO || hc->conn_fd == STDERR_FILENO )
	{
	int newfd = dup( hc->conn_fd );

	if ( newfd >= 0 )
	    hc->conn_fd = newfd;
	/* If the dup fails, shrug.  We'll just take our chances.
	** Shouldn't happen though.
	**
	** If the dup happens to produce an fd that is still one of
	** the standard ones, we should be ok - I think it can be
	** fd 2, stderr, but can never show up as 0 or 1 since at
	** least two file descriptors are always in use.  Because
	** of the order in which we dup2 things below - stderr is
	** always done last - it's actually ok for the socket to
	** be fd 2.  It'll just get dup2'd onto itself.
	*/
	}

    /* Make the environment vector. */
    envp = make_envp( hc );

    /* Make the argument vector. */
    argp = make_argp( hc );

    /* Set up stdin.  For POSTs we may have to set up a pipe from an
    ** interposer process, depending on if we've read some of the data
    ** into our buffer.
    */
    if ( hc->method == METHOD_POST && hc->read_idx > hc->checked_idx )
	{
	int p[2];

	if ( pipe( p ) < 0 )
	    {
	    syslog( LOG_ERR, "pipe - %m" );
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    exit( 1 );
	    }
	r = fork( );
	if ( r < 0 )
	    {
	    syslog( LOG_ERR, "fork - %m" );
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    exit( 1 );
	    }
	if ( r == 0 )
	    {
	    /* Interposer process. */
	    (void) close( p[0] );
	    cgi_interpose_input( hc, p[1] );
	    exit( 0 );
	    }
	(void) close( p[1] );
	(void) dup2( p[0], STDIN_FILENO );
	(void) close( p[0] );
	}
    else
	{
	/* Otherwise, the request socket is stdin. */
	(void) dup2( hc->conn_fd, STDIN_FILENO );
	}

    /* Set up stdout/stderr.  If we're doing CGI header parsing,
    ** we need an output interposer too.
    */
    if ( strncmp( argp[0], "nph-", 4 ) != 0 && hc->mime_flag )
	{
	int p[2];

	if ( pipe( p ) < 0 )
	    {
	    syslog( LOG_ERR, "pipe - %m" );
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    exit( 1 );
	    }
	r = fork( );
	if ( r < 0 )
	    {
	    syslog( LOG_ERR, "fork - %m" );
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    exit( 1 );
	    }
	if ( r == 0 )
	    {
	    /* Interposer process. */
	    (void) close( p[1] );
	    cgi_interpose_output( hc, p[0] );
	    exit( 0 );
	    }
	(void) close( p[0] );
	(void) dup2( p[1], STDOUT_FILENO );
	(void) dup2( p[1], STDERR_FILENO );
	(void) close( p[1] );
	}
    else
	{
	/* Otherwise, the request socket is stdout/stderr. */
	(void) dup2( hc->conn_fd, STDOUT_FILENO );
	(void) dup2( hc->conn_fd, STDERR_FILENO );
	}
    /* At this point we would like to set close-on-exec again for hc->conn_fd
    ** (see previous comments on Linux's broken behavior re: close-on-exec
    ** and dup.)  Unfortunately there seems to be another Linux problem, or
    ** perhaps a different aspect of the same problem - if we do this
    ** close-on-exec in Linux, the socket stays open but stderr gets
    ** closed - the last fd duped from the socket.  What a mess.  So we'll
    ** just leave the socket as is, which under other OSs means an extra
    ** file descriptor gets passed to the child process.  Since the child
    ** probably already has that file open via stdin stdout and/or stderr,
    ** this is not a problem.
    */
    /* (void) fcntl( hc->conn_fd, F_SETFD, 1 ); */

#ifdef CGI_NICE
    /* Set priority. */
    (void) nice( CGI_NICE );
#endif /* CGI_NICE */

    /* Split the program into directory and binary, so we can chdir()
    ** to the program's own directory.  This isn't in the CGI 1.1
    ** spec, but it's what other HTTP servers do.
    */
    directory = strdup( hc->expnfilename );
    if ( directory == (char*) 0 )
	binary = hc->expnfilename;      /* ignore errors */
    else
	{
	binary = strrchr( directory, '/' );
	if ( binary == (char*) 0 )
	    binary = hc->expnfilename;
	else
	    {
	    *binary++ = '\0';
	    (void) chdir( directory );  /* ignore errors */
	    }
	}

    /* Default behavior for SIGPIPE. */
    (void) signal( SIGPIPE, SIG_DFL );

    // added by Jeff on 2013/8/5 to avoid passing down the socket to all subprocess
    // that makes it no closed if one of them is a daemon process(see BUG 34709)
    if ( !(hc->conn_fd == STDIN_FILENO || hc->conn_fd == STDOUT_FILENO || hc->conn_fd == STDERR_FILENO ))
        close(hc->conn_fd);
    /* Run the program. */
    (void) execve( binary, argp, envp );


    /* Something went wrong. */
    syslog( LOG_ERR, "execve %s - %m", hc->expnfilename );
    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
    exit( 1 );
    }


static off_t
cgi( httpd_conn* hc )
    {
    int r;
    ClientData client_data;
    char tmpchar;
int	i;
int	chk_timeout=1;
  /* Markde by Ken Chen 20050926  
	//modify by Kent Tsai 2001/02/06
	//check if it is under the cgi-bin folder
	//if not, show the error message
	tmpchar=hc->encodedurl[8];
        hc->encodedurl[8]=0x0;
        if (!strcmp(hc->encodedurl, "/cgi-bin"))
        {
                hc->encodedurl[8]=tmpchar;
        }
	else
	{
        	hc->encodedurl[8]=tmpchar;
		if (hc->referer[0]!=0)
		{	//user want to exec it
			httpd_send_err(hc, 501, httpd_err504title, "", httpd_err504form, hc->encodedurl);
			return -1;
		}
	}*/

    if ( hc->method == METHOD_GET || hc->method == METHOD_POST )
	{
	clear_ndelay( hc );
	r = fork( );
	if ( r < 0 )
	    {
	    syslog( LOG_ERR, "fork - %m" );
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    return -1;
	    }
	if ( r == 0 )
	    cgi_child( hc );

	/* Parent process. */
	syslog( LOG_INFO, "spawned CGI process %d for file '%.200s'", r, hc->expnfilename );
//======================================
//	timeout issue:
//	add by Kent Tsai 2001/03/07
//	if we want to upload file 
//	don't set cgi time limit
//=======================================
#ifdef CGI_TIMELIMIT
        for (i=0;i<cnt_timeout;i++)
        {
                if (strstr(hc->encodedurl, cgi_timeout[i]))
                        chk_timeout=0;

        }

//    	{
//        	FILE* fp;
//        	fp = fopen("/tmp/.latest_thttpd_request", "w");
//        	fprintf(fp, "%.80s URL \"%.80s\" tried to retrieve file\n", httpd_ntoa( &hc->client_addr ), hc->encodedurl);
//        	fclose(fp);
//    	}
	if (strstr(hc->encodedurl, "utilRequest.cgi"))
		chk_timeout=0;

	// added by Jeff on 2013/6/17 5min time-out removed for some NVR CGIs
	if (!strstr(hc->contenttype, "multipart/form-data") && chk_timeout==1 && !strstr(hc->encodedurl, "notify.cgi") && !strstr(hc->encodedurl, "qlive.cgi") && !strstr(hc->encodedurl, "qplay.cgi") && !strstr(hc->encodedurl, "putaudiostream.cgi"))
//	if (!strstr(hc->contenttype, "multipart/form-data") && !strstr(hc->encodedurl, "security.cgi") &&
//		!strstr(hc->encodedurl, "quick.cgi") && !strstr(hc->encodedurl, "reset_all.cgi") &&
//		!strstr(hc->encodedurl, "snapshot.cgi") && !strstr(hc->encodedurl, "misc/misc.cgi"))
//	if (!strstr(hc->encodedurl, "filemanager.cgi?upload"))
	{
		/* Schedule a kill for the child process, in case it runs too long */
		client_data.i = r;
		(void) tmr_create(
	    (struct timeval*) 0, cgi_kill, client_data, CGI_TIMELIMIT * 1000L,
	    0 );
	}
#endif /* CGI_TIMELIMIT */
	hc->status = 200;
	hc->bytes = CGI_BYTECOUNT;
	hc->should_linger = 0;
	}
    else
	{
	httpd_send_err(
	    hc, 501, err501title, "", err501form, httpd_method_str( hc->method ) );
	return -1;
	}

    return 0;
    }


static int
really_start_request( httpd_conn* hc )
    {
    static char* indexname;
    static int maxindexname = 0;
    static const char* index_names[] = { INDEX_NAMES };
    int i, ret;
#ifdef AUTH_FILE
    static char* dirname;
    static int maxdirname = 0;
#endif /* AUTH_FILE */
    int expnlen, indxlen;
    char* cp;
    char* pi;
    char tmpchar;
    int in_share; //, zero_arg; // add by YF Hunag 2004/02/12

    expnlen = strlen( hc->expnfilename );
    if ( hc->method != METHOD_GET && hc->method != METHOD_HEAD &&
	 hc->method != METHOD_POST )
	{
	httpd_send_err(
	    hc, 501, err501title, "", err501form, httpd_method_str( hc->method ) );
	return -1;
	}

    /* Stat the file. */
    if ( stat( hc->expnfilename, &hc->sb ) < 0 )
	{
	httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	return -1;
	}

    /* Is it world-readable or world-executable?  We check explicitly instead
    ** of just trying to open it, so that no one ever gets surprised by
    ** a file that's not set world-readable and yet somehow is
    ** readable by the HTTP server and therefore the *whole* world.
    */
    if ( ! ( hc->sb.st_mode ) )
	{
	syslog(
	    LOG_NOTICE,
	    "%.80s URL \"%.80s\" resolves to a non world-readable file",
	    httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "The requested URL '%.80s' resolves to a file that is not world-readable.\n" ),
	    hc->encodedurl );
	return -1;
	}

    /* Is it a directory? */
    if ( S_ISDIR(hc->sb.st_mode) )
	{
	/* If there's pathinfo, it's just a non-existent file. */
	if ( hc->pathinfo[0] != '\0' )
	    {
//	    httpd_send_err( hc, 404, err404title, "", err404form, hc->encodedurl );
//Weak point fix: modified by KenChen@QNAP 
	    httpd_send_err( hc, 404, err404title, "", err404form, "" );
	    return -1;
	    }

	/* Special handling for directory URLs that don't end in a slash.
	** We send back an explicit redirect with the slash, because
	** otherwise many clients can't build relative URLs properly.
	*/
	if ( hc->decodedurl[strlen( hc->decodedurl ) - 1] != '/' )
	    {
	    send_dirredirect( hc );
	    return -1;
	    }

	/* Check for an index file. */
	for ( i = 0; i < sizeof(index_names) / sizeof(char*); ++i )
	    {
	    httpd_realloc_str(
		&indexname, &maxindexname,
		expnlen + 1 + strlen( index_names[i] ) );
	    (void) strcpy( indexname, hc->expnfilename );
	    indxlen = strlen( indexname );
	    if ( indxlen == 0 || indexname[indxlen - 1] != '/' )
		(void) strcat( indexname, "/" );
	    (void) strcat( indexname, index_names[i] );
	    if ( stat( indexname, &hc->sb ) >= 0 )
		goto got_one;
	    }

	/* Nope, no index file, so it's an actual directory request. */
#ifdef GENERATE_INDEXES
	/* Directories must be readable for indexing. */
	if ( ! ( hc->sb.st_mode & S_IROTH ) )
	    {
	    syslog(
		LOG_NOTICE,
		"%.80s URL \"%.80s\" tried to index a directory with indexing disabled",
		httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	    httpd_send_err(
		hc, 403, err403title, "",
		ERROR_FORM( err403form, "The requested URL '%.80s' resolves to a directory that has indexing disabled.\n" ),
		hc->encodedurl );
	    return -1;
	    }
#ifdef AUTH_FILE
	/* Check authorization for this directory. */
	if ( (ret=auth_check( hc, hc->expnfilename )) <= -1 )
	{
	    return ret;
	    return -1;
	}
#endif /* AUTH_FILE */
	/* Referer check. */
	if ( ! check_referer( hc ) )
	    return -1;
	/* Ok, generate an index. */
	//return ls( hc );  /*- Mark by Nick -*/
	 return 1;
#else /* GENERATE_INDEXES */
	syslog(
	    LOG_NOTICE, "%.80s URL \"%.80s\" tried to index a directory",
	    httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "The requested URL '%.80s' is a directory, and directory indexing is disabled on this server.\n" ),
	    hc->encodedurl );
	return -1;
#endif /* GENERATE_INDEXES */

	got_one: ;
	/* Got an index file.  Expand symlinks again.  More pathinfo means
	** something went wrong.
	*/
	cp = expand_symlinks( indexname, &pi, hc->hs->no_symlink, hc->tildemapped );
	if ( cp == (char*) 0 || pi[0] != '\0' )
	    {
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    return -1;
	    }
	expnlen = strlen( cp );
	httpd_realloc_str( &hc->expnfilename, &hc->maxexpnfilename, expnlen );
	(void) strcpy( hc->expnfilename, cp );

	/* Now, is the index version world-readable or world-executable? */
	if ( ! ( hc->sb.st_mode ) )
	    {
	    syslog(
		LOG_NOTICE,
		"%.80s URL \"%.80s\" resolves to a non-world-readable index file",
		httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	    httpd_send_err(
		hc, 403, err403title, "",
		ERROR_FORM( err403form, "The requested URL '%.80s' resolves to an index file that is not world-readable.\n" ),
		hc->encodedurl );
	    return -1;
	    }
	}

#ifdef AUTH_FILE
    /* Check authorization for this directory. */
    httpd_realloc_str( &dirname, &maxdirname, expnlen );
    (void) strcpy( dirname, hc->expnfilename );
    cp = strrchr( dirname, '/' );
    if ( cp == (char*) 0 )
	(void) strcpy( dirname, "." );
    else
	*cp = '\0';
/************************************************/
/* add by Kent 2001/11/13			*/
/* ask username and password under root folder	*/
/************************************************/
strcpy(dirname, ".");
/* end */
    if ( (ret=auth_check( hc, dirname )) <= -1 )
    {
	return ret;
	return -1;
    }

    /* Check if the filename is the AUTH_FILE itself - that's verboten. */
    if ( expnlen == sizeof(AUTH_FILE) - 1 )
	{
	if ( strcmp( hc->expnfilename, AUTH_FILE ) == 0 )
	    {
	    syslog(
		LOG_NOTICE,
		"%.80s URL \"%.80s\" tried to retrieve an auth file",
		httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	    httpd_send_err(
		hc, 403, err403title, "",
		ERROR_FORM( err403form, "The requested URL '%.80s' is an authorization file, retrieving it is not permitted.\n" ),
		hc->encodedurl );
	    return -1;
	    }
	}
    else if ( expnlen >= sizeof(AUTH_FILE) &&
	      strcmp( &(hc->expnfilename[expnlen - sizeof(AUTH_FILE) + 1]), AUTH_FILE ) == 0 &&
	      hc->expnfilename[expnlen - sizeof(AUTH_FILE)] == '/' )
	{
	syslog(
	    LOG_NOTICE,
	    "%.80s URL \"%.80s\" tried to retrieve an auth file",
	    httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "The requested URL '%.80s' is an authorization file, retrieving it is not permitted.\n" ),
	    hc->encodedurl );
	return -1;
	}
#endif /* AUTH_FILE */

    /* Referer check. */
    if ( ! check_referer( hc ) )
	return -1;
    
    /* add by YF Hunag 2004/02/12 */
    /* user can't execute file by checking if it have no argument in URL */
 //   zero_arg = strlen( hc->query ) > 0 ? 0 : 1;
    in_share = strstr(hc->expnfilename, "cgi-bin/filemanager/share")==0 ? 0:1;
    
    /* Is it world-executable and in the CGI area? */ //motify by YF Hunag 2004/02/12
    if ( hc->hs->cgi_pattern != (char*) 0 &&
	 ( hc->sb.st_mode & S_IXOTH ) &&
	 match( hc->hs->cgi_pattern, hc->expnfilename )) 
	{
#ifdef HTTP_LIVE_STREAMING
                BOOL processed;
                int r=http_stream_cgi_req(hc,&processed);
                if(processed)return r;
#endif
		if(!in_share) {
			return cgi( hc );
		}
	}

    /* It's not CGI.  If it's executable or there's pathinfo, someone's
    ** trying to either serve or run a non-CGI file as CGI.   Either case
    ** is prohibited.
    */
    if ( hc->sb.st_mode & S_IXOTH && !in_share) //motify by YF Hunag 2004/02/12
	{
//add by Kent Tsai 2001/02/05
//execuable file and we want to save it
	hc->file_address=0x0;
	return 0;
	syslog(
	    LOG_NOTICE, "%.80s URL \"%.80s\" is executable but isn't CGI",
	    httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "The requested URL '%.80s' resolves to a file which is marked executable but is not a CGI file; retrieving it is forbidden.\n" ),
	    hc->encodedurl );
	return -1;
	}
    if ( hc->pathinfo[0] != '\0' )
	{
	syslog(
	    LOG_NOTICE, "%.80s URL \"%.80s\" has pathinfo but isn't CGI",
	    httpd_ntoa( &hc->client_addr ), hc->encodedurl );
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "The requested URL '%.80s' resolves to a file plus CGI-style pathinfo, but the file is not a valid CGI file.\n" ),
	    hc->encodedurl );
	return -1;
	}

    /* Fill in end_byte_loc if necessary. */
    if ( hc->got_range &&
	 ( hc->end_byte_loc == -1 || hc->end_byte_loc >= hc->sb.st_size ) )
	hc->end_byte_loc = hc->sb.st_size - 1;

    figure_mime( hc );

	// for byte range 206 partial content
	if( hc->got_range )
	{
	// added by Jeff on 2014/6/19 to solve "partial content no reply" issue
	hc->file_address = mmc_map( hc->expnfilename, &(hc->sb) );
	if ( hc->file_address == (char*) 0 )
	    {
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    return -1;
	    }
	hc->bytes = hc->sb.st_size;
	send_mime(
	    hc, 200, ok206title, hc->encodings, "", hc->type, hc->sb.st_size,
	    hc->sb.st_mtime );
	}
    else if ( hc->method == METHOD_HEAD )
	{
	hc->bytes = 0;
	send_mime(
	    hc, 200, ok200title, hc->encodings, "", hc->type, hc->sb.st_size,
	    hc->sb.st_mtime );
	}
    else if ( hc->if_modified_since != (time_t) -1 &&
	 hc->if_modified_since >= hc->sb.st_mtime )
	{
	hc->method = METHOD_HEAD;
	hc->bytes = 0;
	send_mime(
	    hc, 304, err304title, hc->encodings, "", hc->type, hc->sb.st_size,
	    hc->sb.st_mtime );
	}
    else
	{
	hc->file_address = mmc_map( hc->expnfilename, &(hc->sb) );
	if ( hc->file_address == (char*) 0 )
	    {
	    httpd_send_err( hc, 500, err500title, "", err500form, hc->encodedurl );
	    return -1;
	    }
	hc->bytes = hc->sb.st_size;
	send_mime(
	    hc, 200, ok200title, hc->encodings, "", hc->type, hc->sb.st_size,
	    hc->sb.st_mtime );
	}
    return 0;
    }


int
httpd_start_request( httpd_conn* hc )
    {
    int r;
    char* ru;
    char url[305];
    char bytes[40];

    /* Really start the request, then do the log entry. */
    r = really_start_request( hc );

    /* This is straight CERN Combined Log Format - the only tweak
    ** being that if we're using syslog() we leave out the date, because
    ** syslogd puts it in.  The included syslogtocern script turns the
    ** results into true CERN format.
    */

    /* Format remote user. */
    if ( hc->remoteuser[0] != '\0' )
	ru = hc->remoteuser;
    else
	ru = "-";
    /* If we're vhosting, prepend the hostname to the url.  This is
    ** a little weird, perhaps writing separate log files for
    ** each vhost would make more sense.
    */
    if ( hc->hs->vhost && ! hc->tildemapped )
	(void) my_snprintf( url, sizeof(url),
	    "/%.100s%.200s", hc->hostname, hc->encodedurl );
    else
	(void) my_snprintf( url, sizeof(url),
	    "%.200s", hc->encodedurl );
    /* Format the bytes. */
    if ( hc->bytes >= 0 )
	{
	(void) my_snprintf( bytes, sizeof(bytes),
	    "%lld", hc->bytes );
	}
    else
	(void) strcpy( bytes, "-" );

    /* Logfile or syslog? */
    if ( hc->hs->logfp != (FILE*) 0 )
	{
	time_t now;
	struct tm* t;
	const char* cernfmt_nozone = "%d/%b/%Y:%H:%M:%S";
	char date_nozone[100];
	int zone;
	char sign;
	char date[100];

	/* Format the time, forcing a numeric timezone (some log analyzers
	** are stoooopid about this).
	*/
	now = time( (time_t*) 0 );
	t = localtime( &now );
	(void) strftime( date_nozone, sizeof(date_nozone), cernfmt_nozone, t );
#ifdef HAVE_TM_GMTOFF
	zone = t->tm_gmtoff / 60L;
#else
	zone = -timezone / 60L;
	/* Probably have to add something about daylight time here. */
#endif
	if ( zone >= 0 )
	    sign = '+';
	else
	    {
	    sign = '-';
	    zone = -zone;
	    }
	zone = ( zone / 60 ) * 100 + zone % 60;
	(void) my_snprintf( date, sizeof(date),
	    "%s %c%04d", date_nozone, sign, zone );
	/* And write the log entry. */
	(void) fprintf( hc->hs->logfp,
	    "%.80s - %.80s [%s] \"%.80s %.300s %.80s\" %d %s \"%.200s\" \"%.80s\"\n",
	    httpd_ntoa( &hc->client_addr ), ru, date,
	    httpd_method_str( hc->method ), url, hc->protocol,
	    hc->status, bytes, hc->referer, hc->useragent );
	(void) fflush( hc->hs->logfp );	/* don't need to flush every time */
	}
    else
	syslog( LOG_INFO,
	    "%.80s - %.80s \"%.80s %.200s %.80s\" %d %s \"%.200s\" \"%.80s\"",
	    httpd_ntoa( &hc->client_addr ), ru,
	    httpd_method_str( hc->method ), url, hc->protocol,
	    hc->status, bytes, hc->referer, hc->useragent );

    return r;
    }


/* Returns 1 if ok to serve the url, 0 if not. */
static int
check_referer( httpd_conn* hc )
    {
    int r;

    /* Are we doing referer checking at all? */
    if ( hc->hs->url_pattern == (char*) 0 )
	return 1;

    r = really_check_referer( hc );

    if ( ! r )
	{
	syslog(
	    LOG_NOTICE, "%.80s non-local referer \"%.80s\" \"%.80s\"",
	    httpd_ntoa( &hc->client_addr ), hc->encodedurl,
	    hc->referer );
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "You must supply a local referer to get URL '%.80s' from this server.\n" ),
	    hc->encodedurl );
	}
    return r;
    }


/* Returns 1 if ok to serve the url, 0 if not. */
static int
really_check_referer( httpd_conn* hc )
    {
    httpd_server* hs;
    char* cp1;
    char* cp2;
    char* cp3;
    static char* refhost = (char*) 0;
    static int refhost_size = 0;
    char *lp;

    hs = hc->hs;

    /* Check for an empty referer. */
    if ( hc->referer == (char*) 0 || hc->referer[0] == '\0' ||
	 ( cp1 = strstr( hc->referer, "//" ) ) == (char*) 0 )
	{
	/* Disallow if we require a referer and the url matches. */
	if ( hs->no_empty_referers && match( hs->url_pattern, hc->decodedurl ) )
	    return 0;
	/* Otherwise ok. */
	return 1;
	}

    /* Extract referer host. */
    cp1 += 2;
    for ( cp2 = cp1; *cp2 != '/' && *cp2 != ':' && *cp2 != '\0'; ++cp2 )
	continue;
    httpd_realloc_str( &refhost, &refhost_size, cp2 - cp1 );
    for ( cp3 = refhost; cp1 < cp2; ++cp1, ++cp3 )
	if ( isupper(*cp1) )
	    *cp3 = tolower(*cp1);
	else
	    *cp3 = *cp1;
    *cp3 = '\0';

    /* Local pattern? */
    if ( hs->local_pattern != (char*) 0 )
	lp = hs->local_pattern;
    else
	{
	/* No local pattern.  What's our hostname? */
	if ( ! hs->vhost )
	    {
	    /* Not vhosting, use the server name. */
	    lp = hs->server_hostname;
	    if ( lp == (char*) 0 )
		/* Couldn't figure out local hostname - give up. */
		return 1;
	    }
	else
	    {
	    /* We are vhosting, use the hostname on this connection. */
	    lp = hc->hostname;
	    if ( lp == (char*) 0 )
		/* Oops, no hostname.  Maybe it's an old browser that
		** doesn't send a Host: header.  We could figure out
		** the default hostname for this IP address, but it's
		** not worth it for the few requests like this.
		*/
		return 1;
	    }
	}

    /* If the referer host doesn't match the local host pattern, and
    ** the URL does match the url pattern, it's an illegal reference.
    */
    if ( ! match( lp, refhost ) && match( hs->url_pattern, hc->decodedurl ) )
	return 0;
    /* Otherwise ok. */
    return 1;
    }


char*
httpd_ntoa( httpd_sockaddr* saP )
    {
#ifdef HAVE_GETNAMEINFO
    static char str[200];

    if ( getnameinfo( &saP->sa, sockaddr_len( saP ), str, sizeof(str), 0, 0, NI_NUMERICHOST ) != 0 )
	{
	str[0] = '?';
	str[1] = '\0';
	}
    return str;

#else /* HAVE_GETNAMEINFO */

    return inet_ntoa( saP->sa_in.sin_addr );

#endif /* HAVE_GETNAMEINFO */
    }


static int
sockaddr_check( httpd_sockaddr* saP )
    {
    switch ( saP->sa.sa_family )
	{
	case AF_INET: return 1;
#if defined(AF_INET6) && defined(HAVE_SOCKADDR_IN6)
	case AF_INET6: return 1;
#endif /* AF_INET6 && HAVE_SOCKADDR_IN6 */
	default:
	return 0;
	}
    }


static size_t
sockaddr_len( httpd_sockaddr* saP )
    {
    switch ( saP->sa.sa_family )
	{
	case AF_INET: return sizeof(struct sockaddr_in);
#if defined(AF_INET6) && defined(HAVE_SOCKADDR_IN6)
	case AF_INET6: return sizeof(struct sockaddr_in6);
#endif /* AF_INET6 && HAVE_SOCKADDR_IN6 */
	default:
	return 0;	/* shouldn't happen */
	}
    }


/* Some systems don't have snprintf(), so we make our own that uses
** either vsnprintf() or vsprintf().  If your system doesn't have
** vsnprintf(), it is probably vulnerable to buffer overruns.
** Upgrade!
*/
static int
my_snprintf( char* str, size_t size, const char* format, ... )
    {
    va_list ap;
    int r;

    va_start( ap, format );
#ifdef HAVE_VSNPRINTF
    r = vsnprintf( str, size, format, ap );
#else /* HAVE_VSNPRINTF */
    r = vsprintf( str, format, ap );
#endif /* HAVE_VSNPRINTF */
    va_end( ap );
    return r;
    }

/* Write the requested buffer completely, accounting for interruptions. */
int
httpd_write_fully( int fd, const void* buf, size_t nbytes )
    {
    int nwritten;

    nwritten = 0;
    while ( nwritten < nbytes )
        {
        int r;

        r = write( fd, buf + nwritten, nbytes - nwritten );
        if ( r < 0 )
            if ( errno == EAGAIN )
                sleep( 1 );
            else
                return r;
        else if ( r == 0 )
            break;
        nwritten += r;
        }

    return nwritten;
    }

//Richard Chang 20070821 add log
static void add_loginfail_log(httpd_conn* hc, char *mgs)
{
	if(hc->source_ip[0] != '\0'){
		SendConnToLogEngine(EVENT_TYPE_WARN, hc->remoteuser, hc->source_ip, 
			"---", (is_https() == 0) ? CONN_SERV_HTTP : CONN_SERV_HTTPS, CONN_ACTION_LOGIN_FAIL,  "---");
	}
	else{
		SendConnToLogEngine(EVENT_TYPE_WARN, hc->remoteuser, httpd_ntoa(&hc->client_addr), 
			"---", (is_https() == 0) ? CONN_SERV_HTTP : CONN_SERV_HTTPS, CONN_ACTION_LOGIN_FAIL,  "---");
	}
}
//end
#ifdef HTTP_LIVE_STREAMING
#include "http_stream.c"
#endif

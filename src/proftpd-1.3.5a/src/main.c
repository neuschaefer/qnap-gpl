/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2015 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* House initialization and main program loop */

#include "conf.h"

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifdef HAVE_LIBUTIL_H
# include <libutil.h>
#endif /* HAVE_LIBUTIL_H */

#ifdef _QNAP_
#include <Util.h>
#include <qLogEngine.h>
#include <ftp_nas.h>
#include <cfg_ftp.h>
#include "getwanip.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <linux/unistd.h>   //for qnap_unlink system call
#define qnap_unlink(a) syscall(__NR_qnap_unlink, a)

#define Q_KEY		1234
#define Q_TEXTSIZE	128

char Qmaster_fifo_rootdir[256] = "";

int q_shmid;
#define RPC_PROC
static pid_t rpc_pid = -1;

// Hugo add. timezone info for list file modify time
#include <time.h>
#include <sys/time.h>
extern long timezone;
int tz_val;
int custom_root = 0;
char custom_root_path[512] = {0};
#endif	//_QNAP_

#ifdef _NVR_
#include "frpc.h"
#endif

#ifdef HAVE_EXECINFO_H
# include <execinfo.h>
#endif

#ifdef HAVE_UNAME
# include <sys/utsname.h>
#endif

#ifdef HAVE_UCONTEXT_H
# include <ucontext.h>
#endif

#include "privs.h"

int (*cmd_auth_chk)(cmd_rec *);
void (*cmd_handler)(server_rec *, conn_t *);

// Modified by Jeff Chang 2011/5/4, for service binding
#ifdef _QNAP_
int  g_bHup=FALSE;

//////////////////////////////////////////////////////////////////////////////
// Network interface related definitaions / functions.

#define	SZP_GET_BIND_ADDR		"/sbin/getbindaddr"
#define SZ_INADDR_ANY           "0.0.0.0"
// The structure for binding address information.
typedef	struct _T_BIND_ADDR_
{
	int		bEnabled;
	int		cnIpv4;
	int		cnIpv6;
	char	**ryszIpv4;
	char	**ryszIpv6;
} T_BIND_ADDR, *PT_BIND_ADDR;

SMB_SHARE_INFO *psmb=NULL;
unsigned char persistent_passwd = FALSE;
#else
#ifdef NEED_PERSISTENT_PASSWD
unsigned char persistent_passwd = TRUE;
#else
unsigned char persistent_passwd = FALSE;
#endif /* NEED_PERSISTENT_PASSWD */
#endif	//_QNAP_

/* From modules/module_glue.c */
extern module *static_modules[];

extern xaset_t *server_list;

unsigned long max_connects = 0UL;
unsigned int max_connect_interval = 1;

session_t session;

/* Is this process the master standalone daemon process? */
unsigned char is_master = TRUE;

pid_t mpid = 0;				/* Master pid */

uid_t daemon_uid;
gid_t daemon_gid;
array_header *daemon_gids;

static time_t shut = 0, deny = 0, disc = 0;
static char shutmsg[81] = {'\0'};

static unsigned char have_dead_child = FALSE;

/* The default command buffer size SHOULD be large enough to handle the
 * maximum path length, plus 4 bytes for the FTP command, plus 1 for the
 * whitespace separating command from path, and 2 for the terminating CRLF.
 */
#define PR_DEFAULT_CMD_BUFSZ	(PR_TUNABLE_PATH_MAX + 7)

/* From response.c */
extern pr_response_t *resp_list, *resp_err_list;

static int nodaemon  = 0;
static int quiet     = 0;
static int shutdownp = 0;
static int syntax_check = 0;

/* Command handling */
static void cmd_loop(server_rec *, conn_t *);

/* Signal handling */
static RETSIGTYPE sig_disconnect(int);
static RETSIGTYPE sig_evnt(int);
static RETSIGTYPE sig_terminate(int);
#ifdef PR_DEVEL_STACK_TRACE
static void install_stacktrace_handler(void);
#endif /* PR_DEVEL_STACK_TRACE */

volatile unsigned int recvd_signal_flags = 0;

/* Used to capture an "unknown" signal value that causes termination. */
static int term_signo = 0;

/* Signal processing functions */
static void handle_abort(void);
static void handle_chld(void);
static void handle_evnt(void);
#ifdef PR_DEVEL_STACK_TRACE
static void handle_stacktrace_signal(int, siginfo_t *, void *);
#endif /* PR_DEVEL_STACK_TRACE */
static void handle_xcpu(void);
static void handle_terminate(void);
static void handle_terminate_other(void);
static void finish_terminate(void);

static cmd_rec *make_ftp_cmd(pool *, char *, int);

static const char *config_filename = PR_CONFIG_FILE_PATH;
#ifdef _QNAP_
void *pipe_cmd(const char *cmd, void *arg);
#endif //_QNAP_

#ifdef _QNAP_
static int qSystem(const char *cmd)
{
#define QNAP_NOTIFY_NAMESPACE "QNAP_NOTIFY_NAMESPACE"
	char lpc[1024];
	int sock;
	sock = socket(AF_LOCAL, SOCK_STREAM, 0);
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	memcpy((char *) &addr.sun_path + 1, QNAP_NOTIFY_NAMESPACE, strlen(QNAP_NOTIFY_NAMESPACE));
	int rc = connect(sock, &addr, sizeof(addr));
	if (rc == 0) {
		rc = -1;
		snprintf(lpc, sizeof(lpc), "SYS:%s", cmd);
		send(sock, lpc, strlen(lpc)+1, 0);
		char buf[128];
		recv(sock, buf, sizeof(buf), 0);
		if (strncmp(buf, "ret ", 4) == 0) {
			rc = atoi(buf+4);
		}
        pr_log_pri(PR_LOG_DEBUG, "qSystem: %s", buf);
	}
	else {
        pr_log_pri(PR_LOG_ERR, "qSystem: failed %d", errno);
	}
	close(sock);
	return rc;
}
#endif

#ifdef _NVR_
TNvrFenInfo __nvr_fen_info;
static void read_nvr_config(void)
{
    TMsFenInfo *p;
    FILE *cfg_file, *fp;
    if(__nvr_fen_info.fen_info)
	free(__nvr_fen_info.fen_info);
    cfg_file=fopen( "/etc/config/nvrd.xml", "r");
    if( !cfg_file )
    {
        __nvr_fen_info.channel_num=0;
	return;
    }
    fclose(cfg_file);

//Hugo. check file existence when first use NAS
    fp=fopen("/usr/bin/gfeninfo", "r");
    if (!fp)
        return;
    fclose(fp);

    FRPC_REQ_CALL_Q("/usr/bin/gfeninfo");
    FRPC_READ_N( __nvr_fen_info.channel_num );
    if( __nvr_fen_info.channel_num>0 )
    {
	int i;
	__nvr_fen_info.fen_info=(TMsFenInfo *)malloc(__nvr_fen_info.channel_num*sizeof(TMsFenInfo));
	p=__nvr_fen_info.fen_info;
	for( i=0; i<__nvr_fen_info.channel_num; i++, p++ )
	{
            char *addr=NULL, addr2[64];
	    FRPC_READ_S(addr);
            if( !is_domain_name( addr )|| !resolve_ip( addr2, addr ))
                strcpy( addr2, addr );
	    p->address=strdup(addr2);
	    FRPC_READ_N(p->ftp_evt_notifiy_enabled );
		if(addr) free(addr);
	}
    }
    FRPC_END_REQ;
}
#endif

/* Add child semaphore fds into the rfd for selecting */
static int semaphore_fds(fd_set *rfd, int maxfd) {

  if (child_count()) {
    pr_child_t *ch;

    for (ch = child_get(NULL); ch; ch = child_get(ch)) {
      if (ch->ch_pipefd != -1) {
        FD_SET(ch->ch_pipefd, rfd);
        if (ch->ch_pipefd > maxfd) {
          maxfd = ch->ch_pipefd;
        }
      }
    }
  }

  return maxfd;
}

void set_auth_check(int (*chk)(cmd_rec*)) {
  cmd_auth_chk = chk;
}

void pr_cmd_set_handler(void (*handler)(server_rec *, conn_t *)) {
  if (handler == NULL) {
    cmd_handler = cmd_loop;

  } else {
    cmd_handler = handler;
  }
}

void session_exit(int pri, void *lv, int exitval, void *dummy) {
  char *msg = (char *) lv;

  pr_log_pri(pri, "%s", msg);

  if (ServerType == SERVER_STANDALONE &&
      is_master) {
    pr_log_pri(PR_LOG_NOTICE, "ProFTPD " PROFTPD_VERSION_TEXT
      " standalone mode SHUTDOWN");

    PRIVS_ROOT
    pr_delete_scoreboard();
    if (!nodaemon)
      pr_pidfile_remove();
    PRIVS_RELINQUISH
  }

  pr_session_end(0);
}

static void shutdown_exit(void *d1, void *d2, void *d3, void *d4) {
  if (check_shutmsg(&shut, &deny, &disc, shutmsg, sizeof(shutmsg)) == 1) {
    char *user;
    time_t now;
    char *msg;
    const char *serveraddress;
    config_rec *c = NULL;
    unsigned char *authenticated = get_param_ptr(main_server->conf,
      "authenticated", FALSE);

    serveraddress = (session.c && session.c->local_addr) ?
      pr_netaddr_get_ipstr(session.c->local_addr) :
      main_server->ServerAddress;

    if ((c = find_config(main_server->conf, CONF_PARAM, "MasqueradeAddress",
        FALSE)) != NULL) {

      pr_netaddr_t *masq_addr = (pr_netaddr_t *) c->argv[0];
      serveraddress = pr_netaddr_get_ipstr(masq_addr);
    }

    time(&now);
    if (authenticated && *authenticated == TRUE) {
      user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);

    } else {
      user = "NONE";
    }

    msg = sreplace(permanent_pool, shutmsg,
                   "%s", pstrdup(permanent_pool, pr_strtime(shut)),
                   "%r", pstrdup(permanent_pool, pr_strtime(deny)),
                   "%d", pstrdup(permanent_pool, pr_strtime(disc)),
		   "%C", (session.cwd[0] ? session.cwd : "(none)"),
		   "%L", serveraddress,
		   "%R", (session.c && session.c->remote_name ?
                         session.c->remote_name : "(unknown)"),
		   "%T", pstrdup(permanent_pool, pr_strtime(now)),
		   "%U", user,
		   "%V", main_server->ServerName,
                   NULL );

    pr_response_send_async(R_421, _("FTP server shutting down - %s"), msg);

    pr_log_pri(PR_LOG_NOTICE, "%s", msg);
    pr_session_disconnect(NULL, PR_SESS_DISCONNECT_SERVER_SHUTDOWN, NULL);
  }

  if (signal(SIGUSR1, sig_disconnect) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGUSR1 (signal %d) handler: %s", SIGUSR1,
      strerror(errno));
  }
}

static int get_command_class(const char *name) {
  int idx = -1;
  cmdtable *c = pr_stash_get_symbol(PR_SYM_CMD, name, NULL, &idx);

  while (c && c->cmd_type != CMD) {
    pr_signals_handle();
    c = pr_stash_get_symbol(PR_SYM_CMD, name, c, &idx);
  }

  /* By default, every command has a class of CL_ALL.  This insures that
   * any configured ExtendedLogs that default to "all" will log the command.
   */
  return (c ? c->cmd_class : CL_ALL);
}

static int _dispatch(cmd_rec *cmd, int cmd_type, int validate, char *match) {
  char *cmdargstr = NULL;
  cmdtable *c;
  modret_t *mr;
  int success = 0, xerrno = 0;
  int send_error = 0;
  static int match_index_cache = -1;
  static char *last_match = NULL;
  int *index_cache;

  send_error = (cmd_type == PRE_CMD || cmd_type == CMD ||
    cmd_type == POST_CMD_ERR);

  if (!match) {
    match = cmd->argv[0];
    index_cache = &cmd->stash_index;

  } else {
    if (last_match != match) {
      match_index_cache = -1;
      last_match = match;
    }

    index_cache = &match_index_cache;
  }

  c = pr_stash_get_symbol(PR_SYM_CMD, match, NULL, index_cache);

  while (c && !success) {
    size_t cmdargstrlen = 0;

    pr_signals_handle();

    session.curr_cmd = cmd->argv[0];
    session.curr_cmd_id = cmd->cmd_id;
    session.curr_cmd_rec = cmd;
    session.curr_phase = cmd_type;

    if (c->cmd_type == cmd_type) {
      if (c->group)
        cmd->group = pstrdup(cmd->pool, c->group);

      if (c->requires_auth &&
          cmd_auth_chk &&
          !cmd_auth_chk(cmd)) {
        pr_trace_msg("command", 8,
          "command '%s' failed 'requires_auth' check for mod_%s.c",
          cmd->argv[0], c->m->name);
        errno = EACCES;
        return -1;
      }

      if (cmd->tmp_pool == NULL) {
        cmd->tmp_pool = make_sub_pool(cmd->pool);
        pr_pool_tag(cmd->tmp_pool, "cmd_rec tmp pool");
      }

      cmdargstr = pr_cmd_get_displayable_str(cmd, &cmdargstrlen);

      if (cmd_type == CMD) {

        /* The client has successfully authenticated... */
        if (session.user) {
          char *args = NULL;

          /* Be defensive, and check whether cmdargstrlen has a value.
           * If it's zero, assume we need to use strchr(3), rather than
           * memchr(2); see Bug#3714.
           */
          if (cmdargstrlen > 0) {
            args = memchr(cmdargstr, ' ', cmdargstrlen);

          } else {
            args = strchr(cmdargstr, ' ');
          }

          pr_scoreboard_entry_update(session.pid,
            PR_SCORE_CMD, "%s", cmd->argv[0], NULL, NULL);
          pr_scoreboard_entry_update(session.pid,
            PR_SCORE_CMD_ARG, "%s", args ? (args + 1) : "", NULL, NULL);

          pr_proctitle_set("%s - %s: %s", session.user, session.proc_prefix,
            cmdargstr);

        /* ...else the client has not yet authenticated */
        } else {
          pr_proctitle_set("%s:%d: %s", session.c->remote_addr ?
            pr_netaddr_get_ipstr(session.c->remote_addr) : "?",
            session.c->remote_port ? session.c->remote_port : 0, cmdargstr);
        }
      }

      pr_log_debug(DEBUG4, "dispatching %s command '%s' to mod_%s",
        (cmd_type == PRE_CMD ? "PRE_CMD" :
         cmd_type == CMD ? "CMD" :
         cmd_type == POST_CMD ? "POST_CMD" :
         cmd_type == POST_CMD_ERR ? "POST_CMD_ERR" :
         cmd_type == LOG_CMD ? "LOG_CMD" :
         cmd_type == LOG_CMD_ERR ? "LOG_CMD_ERR" :
         "(unknown)"),
        cmdargstr, c->m->name);

      pr_trace_msg("command", 7, "dispatching %s command '%s' to mod_%s.c",
        (cmd_type == PRE_CMD ? "PRE_CMD" :
         cmd_type == CMD ? "CMD" :
         cmd_type == POST_CMD ? "POST_CMD" :
         cmd_type == POST_CMD_ERR ? "POST_CMD_ERR" :
         cmd_type == LOG_CMD ? "LOG_CMD" :
         cmd_type == LOG_CMD_ERR ? "LOG_CMD_ERR" :
         "(unknown)"),
        cmdargstr, c->m->name);

      cmd->cmd_class |= c->cmd_class;

      /* KLUDGE: disable umask() for not G_WRITE operations.  Config/
       * Directory walking code will be completely redesigned in 1.3,
       * this is only necessary for perfomance reasons in 1.1/1.2
       */

      if (!c->group || strcmp(c->group, G_WRITE) != 0)
        kludge_disable_umask();

#ifdef _NVR_
      if( session.is_event_report_session )
      {
#if 0
log_setdebuglevel(DEBUG1);
if( !strcmp( cmd->argv[0], "STOR")|| !strcmp( cmd->argv[0], "CWD"))
log_debug(DEBUG1, "EVENT REPORT FTP cmd=[%s],arg=[%s]\n", cmd->argv[0], cmd->argv[1]);
else
log_debug(DEBUG1, "EVENT REPORT FTP cmd=[%s]\n", cmd->argv[0]);
#endif
        // only some commands are allowed
        if(!strcmp( cmd->argv[0], "CWD"))
        {
//        extern int notify_event( int channel_id, char *ftp_file_path );
          if (cmd->argv[1]) {
	    if (strstr(cmd->argv[1], "event")) {
              if (cmd->argv[1][0] == '/' && strcmp(cmd->argv[1], "/"))
                strcpy(session.cwd_path, cmd->argv[1]+1);
              else
                strcpy(session.cwd_path, cmd->argv[1]);
	    }
          }
          if (cmd_type==CMD)
            pr_response_send("250", "CWD command successful.");
	  return 1;
        }
        else if (!strcmp(cmd->argv[0], "MKD"))
        {
          if (cmd_type==CMD)
            pr_response_send("250", "MKD command successful.");
          return 1;
        }
        else if (!strcmp(cmd->argv[0], "CLNT")) // for basler & planet
        {
            if (cmd_type==CMD)
                pr_response_send("500", "CLNT command not understoon.");
            return 1;
        }
        else if (!strcmp(cmd->argv[0], "HELP")) // for basler & planet
        {
            if (cmd_type==CMD)
                pr_response_send("214", "HELP command successful.");
            return 1;
        }
	else if (!strcmp(cmd->argv[0], "RMD"))
	{
	  if (cmd_type==CMD)
	    pr_response_send("250", "RMD command successful.");
	  return 1;
	}
	else if (!strcmp(cmd->argv[0], "NOOP"))
	{
	    if (cmd_type==CMD)
	        pr_response_send("200", "NOOP command successful.");
	    return 1;
	}
	else if (!strcmp(cmd->argv[0], "SIZE"))
	{
	    if (cmd_type==CMD)
	        pr_response_send("550", "FILE not exist");
	    return 1;
	}
	else if (!strcmp( cmd->argv[0], "STOR")) // for fiti
	{
          if (cmd->argv[1][0] == '/') {
            char filepath[256];
            char *p;
            strcpy(filepath, cmd->argv[1]+1);
            p = strstr(filepath, "/");
            if (p) {
              *p = 0;
              strcpy (session.cwd_path, filepath);
            }
          }
	}
	else if (!strcmp( cmd->argv[0], "ALLO")) { // for CNB
	  if (cmd_type==CMD)
	    pr_response_send("200", "NOOP command successful.");
	  return 1;
	}
        else if (!strcmp( cmd->argv[0], "FEAT") || !strcmp( cmd->argv[0], "EPSV")) { // for Brickcom
          if (cmd_type==CMD)
            pr_response_send("502", "Command not support");
          return 1;
        }
	else if (strcmp(cmd->argv[0], "PASS") && strcmp(cmd->argv[0], "STOR")
	&& strcmp(cmd->argv[0], "SYST") && strcmp(cmd->argv[0], "TYPE")
	&& strcmp(cmd->argv[0], "USER") && strcmp(cmd->argv[0], "PASV") 
	&& strcmp(cmd->argv[0], "PORT") && strcmp(cmd->argv[0], "MODE") 
	&& strcmp(cmd->argv[0], "STRU") && strcmp(cmd->argv[0], "ABOR") 
	&& strcmp(cmd->argv[0], "QUIT") && strcmp(cmd->argv[0], "PWD") 
	&& strcmp(cmd->argv[0], "REST") )
	{
          pr_response_send(R_550, "Event Notificaton Only!(cmd %s not allowed)", cmd->argv[0]);
          end_login(0);
          return 0;
        }
	// Hugo add. 2007.03.09
	// Fix bug for Axis protocol. It must wait for a while, so that
	// it won't be hang on. So the next event notification in one minutes
	// won't be blocked
	usleep(10000);
      }
#endif
			
      mr = pr_module_call(c->m, c->handler, cmd);
      kludge_enable_umask();

      if (MODRET_ISHANDLED(mr)) {
        success = 1;

      } else if (MODRET_ISERROR(mr)) {
        xerrno = errno;
        success = -1;

        if (cmd_type == POST_CMD ||
            cmd_type == LOG_CMD ||
            cmd_type == LOG_CMD_ERR) {
          if (MODRET_ERRMSG(mr)) {
            pr_log_pri(PR_LOG_NOTICE, "%s", MODRET_ERRMSG(mr));
          }

          /* Even though we normally want to return a negative value
           * for success (indicating lack of success), for
           * LOG_CMD/LOG_CMD_ERR handlers, we always want to handle
           * errors as a success value of zero (meaning "keep looking").
           *
           * This will allow the cmd_rec to continue to be dispatched to
           * the other interested handlers (Bug#3633).
           */
          if (cmd_type == LOG_CMD || 
              cmd_type == LOG_CMD_ERR) {
            success = 0;
          }

        } else if (send_error) {
          if (MODRET_ERRNUM(mr) &&
              MODRET_ERRMSG(mr)) {
            pr_response_add_err(MODRET_ERRNUM(mr), "%s", MODRET_ERRMSG(mr));

          } else if (MODRET_ERRMSG(mr)) {
            pr_response_send_raw("%s", MODRET_ERRMSG(mr));
          }
        }

        errno = xerrno;
      }

      if (session.user &&
          !(session.sf_flags & SF_XFER) &&
          cmd_type == CMD) {
        pr_session_set_idle();
      }

      destroy_pool(cmd->tmp_pool);
      cmd->tmp_pool = NULL;
    }

    if (!success) {
      c = pr_stash_get_symbol(PR_SYM_CMD, match, c, index_cache);
    }
  }

  /* Note: validate is only TRUE for the CMD phase, for specific handlers
   * (as opposed to any C_ANY handlers).
   */

  if (!c &&
      !success &&
      validate) {
    char *method;

    /* Prettify the command method, if need be. */
    if (strchr(cmd->argv[0], '_') == NULL) {
      method = cmd->argv[0];

    } else {
      register unsigned int i;

      method = pstrdup(cmd->pool, cmd->argv[0]);
      for (i = 0; method[i]; i++) {
        if (method[i] == '_')
          method[i] = ' ';
      }
    }

    pr_event_generate("core.unhandled-command", cmd);

    pr_response_add_err(R_500, _("%s not understood"), method);
    success = -1;
  }

  return success;
}

/* Returns the appropriate maximum buffer size to use for FTP commands
 * from the client.
 */
static size_t get_max_cmd_sz(void) {
  size_t res;
  size_t *bufsz = NULL;

  bufsz = get_param_ptr(main_server->conf, "CommandBufferSize", FALSE);
  if (bufsz == NULL) {
    res = PR_DEFAULT_CMD_BUFSZ;

  } else {
    pr_log_debug(DEBUG1, "setting CommandBufferSize to %lu",
      (unsigned long) *bufsz);
    res = *bufsz;
  }

  return res;
}

int pr_cmd_read(cmd_rec **res) {
  static long cmd_bufsz = -1;
  static char *cmd_buf = NULL;
  char *cp;
  size_t cmd_buflen;

  if (res == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (cmd_bufsz == -1) {
    cmd_bufsz = get_max_cmd_sz();
  }

  if (cmd_buf == NULL) {
    cmd_buf = pcalloc(session.pool, cmd_bufsz + 1);
  }

  while (TRUE) {
    pr_signals_handle();

    memset(cmd_buf, '\0', cmd_bufsz);

    if (pr_netio_telnet_gets(cmd_buf, cmd_bufsz, session.c->instrm,
        session.c->outstrm) == NULL) {

      if (errno == E2BIG) {
        /* The client sent a too-long command which was ignored; give
         * them another chance?
         */
        continue;
      }

      if (session.c->instrm->strm_errno == 0) {
        pr_trace_msg("command", 6,
          "client sent EOF, closing control connection");
      }

      return -1;
    }

    break;
  }

  /* This strlen(3) is guaranteed to terminate; the last byte of buf is
   * always NUL, since pr_netio_telnet_gets() is told that the buf size is
   * one byte less than it really is.
   *
   * If the strlen(3) says that the length is less than the cmd_bufsz, then
   * there is no need to truncate the buffer by inserting a NUL.
   */
  cmd_buflen = strlen(cmd_buf);
  if (cmd_buflen > cmd_bufsz) {
    pr_log_debug(DEBUG0, "truncating incoming command length (%lu bytes) to "
      "CommandBufferSize %lu; use the CommandBufferSize directive to increase "
      "the allowed command length", (unsigned long) cmd_buflen,
      (unsigned long) cmd_bufsz);
    cmd_buf[cmd_bufsz-1] = '\0';
  }

  if (cmd_buflen &&
      (cmd_buf[cmd_buflen-1] == '\n' || cmd_buf[cmd_buflen-1] == '\r')) {
    cmd_buf[cmd_buflen-1] = '\0';
    cmd_buflen--;

    if (cmd_buflen &&
        (cmd_buf[cmd_buflen-1] == '\n' || cmd_buf[cmd_buflen-1] =='\r')) {
      cmd_buf[cmd_buflen-1] = '\0';
      cmd_buflen--;
    }
  }

  cp = cmd_buf;
  if (*cp == '\r')
    cp++;

  if (*cp) {
    int flags = 0;
    cmd_rec *cmd;

    /* If this is a SITE command, preserve embedded whitespace in the
     * command parameters, in order to handle file names that have multiple
     * spaces in the names.  Arguably this should be handled in the SITE
     * command handlers themselves, via cmd->arg.  This small hack
     * reduces the burden on SITE module developers, however.
     */
    if (strncasecmp(cp, C_SITE, 4) == 0) {
      flags |= PR_STR_FL_PRESERVE_WHITESPACE;
    }

    cmd = make_ftp_cmd(session.pool, cp, flags);
    if (cmd) {
      *res = cmd;

      if (pr_cmd_is_http(cmd) == TRUE) {
        cmd->is_ftp = FALSE;
        cmd->protocol = "HTTP";

      } else if (pr_cmd_is_smtp(cmd) == TRUE) {
        cmd->is_ftp = FALSE;
        cmd->protocol = "SMTP";

      } else {
        /* Assume that the client is sending valid FTP commands. */
        cmd->is_ftp = TRUE;
        cmd->protocol = "FTP";
      }
    } 
  }

  return 0;
}

int pr_cmd_dispatch_phase(cmd_rec *cmd, int phase, int flags) {
  char *cp = NULL;
  int success = 0, xerrno = 0;
  pool *resp_pool = NULL;

  if (cmd == NULL) {
    errno = EINVAL;
    return -1;
  }

  cmd->server = main_server;

  if (flags & PR_CMD_DISPATCH_FL_CLEAR_RESPONSE) {
    pr_trace_msg("response", 9,
      "clearing response lists before dispatching command '%s'", cmd->argv[0]);
    pr_response_clear(&resp_list);
    pr_response_clear(&resp_err_list);
  }

  /* Get any previous pool that may be being used by the Response API.
   *
   * In most cases, this will be NULL.  However, if proftpd is in the
   * midst of a data transfer when a command comes in on the control
   * connection, then the pool in use will be that of the data transfer
   * instigating command.  We want to stash that pool, so that after this
   * command is dispatched, we can return the pool of the old command.
   * Otherwise, Bad Things (segfaults) happen.
   */
  resp_pool = pr_response_get_pool();

  /* Set the pool used by the Response API for this command. */
  pr_response_set_pool(cmd->pool);

  for (cp = cmd->argv[0]; *cp; cp++)
    *cp = toupper(*cp);

  if (cmd->cmd_class == 0) {
    cmd->cmd_class = get_command_class(cmd->argv[0]);
  }

  if (cmd->cmd_id == 0) {
    cmd->cmd_id = pr_cmd_get_id(cmd->argv[0]);
  }

  if (phase == 0) {
        
    /* First, dispatch to wildcard PRE_CMD handlers. */
    success = _dispatch(cmd, PRE_CMD, FALSE, C_ANY);

    if (!success)	/* run other pre_cmd */
      success = _dispatch(cmd, PRE_CMD, FALSE, NULL);

    if (success < 0) {
      /* Dispatch to POST_CMD_ERR handlers as well. */

      _dispatch(cmd, POST_CMD_ERR, FALSE, C_ANY);
      _dispatch(cmd, POST_CMD_ERR, FALSE, NULL);

      _dispatch(cmd, LOG_CMD_ERR, FALSE, C_ANY);
      _dispatch(cmd, LOG_CMD_ERR, FALSE, NULL);

      xerrno = errno;
      pr_trace_msg("response", 9, "flushing error response list for '%s'",
        cmd->argv[0]);
      pr_response_flush(&resp_err_list);

      /* Restore any previous pool to the Response API. */
      pr_response_set_pool(resp_pool);

      errno = xerrno;
      return success;
    }

    success = _dispatch(cmd, CMD, FALSE, C_ANY);
    if (!success)
      success = _dispatch(cmd, CMD, TRUE, NULL);

    if (success == 1) {
      success = _dispatch(cmd, POST_CMD, FALSE, C_ANY);
      if (!success)
        success = _dispatch(cmd, POST_CMD, FALSE, NULL);

      _dispatch(cmd, LOG_CMD, FALSE, C_ANY);
      _dispatch(cmd, LOG_CMD, FALSE, NULL);

      xerrno = errno;
      pr_trace_msg("response", 9, "flushing response list for '%s'",
        cmd->argv[0]);
      pr_response_flush(&resp_list);

      errno = xerrno;

    } else if (success < 0) {

      /* Allow for non-logging command handlers to be run if CMD fails. */

      success = _dispatch(cmd, POST_CMD_ERR, FALSE, C_ANY);
      if (!success)
        success = _dispatch(cmd, POST_CMD_ERR, FALSE, NULL);

      _dispatch(cmd, LOG_CMD_ERR, FALSE, C_ANY);
      _dispatch(cmd, LOG_CMD_ERR, FALSE, NULL);

      xerrno = errno;
      pr_trace_msg("response", 9, "flushing error response list for '%s'",
        cmd->argv[0]);
      pr_response_flush(&resp_err_list);

      errno = xerrno;
    }

  } else {
    switch (phase) {
      case PRE_CMD:
      case POST_CMD:
      case POST_CMD_ERR:
        success = _dispatch(cmd, phase, FALSE, C_ANY);
        if (!success) {
          success = _dispatch(cmd, phase, FALSE, NULL);
          xerrno = errno;
        }
        break;

      case CMD:
        success = _dispatch(cmd, phase, FALSE, C_ANY);
        if (!success)
          success = _dispatch(cmd, phase, TRUE, NULL);
        break;

      case LOG_CMD:
      case LOG_CMD_ERR:
        (void) _dispatch(cmd, phase, FALSE, C_ANY);
        (void) _dispatch(cmd, phase, FALSE, NULL);
        break;

      default:
        /* Restore any previous pool to the Response API. */
        pr_response_set_pool(resp_pool);

        errno = EINVAL;
        return -1;
    }

    if (flags & PR_CMD_DISPATCH_FL_SEND_RESPONSE) {
      xerrno = errno;

      if (success == 1) {
        pr_trace_msg("response", 9, "flushing response list for '%s'",
          cmd->argv[0]);
        pr_response_flush(&resp_list);

      } else if (success < 0) {
        pr_trace_msg("response", 9, "flushing error response list for '%s'",
          cmd->argv[0]);
        pr_response_flush(&resp_err_list);
      }

      errno = xerrno;
    }
  }

  /* Restore any previous pool to the Response API. */
  pr_response_set_pool(resp_pool);

  errno = xerrno;
  return success;
}

int pr_cmd_dispatch(cmd_rec *cmd) {
  return pr_cmd_dispatch_phase(cmd, 0,
    PR_CMD_DISPATCH_FL_SEND_RESPONSE|PR_CMD_DISPATCH_FL_CLEAR_RESPONSE);
}

#ifdef _QNAP_
static int change_ftp_cmd(char *buf) {
  char *cp = buf, *ck;
  char username[128], remain[PR_TUNABLE_PATH_MAX] = "", newpath[PR_TUNABLE_PATH_MAX] = "/", cmd[32];
  char *check_cmd[32] = {"DELE", "MKD", "MDTM", "NLST", "RMD", "STOU", "RETR", "RNFR", "RNTO", "SIZE", "CWD", "STOR", "LIST", "CDUP", NULL};
  char *str_rest = strchr(cp, ' '), *tmp_ptr;
  int i = 0, cmd_match = 0, ret = 0, relative_path = 0, absolute_path = 0;

  if(!l_home){
    return ret;
  }

  if(!session.cwd || !session.chroot_path){
    return ret;
  }

  tmp_ptr = get_fullhomedir() + strlen(session.chroot_path);
  /* 'Change Directory UP' from home will force 'Change Working Directory' to root */
  if(strcmp(cp, "CDUP") == 0 && strcmp(session.cwd, tmp_ptr) == 0){
    sprintf(newpath, "CWD /");
    strncpy(cp, newpath, strlen(newpath)+1);
    return ret;
  }

  if(!str_rest){
    return ret;
  }
  if(!session.user){
    return ret;
  }
  strncpy(cmd, cp, str_rest - cp);
  cmd[str_rest - cp]  = '\0';
  for(ck = check_cmd[i]; ck; ck = check_cmd[++i]){
    if(strcmp(ck, cmd ) == 0){
      cmd_match = 1;
      break;
    }
  }

  if(!cmd_match)
    return ret;

  str_rest++;
  //pr_log_pri(PR_LOG_NOTICE, "%s %d cp=%s rest=%s scwd=%s cwd=%s\n", __func__, __LINE__, cp, str_rest,session.cwd, pr_fs_getcwd());

  /* 'Change Directory UP' from home will force 'Change Working Directory' to root */
  if(strcmp(cmd, "CWD") == 0 && strcmp(str_rest, "..") == 0 && strcmp(session.cwd, tmp_ptr) == 0){
    strncpy(str_rest, newpath, strlen(newpath)+1);
    return ret;
  }

  if(str_rest[0] == '/'){    // start at root
    str_rest++;
    absolute_path = 1;
  }else if(strcmp(session.cwd, "/") == 0){
    relative_path = 1;
  }

  if(absolute_path || relative_path){
    while(strncmp(str_rest, "../", 3) == 0){
      str_rest+=3;
    }
    tmp_ptr = strchr(str_rest, '/');
    if(!tmp_ptr){
      tmp_ptr = &str_rest[strlen(str_rest)];
    }
    strncpy(username, str_rest, tmp_ptr-str_rest);
    username[tmp_ptr-str_rest] = '\0';
    if(strcmp(username, USER_HOME_SHARE_NAME)){
      return ret;
    }
    strncpy(remain, tmp_ptr, strlen(tmp_ptr));
    tmp_ptr = get_homedir();
    if(tmp_ptr[0] == '/')
      tmp_ptr++;
    sprintf(newpath, "%s%s", tmp_ptr , remain);
    strncpy(str_rest, newpath, strlen(newpath)+1);
    ret = 1;
  }
  return ret;
}
#endif

static cmd_rec *make_ftp_cmd(pool *p, char *buf, int flags) {
  char *cp = buf, *wrd;
  cmd_rec *cmd;
  pool *subpool;
  array_header *tarr;
  int str_flags = PR_STR_FL_PRESERVE_COMMENTS|flags;
#ifdef _QNAP_
  int ret = change_ftp_cmd(cp);
#endif

  /* Be pedantic (and RFC-compliant) by not allowing leading whitespace
   * in an issued FTP command.  Will this cause troubles with many clients?
   */
  if (PR_ISSPACE(buf[0])) {
    return NULL;
  }

  /* Nothing there...bail out. */
  wrd = pr_str_get_word(&cp, str_flags);
  if (wrd == NULL) {
    return NULL;
  }

  subpool = make_sub_pool(p);
  cmd = (cmd_rec *) pcalloc(subpool, sizeof(cmd_rec));
  cmd->pool = subpool;
  cmd->tmp_pool = NULL;
  cmd->stash_index = -1;

  tarr = make_array(cmd->pool, 2, sizeof(char *));

  *((char **) push_array(tarr)) = pstrdup(cmd->pool, wrd);
  cmd->argc++;
  cmd->arg = pstrdup(cmd->pool, cp);

  while ((wrd = pr_str_get_word(&cp, str_flags)) != NULL) {
    *((char **) push_array(tarr)) = pstrdup(cmd->pool, wrd);
    cmd->argc++;
  }

  *((char **) push_array(tarr)) = NULL;
  cmd->argv = (char **) tarr->elts;

  /* This table will not contain that many entries, so a low number
   * of chains should suffice.
   */
  cmd->notes = pr_table_nalloc(cmd->pool, 0, 8);
  return cmd;
}

static void cmd_loop(server_rec *server, conn_t *c) {

  while (TRUE) {
    int res = 0; 
    cmd_rec *cmd = NULL;

    pr_signals_handle();

    res = pr_cmd_read(&cmd);
    if (res < 0) {
      if (PR_NETIO_ERRNO(session.c->instrm) == EINTR) {
        /* Simple interrupted syscall */
        continue;
      }

#ifndef PR_DEVEL_NO_DAEMON
      /* Otherwise, EOF */
      pr_session_disconnect(NULL, PR_SESS_DISCONNECT_CLIENT_EOF, NULL);
#else
      return;
#endif /* PR_DEVEL_NO_DAEMON */
    }

    /* Data received, reset idle timer */
    if (pr_data_get_timeout(PR_DATA_TIMEOUT_IDLE) > 0) {
      pr_timer_reset(PR_TIMER_IDLE, ANY_MODULE);
    }

    if (cmd) {

      /* Detect known commands for other protocols; if found, drop the
       * connection, lest we be used as part of an attack on a different
       * protocol server (Bug#4143).
       */
      if (cmd->is_ftp == FALSE) {
        pr_log_pri(PR_LOG_WARNING,
          "client sent %s command '%s', disconnecting", cmd->protocol,
          cmd->argv[0]);
        pr_event_generate("core.bad-protocol", cmd);
        pr_session_disconnect(NULL, PR_SESS_DISCONNECT_BAD_PROTOCOL,
          cmd->protocol);
      }
 
      pr_cmd_dispatch(cmd);
      destroy_pool(cmd->pool);

    } else {
      pr_event_generate("core.invalid-command", NULL);
      pr_response_send(R_500, _("Invalid command: try being more creative"));
    }

    /* Release any working memory allocated in inet */
    pr_inet_clear();
  }
}

static void core_restart_cb(void *d1, void *d2, void *d3, void *d4) {
  if (is_master && mpid) {
    int maxfd;
    fd_set childfds;
    struct timeval restart_start, restart_finish;
    long restart_elapsed = 0;

    pr_log_pri(PR_LOG_NOTICE, "received SIGHUP -- master server reparsing "
      "configuration file");

    gettimeofday(&restart_start, NULL);

#ifdef _NVR_
    read_nvr_config();
#endif

    /* Make sure none of our children haven't completed start up */
    FD_ZERO(&childfds);
    maxfd = -1;

    maxfd = semaphore_fds(&childfds, maxfd);
    if (maxfd > -1) {
      pr_log_pri(PR_LOG_NOTICE, "waiting for child processes to complete "
        "initialization");

      while (maxfd != -1) {
	int i;
	
	i = select(maxfd + 1, &childfds, NULL, NULL, NULL);

        if (i > 0) {
          pr_child_t *ch;

          for (ch = child_get(NULL); ch; ch = child_get(ch)) {
            if (ch->ch_pipefd != -1 &&
               FD_ISSET(ch->ch_pipefd, &childfds)) {
              (void) close(ch->ch_pipefd);
              ch->ch_pipefd = -1;
            }
          }
        }

	FD_ZERO(&childfds);
        maxfd = -1;
	maxfd = semaphore_fds(&childfds, maxfd);
      }
    }

    free_bindings();

    /* Run through the list of registered restart callbacks. */
    pr_event_generate("core.restart", NULL);

    init_log();
    init_netaddr();
    init_class();
    init_config();

#ifdef PR_USE_NLS
    encode_free();
#endif /* PR_USE_NLS */

    pr_netaddr_clear_cache();

    pr_parser_prepare(NULL, NULL);

    pr_event_generate("core.preparse", NULL);

    PRIVS_ROOT
    if (pr_parser_parse_file(NULL, config_filename, NULL, 0) == -1) {
      int xerrno = errno;

      PRIVS_RELINQUISH
      pr_log_pri(PR_LOG_WARNING,
        "fatal: unable to read configuration file '%s': %s", config_filename,
        strerror(xerrno));
      pr_session_end(0);
    }
    PRIVS_RELINQUISH

    if (pr_parser_cleanup() < 0) {
      pr_log_pri(PR_LOG_WARNING,
        "fatal: error processing configuration file '%s': "
        "unclosed configuration section", config_filename);
      pr_session_end(0);
    }

#ifdef PR_USE_NLS
    encode_init();
#endif /* PR_USE_NLS */

    /* After configuration is complete, make sure that passwd, group
     * aren't held open (unnecessary fds for master daemon)
     */
    endpwent();
    endgrent();

    if (fixup_servers(server_list) < 0) {
      pr_log_pri(PR_LOG_WARNING,
        "fatal: error processing configuration file '%s'", config_filename);
      pr_session_end(0);
    }

    pr_event_generate("core.postparse", NULL);

    /* Recreate the listen connection.  Can an inetd-spawned server accept
     * and process HUP?
     */
    init_bindings();

    gettimeofday(&restart_finish, NULL);

    restart_elapsed = ((restart_finish.tv_sec - restart_start.tv_sec) * 1000L) +
      ((restart_finish.tv_usec - restart_start.tv_usec) / 1000L);
    pr_trace_msg("config", 12, "restart took %ld millisecs", restart_elapsed);

#ifdef _QNAP_
{
	pr_child_t *ch = child_get(NULL);
	if (ch) {
		//Tell the first child to reconfig
		kill(ch->ch_pid, SIGHUP);
	}
}
#endif
  } else {

    /* Child process -- cannot restart, log error */
    pr_log_pri(PR_LOG_ERR, "received SIGHUP, cannot restart child process");
  }
}

#ifndef PR_DEVEL_NO_FORK
static int dup_low_fd(int fd) {
  int i, need_close[3] = {-1, -1, -1};

  for (i = 0; i < 3; i++) {
    if (fd == i) {
      fd = dup(fd);
      fcntl(fd, F_SETFD, FD_CLOEXEC);

      need_close[i] = 1;
    }
  }

  for (i = 0; i < 3; i++) {
    if (need_close[i] > -1) {
      (void) close(i);
    }
  }

  return fd;
}
#endif /* PR_DEVEL_NO_FORK */

static void set_server_privs(void) {
  uid_t server_uid, current_euid = geteuid();
  gid_t server_gid, current_egid = getegid();
  unsigned char switch_server_id = FALSE;

  uid_t *uid = get_param_ptr(main_server->conf, "UserID", FALSE);
  gid_t *gid =  get_param_ptr(main_server->conf, "GroupID", FALSE);

  if (uid) {
    server_uid = *uid;
    switch_server_id = TRUE;

  } else
    server_uid = current_euid;

  if (gid) {
    server_gid = *gid;
    switch_server_id = TRUE;

  } else
    server_gid = current_egid;

  if (switch_server_id) {
    PRIVS_ROOT

    /* Note: will it be necessary to double check this switch, as is done
     * in elsewhere in this file?
     */
    PRIVS_SETUP(server_uid, server_gid);
  }
}
#ifdef _QNAP_DEBUG_
static void walk_config_limit_set(xaset_t *set)
{
	config_rec *lc = NULL;
	int i;

	if (!set)
		return;
	for (lc = (config_rec *) set->xas_list; lc; lc = lc->next) {
		pr_log_pri(PR_LOG_DEBUG, "%s:%d::config_type is %x, argc is %d", __FUNCTION__, __LINE__, lc->config_type, lc->argc);
		if (lc->config_type == CONF_LIMIT) {
			for (i = 0; i < lc->argc; i++) {
				pr_log_pri(PR_LOG_DEBUG, "%s:%d::lc=%p, argv[%d]=(%s)", __FUNCTION__, __LINE__, lc, i, lc->argv[i]);
			}
		}
	}
}
static void walk_config_limit(config_rec *c)
{
	for (; c ; c = c->parent) {
		walk_config_limit_set(c->subset);
	}
//	walk_config_rec(main_server->conf);
}
void walk_config_dir(xaset_t *s, const char *path)
{
	config_rec *c = NULL;
	pr_log_pri(PR_LOG_DEBUG, "%s(%p) Begin", __FUNCTION__, s);
	for (c = (config_rec *) s->xas_list; c; c = c->next) {
		pr_log_pri(PR_LOG_DEBUG, "%s:%d::config_type(%p, %x, %s)", __FUNCTION__, __LINE__, c, c->config_type, (c->name)?c->name:"NULL");
		if (c->config_type == CONF_DIR) {
			if (strcasestr(c->name, path) == NULL) {
				if (c->subset) {
					walk_config_dir(c->subset, path);
				}
				continue;
			}
			walk_config_limit(c);
		}
	}
	pr_log_pri(PR_LOG_DEBUG, "%s(%p) End", __FUNCTION__, s);
}
void walk_config_dirs(xaset_t *s)
{
	config_rec *c = NULL;
	pr_log_pri(PR_LOG_DEBUG, "%s(%p) Begin", __FUNCTION__, s);
	for (c = (config_rec *) s->xas_list; c; c = c->next) {
		pr_log_pri(PR_LOG_DEBUG, "%s:%d::c=%p, config_type(%x, %s)", __FUNCTION__, __LINE__, c, c->config_type, c->name);
		if (c->config_type == CONF_DIR) {
			walk_config_limit(c);
			if (c->subset) {
				walk_config_dirs(c->subset);
			}
		}
	}
	pr_log_pri(PR_LOG_DEBUG, "%s(%p) End", __FUNCTION__, s);
}
void dump_config(char *prefix, config_rec *c)
{
	pr_log_pri(PR_LOG_DEBUG, "%s<%s, %d, %d>", prefix, c->name, c->config_type, c->config_id);
	pr_log_pri(PR_LOG_DEBUG, "%s[%p, %p, %p][%p, %p](%d)(%x)[%p, %p, %p]",  prefix,
		c->prev, c, c->next,
		c->pool, c->set,
		c->argc, c->flags,
		c->server, c->parent, c->subset);
	if (c->subset) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%s\t", prefix);
		walk_configs(buffer, c->subset);
	}
	pr_log_pri(PR_LOG_DEBUG, "%s</%s, %d, %d>", prefix, c->name, c->config_type, c->config_id);
}
void walk_configs(char *prefix, xaset_t *s)
{
	config_rec *c = NULL;
//	pr_log_pri(PR_LOG_DEBUG, "%s(%p) Begin", __FUNCTION__, s);
	for (c = (config_rec *) s->xas_list; c; c = c->next) {
		dump_config(prefix, c);
	}
//	pr_log_pri(PR_LOG_DEBUG, "%s(%p) End", __FUNCTION__, s);
}
#endif //_QNAP_DEBUG_

char *dir_best_path_child(pool *p, const char *path)
{
	char buffer[260];
	char *root_dir = session.chroot_path;
	char *relate_dir;
	int root_len = snprintf(buffer, sizeof(buffer), "%s/", root_dir);
    if(custom_root) {
        if(strcmp(custom_root_path, path))
            return NULL;
        return pstrdup(p, session.chroot_path);
    }
    else if (strncmp(buffer, path, root_len) != 0) {
		return NULL;
	}
	struct stat st;

	relate_dir = path+(root_len-1);
	if (stat(relate_dir, &st) != 0) {
		return NULL;
	}
	//pr_log_pri(PR_LOG_DEBUG, "%s::%s(st.st_mode=%x)", __FUNCTION__, relate_dir, st.st_mode);
	if (pr_fsio_readlink(relate_dir, buffer, sizeof(buffer)) != -1) {
		char realpath[260];
		snprintf(realpath, sizeof(realpath), "%s/%s", root_dir, buffer);
		return pstrdup(p, realpath);
	}
    else { // Jacky 20140911 : FIX: /share/homes/user -> /share/CACHEDEV1_DATA/homes/user
        char *home = get_homedir();
        if(strncmp(relate_dir, home, strlen(home)) == 0) {
            char *fhome = get_fullhomedir();
            pr_log_pri(PR_LOG_DEBUG, "path=%s => fullhome=%s", path, fhome);
            return pstrdup(p, fhome);
        }
    }
	return pstrdup(p, path);
}
void resolve_deferred_dirs_child(xaset_t *conf)
{
	config_rec *c;

	if (!conf)
		return;

	for (c = (config_rec *) conf->xas_list; c; c = c->next) {
		if (c->config_type == CONF_DIR && (c->flags & CF_DEFER)) {
			char *realdir;
			// Jacky 20140911 : FIX: replace %H to user home dir
            c->name = path_subst_uservar(c->pool, &c->name);
			realdir = dir_best_path_child(c->pool, c->name);
			pr_log_pri(PR_LOG_DEBUG, "%s(%s, %s)", __FUNCTION__, c->name, (realdir)?realdir:"NULL");
			if (realdir) {
				c->name = realdir;
			}
			if (c->subset) {
				resolve_deferred_dirs_child(c->subset);
			}
		}
	}
}

//static RETSIGTYPE sig_child_reconfig(int signo)
void sig_child_reconfig(int signo, siginfo_t *sinfo, void *p)
{
	int main_pid = getppid();
	if (sinfo->si_pid != main_pid) {
		//Child only access SIGHUP from main process
		pr_log_pri(PR_LOG_DEBUG, "%d:%s::Got signal from pid(%d) not main process", getpid(), __FUNCTION__, sinfo->si_pid, main_pid);
		return ;
	}
	pr_log_pri(PR_LOG_DEBUG, "%d:%s::Get signal(%d)", getpid(), __FUNCTION__, signo);
#ifdef _QNAP_DEBUG_
	walk_configs("", main_server->conf);
//	walk_config_dir(main_server->conf, "Qdownload");
//	pr_log_pri(PR_LOG_DEBUG, "%d:%s::Reconfig", getpid(), __FUNCTION__);
#endif //_QNAP_DEBUG_
	init_log();
	init_netaddr();
	init_class();
	init_config();

#ifdef PR_USE_NLS
	encode_free();
#endif /* PR_USE_NLS */

	pr_netaddr_clear_cache();

	session.anon_config = NULL;
	session.dir_config = NULL;

	pr_parser_prepare(NULL, NULL);

	PRIVS_ROOT
	if (pr_parser_parse_file(NULL, config_filename, NULL, 0) == -1) {
		PRIVS_RELINQUISH
		pr_log_pri(PR_LOG_ERR,
	        "Fatal: unable to read configuration file '%s': %s",
	        config_filename, strerror(errno));
		kill(main_pid, SIGHUP);		//Tell main process handle next child
		end_login(1);
	}
	PRIVS_RELINQUISH

	if (pr_parser_cleanup() < 0) {
		pr_log_pri(PR_LOG_ERR, "Fatal: error processing configuration file '%s': "
	       "unclosed configuration section", config_filename);
		kill(main_pid, SIGHUP);		//Tell main process handle next child
		end_login(1);
	}

#ifdef PR_USE_NLS
	encode_init();
#endif /* PR_USE_NLS */

	/* After configuration is complete, make sure that passwd, group
        * aren't held open (unnecessary fds for master daemon)
        */
	endpwent();
	endgrent();

	if (fixup_servers(server_list) < 0) {
		pr_log_pri(PR_LOG_ERR, "Fatal: error processing configuration file '%s'",
	        config_filename);
		kill(main_pid, SIGHUP);		//Tell main process handle next child
		end_login(1);
	}

	resolve_deferred_dirs_child(main_server->conf);
	// Jacky 20140905 - Fixed for reorder the DIRs
	fixup_dirs(main_server, CF_DEFER);

	pr_log_pri(PR_LOG_DEBUG, "%s() end", __FUNCTION__);
#ifdef _QNAP_DEBUG_
	walk_config_dir(main_server->conf, "Qdownload");
#endif //_QNAP_DEBUG_
//	signal(SIGHUP, sig_child_reconfig);
	kill(main_pid, SIGHUP);		//Tell main process handle next child
}

static void fork_server(int fd, conn_t *l, unsigned char nofork) {
  conn_t *conn = NULL;
  int i, rev;
  int semfds[2] = { -1, -1 };
  int xerrno = 0;

#ifndef PR_DEVEL_NO_FORK
  pid_t pid;
  sigset_t sig_set;

  if (!nofork) {

    /* A race condition exists on heavily loaded servers where the parent
     * catches SIGHUP and attempts to close/re-open the main listening
     * socket(s), however the children haven't finished closing them
     * (EADDRINUSE).  We use a semaphore pipe here to flag the parent once
     * the child has closed all former listening sockets.
     */

    if (pipe(semfds) == -1) {
      pr_log_pri(PR_LOG_ALERT, "pipe(2) failed: %s", strerror(errno));
      (void) close(fd);
      return;
    }

    /* Need to make sure the child (writer) end of the pipe isn't
     * < 2 (stdio/stdout/stderr) as this will cause problems later.
     */
    if (semfds[1] < 3) {
      semfds[1] = dup_low_fd(semfds[1]);
    }

    /* Make sure we set the close-on-exec flag for the parent's read side
     * of the pipe.
     */
    (void) fcntl(semfds[0], F_SETFD, FD_CLOEXEC);

    /* We block SIGCHLD to prevent a race condition if the child
     * dies before we can record it's pid.  Also block SIGTERM to
     * prevent sig_terminate() from examining the child list
     */

    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGTERM);
    sigaddset(&sig_set, SIGCHLD);
    sigaddset(&sig_set, SIGUSR1);
    sigaddset(&sig_set, SIGUSR2);

    if (sigprocmask(SIG_BLOCK, &sig_set, NULL) < 0) {
      pr_log_pri(PR_LOG_NOTICE,
        "unable to block signal set: %s", strerror(errno));
    }

    pid = fork();
    xerrno = errno;

    switch (pid) {

    case 0: /* child */
      /* No longer the master process. */
      is_master = FALSE;
      if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
        pr_log_pri(PR_LOG_NOTICE,
          "unable to unblock signal set: %s", strerror(errno));
      }

      /* No longer need the read side of the semaphore pipe. */
      (void) close(semfds[0]);
      break;

    case -1:
      if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
        pr_log_pri(PR_LOG_NOTICE,
          "unable to unblock signal set: %s", strerror(errno));
      }

      pr_log_pri(PR_LOG_ALERT, "unable to fork(): %s", strerror(xerrno));

      /* The parent doesn't need the socket open. */
      (void) close(fd);
      (void) close(semfds[0]);
      (void) close(semfds[1]);

      return;

    default: /* parent */
      /* The parent doesn't need the socket open */
      (void) close(fd);

      child_add(pid, semfds[0]);
      (void) close(semfds[1]);

      /* Unblock the signals now as sig_child() will catch
       * an "immediate" death and remove the pid from the children list
       */
      if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
        pr_log_pri(PR_LOG_NOTICE,
          "unable to unblock signal set: %s", strerror(errno));
      }

      return;
    }
  }

  session.pid = getpid();

  /* No longer need any listening fds. */
  pr_ipbind_close_listeners();

  /* There would appear to be no useful purpose behind setting the process
   * group of the newly forked child.  In daemon/inetd mode, we should have no
   * controlling tty and either have the process group of the parent or of
   * inetd.  In non-daemon mode (-n), doing this may cause SIGTTOU to be
   * raised on output to the terminal (stderr logging).
   *
   * #ifdef HAVE_SETPGID
   *   setpgid(0,getpid());
   * #else
   * # ifdef SETPGRP_VOID
   *   setpgrp();
   * # else
   *   setpgrp(0,getpid());
   * # endif
   * #endif
   *
   */

  /* Reseed pseudo-randoms */
  srand((unsigned int) (time(NULL) * getpid()));

#endif /* PR_DEVEL_NO_FORK */

  /* Child is running here */
  if (signal(SIGUSR1, sig_disconnect) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGUSR1 (signal %d) handler: %s", SIGUSR1,
      strerror(errno));
  }

  if (signal(SIGUSR2, sig_evnt) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGUSR2 (signal %d) handler: %s", SIGUSR2,
      strerror(errno));
  }

  if (signal(SIGCHLD, SIG_DFL) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGCHLD (signal %d) handler: %s", SIGCHLD,
      strerror(errno));
  }

#ifdef _QNAP_
//  signal(SIGHUP, sig_child_reconfig);
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sig_child_reconfig;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGHUP, &act, NULL);
}
#else
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGHUP (signal %d) handler: %s", SIGHUP,
      strerror(errno));
  }
#endif

  /* From this point on, syslog stays open. We close it first so that the
   * logger will pick up our new PID.
   *
   * We have to delay calling log_opensyslog() until after pr_inet_openrw()
   * is called, otherwise the potential exists for the syslog FD to
   * be overwritten and the user to see logging information.
   *
   * This isn't that big of a deal because the logging functions will
   * just open it dynamically if they need to.
   */
  log_closesyslog();

  /* Specifically DO NOT perform reverse DNS at this point, to alleviate
   * the race condition mentioned above.  Instead we do it after closing
   * all former listening sockets.
   */
  conn = pr_inet_openrw(permanent_pool, l, NULL, PR_NETIO_STRM_CTRL, fd,
    STDIN_FILENO, STDOUT_FILENO, FALSE);

  /* Capture errno here, if necessary. */
  if (conn == NULL) {
    xerrno = errno;
  }

  /* Now do the permanent syslog open
   */
  pr_signals_block();
  PRIVS_ROOT

  log_opensyslog(NULL);

  PRIVS_RELINQUISH
  pr_signals_unblock();

  if (conn == NULL) {
    /* There are some errors, e.g. ENOTCONN ("Transport endpoint is not
     * connected") which can easily happen, as during scans/TCP
     * probes/healthchecks, commonly done by load balancers, firewalls, and
     * other clients.  By the time proftpd reaches the point of looking up
     * the peer data for that connection, the client has disconnected.
     *
     * These are normal errors, and thus should not be logged as fatal
     * conditions.
     */
    if (xerrno == ENOTCONN ||
        xerrno == ECONNABORTED ||
        xerrno == ECONNRESET) {
      pr_log_pri(PR_LOG_DEBUG, "unable to open incoming connection: %s",
        strerror(xerrno));

    } else {
      pr_log_pri(PR_LOG_ERR, "fatal: unable to open incoming connection: %s",
        strerror(xerrno));
    }

    exit(1);
  }

  pr_event_generate("core.connect", conn);

  /* Find the server for this connection. */
  main_server = pr_ipbind_get_server(conn->local_addr, conn->local_port);

  /* Make sure we allocate a session pool, even if this connection will
   * dropped soon.
   */
  session.pool = make_sub_pool(permanent_pool);
  pr_pool_tag(session.pool, "Session Pool");

  session.c = conn;
  session.data_port = conn->remote_port - 1;
  session.sf_flags = 0;
  session.sp_flags = 0;
  session.proc_prefix = "(connecting)";

  /* If no server is configured to handle the addr the user is connected to,
   * drop them.
   */
  if (main_server == NULL) {
    pr_log_debug(DEBUG2, "No server configuration found for IP address %s",
      pr_netaddr_get_ipstr(conn->local_addr));
    pr_log_debug(DEBUG2, "Use the DefaultServer directive to designate "
      "a default server configuration to handle requests like this");

    pr_response_send(R_500,
      _("Sorry, no server available to handle request on %s"),
      pr_netaddr_get_dnsstr(conn->local_addr));
    exit(0);
  }

  pr_inet_set_proto_opts(permanent_pool, conn, 0, 1, IPTOS_LOWDELAY, 0);

  /* Close the write side of the semaphore pipe to tell the parent
   * we are all grown up and have finished housekeeping (closing
   * former listen sockets).
   */
  close(semfds[1]);

  /* Now perform reverse DNS lookups. */
  if (ServerUseReverseDNS) {
    rev = pr_netaddr_set_reverse_dns(ServerUseReverseDNS);

    if (conn->remote_addr)
      conn->remote_name = pr_netaddr_get_dnsstr(conn->remote_addr);

    pr_netaddr_set_reverse_dns(rev);
  }

  pr_netaddr_set_sess_addrs();

  /* Check and see if we are shutdown */
  if (shutdownp) {
    time_t now;

    time(&now);
    if (!deny || deny <= now) {
      config_rec *c = NULL;
      char *reason = NULL;
      const char *serveraddress;

      serveraddress = (session.c && session.c->local_addr) ?
        pr_netaddr_get_ipstr(session.c->local_addr) :
        main_server->ServerAddress;

      c = find_config(main_server->conf, CONF_PARAM, "MasqueradeAddress",
        FALSE);
      if (c != NULL) {
        pr_netaddr_t *masq_addr = (pr_netaddr_t *) c->argv[0];
        serveraddress = pr_netaddr_get_ipstr(masq_addr);
      }

      reason = sreplace(permanent_pool, shutmsg,
                   "%s", pstrdup(permanent_pool, pr_strtime(shut)),
                   "%r", pstrdup(permanent_pool, pr_strtime(deny)),
                   "%d", pstrdup(permanent_pool, pr_strtime(disc)),
		   "%C", (session.cwd[0] ? session.cwd : "(none)"),
		   "%L", serveraddress,
		   "%R", (session.c && session.c->remote_name ?
                         session.c->remote_name : "(unknown)"),
		   "%T", pstrdup(permanent_pool, pr_strtime(now)),
		   "%U", "NONE",
		   "%V", main_server->ServerName,
                   NULL );

      pr_log_auth(PR_LOG_NOTICE, "connection refused (%s) from %s [%s]",
               reason, session.c->remote_name,
               pr_netaddr_get_ipstr(session.c->remote_addr));

      pr_response_send(R_500,
        _("FTP server shut down (%s) -- please try again later"), reason);
      exit(0);
    }
  }

  if (main_server->listen) {
    if (main_server->listen->listen_fd == conn->rfd ||
        main_server->listen->listen_fd == conn->wfd) {
      main_server->listen->listen_fd = -1;
    }

    main_server->listen = NULL;
  }

  /* Set the ID/privs for the User/Group in this server */
  set_server_privs();

  /* Find the class for this session. */
  session.conn_class = pr_class_match_addr(session.c->remote_addr);
  if (session.conn_class != NULL) {
    pr_log_debug(DEBUG2, "session requested from client in '%s' class",
      session.conn_class->cls_name);

  } else {
    pr_log_debug(DEBUG5, "session requested from client in unknown class");
  }

  /* Check config tree for <Limit LOGIN> directives.  Do not perform
   * this check until after the class of the session has been determined,
   * in order to properly handle any AllowClass/DenyClass directives
   * within the <Limit> section.
   */
  if (!login_check_limits(main_server->conf, TRUE, FALSE, &i)) {
    pr_log_pri(PR_LOG_NOTICE, "Connection from %s [%s] denied",
      session.c->remote_name,
      pr_netaddr_get_ipstr(session.c->remote_addr));
    exit(0);
  }

  /* Create a table for modules to use. */
  session.notes = pr_table_alloc(session.pool, 0);
  if (session.notes == NULL) {
    pr_log_debug(DEBUG3, "error creating session.notes table: %s",
      strerror(errno));
  }

  /* Prepare the Timers API. */
  timers_init();

  /* Inform all the modules that we are now a child */
  pr_log_debug(DEBUG7, "performing module session initializations");
  if (modules_session_init() < 0) {
    pr_session_disconnect(NULL, PR_SESS_DISCONNECT_SESSION_INIT_FAILED, NULL);
  }

  pr_log_debug(DEBUG4, "connected - local  : %s:%d",
    pr_netaddr_get_ipstr(session.c->local_addr), session.c->local_port);
  pr_log_debug(DEBUG4, "connected - remote : %s:%d",
    pr_netaddr_get_ipstr(session.c->remote_addr), session.c->remote_port);

  pr_proctitle_set("connected: %s (%s:%d)",
    session.c->remote_name ? session.c->remote_name : "?",
    session.c->remote_addr ? pr_netaddr_get_ipstr(session.c->remote_addr) : "?",
    session.c->remote_port ? session.c->remote_port : 0);

  pr_log_pri(PR_LOG_INFO, "%s session opened.",
    pr_session_get_protocol(PR_SESS_PROTO_FL_LOGOUT));

  /* Make sure we can receive OOB data */
  pr_inet_set_async(session.pool, session.c);

#ifdef _QNAP_
  // Jacky 20140911 - for QoS module to set TOS on control socket
  pr_inet_generate_socket_event("core.ctrl-listen", main_server, conn->remote_addr, conn->wfd);
#endif

  pr_session_send_banner(main_server,
    PR_DISPLAY_FL_NO_EOM|PR_DISPLAY_FL_SEND_NOW);

  cmd_handler(main_server, conn);

#ifdef PR_DEVEL_NO_DAEMON
  /* Cleanup */
#ifdef _QNAP_
  if(session.user && session.c->remote_name)
    SendConnToLogEngine(EVENT_TYPE_INFO, session.user, (char *)session.c->remote_name, "", CONN_SERV_FTP, CONN_ACTION_LOGOUT, "---");
#endif
  pr_session_end(PR_SESS_END_FL_NOEXIT);
  main_server = NULL;
  free_pools();
  pr_proctitle_free();
#endif /* PR_DEVEL_NO_DAEMON */
}

static void disc_children(void) {

  if (disc && disc <= time(NULL) && child_count()) {
    sigset_t sig_set;

    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGTERM);
    sigaddset(&sig_set, SIGCHLD);
    sigaddset(&sig_set, SIGUSR1);
    sigaddset(&sig_set, SIGUSR2);

    if (sigprocmask(SIG_BLOCK, &sig_set, NULL) < 0) {
      pr_log_pri(PR_LOG_NOTICE,
        "unable to block signal set: %s", strerror(errno));
    }

    PRIVS_ROOT
    child_signal(SIGUSR1);
    PRIVS_RELINQUISH

    if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
      pr_log_pri(PR_LOG_NOTICE,
        "unable to unblock signal set: %s", strerror(errno));
    }
  }
}
// Modified by Jeff Chang 2011/5/4, for service binding
#ifdef _QNAP_
static void Release_Bind_Addr(PT_BIND_ADDR ptbaAddr)
{
	if(!ptbaAddr) return;

	if(ptbaAddr->ryszIpv4)
	{
		int idx;
		for(idx=0; idx<ptbaAddr->cnIpv4; idx++)
			free(ptbaAddr->ryszIpv4[idx]);
		free(ptbaAddr->ryszIpv4);
		ptbaAddr->ryszIpv4 = NULL;
	}
	if(ptbaAddr->ryszIpv6)
	{
		int idx;
		for(idx=0; idx<ptbaAddr->cnIpv6; idx++)
			free(ptbaAddr->ryszIpv6[idx]);
		free(ptbaAddr->ryszIpv6);
		ptbaAddr->ryszIpv6 = NULL;
	}
}

/**
 * \brief	Get the service binding address.
 * \param	ptBindAddr	The buffer to the service binding settings.
 * \return	0 if function success, or -XXX if error.
 */
static int Get_Binding_Address(PT_BIND_ADDR ptBindAddr)
{
	int  iRet=-ENOMEM, cbAddr;
	char  szLine[256];
	FILE  *pfAddr = NULL;
	struct stat  tStat;

	// Reset the inet address first.
	ptBindAddr->bEnabled = FALSE;
	ptBindAddr->cnIpv4 = ptBindAddr->cnIpv6 = 0;
    Release_Bind_Addr(ptBindAddr);
	// Just report no binding address if the getbindaddr is not existed.
	if (-1 == stat(SZP_GET_BIND_ADDR, &tStat))  return 0;
	//if (NULL == (pfAddr = popen(SZP_GET_BIND_ADDR " FTP 2>/dev/null", "r")))  return 0;
    qSystem(SZP_GET_BIND_ADDR " FTP > /tmp/ftp-bind.txt 2>/dev/null");
    if (NULL == (pfAddr = fopen("/tmp/ftp-bind.txt", "r"))) return 0;
	while (NULL != fgets(szLine, sizeof(szLine), pfAddr))
	{
		szLine[sizeof(szLine)-1] = '\0';
		if (0 >= (cbAddr = strlen(szLine)))  continue;
		if ('\n' == szLine[cbAddr-1])  szLine[cbAddr-1] = '\0';
		if (NULL != strchr(szLine, ':'))
		{
			if(NULL == (ptBindAddr->ryszIpv6 = (char**)realloc(ptBindAddr->ryszIpv6, (++ptBindAddr->cnIpv6)*sizeof(char*))))
			{
				ptBindAddr->cnIpv6 = 0;
                goto _exit_;
			}
			if(NULL == (ptBindAddr->ryszIpv6[ptBindAddr->cnIpv6-1] = (char*)malloc(INET6_ADDRSTRLEN)))
			{
				ptBindAddr->cnIpv6 = 0;
                goto _exit_;
			}
			strcpy(ptBindAddr->ryszIpv6[ptBindAddr->cnIpv6-1], szLine);
		}
		else if (NULL != strchr(szLine, '.'))
		{
			if (strstr(SZ_INADDR_ANY, szLine))
			{
				ptBindAddr->bEnabled = FALSE;
				ptBindAddr->cnIpv4 = ptBindAddr->cnIpv6 = 0;
				break;
			}
			if(NULL == (ptBindAddr->ryszIpv4 = (char**)realloc(ptBindAddr->ryszIpv4, (++ptBindAddr->cnIpv4)*sizeof(char*)*INET6_ADDRSTRLEN)))
			{
				ptBindAddr->cnIpv4 = 0;
                goto _exit_;
			}
			if(NULL == (ptBindAddr->ryszIpv4[ptBindAddr->cnIpv4-1] = (char*)malloc(INET_ADDRSTRLEN)))
			{
				ptBindAddr->cnIpv4 = 0;
                goto _exit_;
			}
			strcpy(ptBindAddr->ryszIpv4[ptBindAddr->cnIpv4-1], szLine);
		}
	}
    iRet = 0;
_exit_:
    if(iRet != 0)
        Release_Bind_Addr(ptBindAddr);
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
#endif //_QNAP_
static void daemon_loop(void) {
  fd_set listenfds;
  conn_t *listen_conn;
  int fd, maxfd;
  int i, err_count = 0, xerrno = 0;
  unsigned long nconnects = 0UL;
  time_t last_error;
  struct timeval tv;
  static int running = 0;
// Modified by Jeff Chang 2011/5/4, for service binding
#ifdef _QNAP_
  char  szIpAddr[INET6_ADDRSTRLEN];
  T_BIND_ADDR tbaDaemon = {0};
  Get_Binding_Address(&tbaDaemon);
#endif
  pr_proctitle_set("(accepting connections)");

  time(&last_error);

  while (TRUE) {
    run_schedule();

    FD_ZERO(&listenfds);
    maxfd = pr_ipbind_listen(&listenfds);

    /* Monitor children pipes */
    maxfd = semaphore_fds(&listenfds, maxfd);

    /* Check for ftp shutdown message file */
    switch (check_shutmsg(&shut, &deny, &disc, shutmsg, sizeof(shutmsg))) {
      case 1:
        if (!shutdownp)
          disc_children();
        shutdownp = 1;
        break;

      case 0:
        shutdownp = 0;
        deny = disc = (time_t) 0;
        break;
    }

    if (shutdownp) {
      tv.tv_sec = 5L;
      tv.tv_usec = 0L;

    } else {

      tv.tv_sec = PR_TUNABLE_SELECT_TIMEOUT;
      tv.tv_usec = 0L;
    }

    /* If running (a flag signaling whether proftpd is just starting up)
     * AND shutdownp (a flag signalling the present of /etc/shutmsg) are
     * true, then log an error stating this -- but don't stop the server.
     */
    if (shutdownp && !running) {

      /* Check the value of the deny time_t struct w/ the current time.
       * If the deny time has passed, log that all incoming connections
       * will be refused.  If not, note the date at which they will be
       * refused in the future.
       */
      time_t now = time(NULL);

      if (difftime(deny, now) < 0.0) {
        pr_log_pri(PR_LOG_WARNING, PR_SHUTMSG_PATH
          " present: all incoming connections will be refused");

      } else {
        pr_log_pri(PR_LOG_NOTICE,
          PR_SHUTMSG_PATH " present: incoming connections "
          "will be denied starting %s", CHOP(ctime(&deny)));
      }
    }

    running = 1;

    PR_DEVEL_CLOCK(i = select(maxfd + 1, &listenfds, NULL, NULL, &tv));
    if (i < 0) {
      xerrno = errno;
    }

    if (i == -1 && xerrno == EINTR) {
      pr_signals_handle();
      continue;
    }

    if (have_dead_child) {
      sigset_t sig_set;

      sigemptyset(&sig_set);
      sigaddset(&sig_set, SIGCHLD);
      sigaddset(&sig_set, SIGTERM);
      pr_alarms_block();
      if (sigprocmask(SIG_BLOCK, &sig_set, NULL) < 0) {
        pr_log_pri(PR_LOG_NOTICE,
          "unable to block signal set: %s", strerror(errno));
      }

      have_dead_child = FALSE;
      child_update();

      if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
        pr_log_pri(PR_LOG_NOTICE,
          "unable to unblock signal set: %s", strerror(errno));
      }

      pr_alarms_unblock();
    }

    if (i == -1) {
      time_t this_error;

      time(&this_error);

      if ((this_error - last_error) <= 5 && err_count++ > 10) {
        pr_log_pri(PR_LOG_ERR, "fatal: select(2) failing repeatedly, shutting "
          "down");
        exit(1);

      } else if ((this_error - last_error) > 5) {
        last_error = this_error;
        err_count = 0;
      }

      pr_log_pri(PR_LOG_WARNING, "select(2) failed in daemon_loop(): %s",
        strerror(xerrno));
    }

    if (i == 0)
      continue;

    /* Reset the connection counter.  Take into account this current
     * connection, which does not (yet) have an entry in the child list.
     */
    nconnects = 1UL;

    /* See if child semaphore pipes have signaled */
    if (child_count()) {
      pr_child_t *ch;
      time_t now = time(NULL);

      for (ch = child_get(NULL); ch; ch = child_get(ch)) {
	if (ch->ch_pipefd != -1 &&
            FD_ISSET(ch->ch_pipefd, &listenfds)) {
	  (void) close(ch->ch_pipefd);
	  ch->ch_pipefd = -1;
	}

        /* While we're looking, tally up the number of children forked in
         * the past interval.
         */
        if (ch->ch_when >= (now - (unsigned long) max_connect_interval))
          nconnects++;
      }
    }

    pr_signals_handle();

    if (i < 0) {
      continue;
    }

    /* Accept the connection. */
    listen_conn = pr_ipbind_accept_conn(&listenfds, &fd);
// Modified by Jeff Chang 2011/5/4, for service binding
#ifdef _QNAP_
	if(g_bHup)
	{
		Get_Binding_Address(&tbaDaemon);
		g_bHup = FALSE;
	}
	// Check if the incomming interface matched the service binding settings.
	if (0 != Check_Binding_Address(fd, &tbaDaemon, szIpAddr))
	{
		close(fd);
		continue;
	}
#endif //_QNAP_

    /* Fork off servers to handle each connection our job is to get back to
     * answering connections asap, so leave the work of determining which
     * server the connection is for to our child.
     */

    if (listen_conn) {

      /* Check for exceeded MaxInstances. */
      if (ServerMaxInstances && (child_count() >= ServerMaxInstances)) {
        pr_event_generate("core.max-instances", NULL);
        
        pr_log_pri(PR_LOG_WARNING,
          "MaxInstances (%d) reached, new connection denied",
          ServerMaxInstances);
        close(fd);

      /* Check for exceeded MaxConnectionRate. */
      } else if (max_connects && (nconnects > max_connects)) {
        pr_event_generate("core.max-connection-rate", NULL);

        pr_log_pri(PR_LOG_WARNING,
          "MaxConnectionRate (%lu/%u secs) reached, new connection denied",
          max_connects, max_connect_interval);
        close(fd);

      /* Fork off a child to handle the connection. */
      } else {
        PR_DEVEL_CLOCK(fork_server(fd, listen_conn, FALSE));
      }
    }
#ifdef PR_DEVEL_NO_DAEMON
    /* Do not continue the while() loop here if not daemonizing. */
    break;
#endif /* PR_DEVEL_NO_DAEMON */
  }
}

/* This function is to handle the dispatching of actions based on
 * signals received by the signal handlers, to avoid signal handler-based
 * race conditions.
 */

void pr_signals_handle(void) {
  table_handling_signal(TRUE);

  if (errno == EINTR &&
      PR_TUNABLE_EINTR_RETRY_INTERVAL > 0) {
    struct timeval tv;
    unsigned long interval_usecs = PR_TUNABLE_EINTR_RETRY_INTERVAL * 1000000;

    tv.tv_sec = (interval_usecs / 1000000);
    tv.tv_usec = (interval_usecs - (tv.tv_sec * 1000000));

    pr_trace_msg("signal", 18, "interrupted system call, "
      "delaying for %lu %s, %lu %s",
      (unsigned long) tv.tv_sec, tv.tv_sec != 1 ? "secs" : "sec",
      (unsigned long) tv.tv_usec, tv.tv_usec != 1 ? "microsecs" : "microsec");

    pr_timer_usleep(interval_usecs);
  }

  while (recvd_signal_flags) {

    if (recvd_signal_flags & RECEIVED_SIG_ALRM) {
      recvd_signal_flags &= ~RECEIVED_SIG_ALRM;
      pr_trace_msg("signal", 9, "handling SIGALRM (signal %d)", SIGALRM);
      handle_alarm();
    }

    if (recvd_signal_flags & RECEIVED_SIG_CHLD) {
      recvd_signal_flags &= ~RECEIVED_SIG_CHLD;
      pr_trace_msg("signal", 9, "handling SIGCHLD (signal %d)", SIGCHLD);
      handle_chld();
    }

    if (recvd_signal_flags & RECEIVED_SIG_EVENT) {
      recvd_signal_flags &= ~RECEIVED_SIG_EVENT;

      /* The "event" signal is SIGUSR2 in proftpd. */
      pr_trace_msg("signal", 9, "handling SIGUSR2 (signal %d)", SIGUSR2);
      handle_evnt();
    }

    if (recvd_signal_flags & RECEIVED_SIG_SEGV) {
      recvd_signal_flags &= ~RECEIVED_SIG_SEGV;
      pr_trace_msg("signal", 9, "handling SIGSEGV (signal %d)", SIGSEGV);
      handle_terminate_other();
    }

    if (recvd_signal_flags & RECEIVED_SIG_TERMINATE) {
      recvd_signal_flags &= ~RECEIVED_SIG_TERMINATE;
      pr_trace_msg("signal", 9, "handling signal %d", term_signo);
      handle_terminate();
    }

    if (recvd_signal_flags & RECEIVED_SIG_TERM_OTHER) {
      recvd_signal_flags &= ~RECEIVED_SIG_TERM_OTHER;
      pr_trace_msg("signal", 9, "handling signal %d", term_signo);
      handle_terminate_other();
    }

    if (recvd_signal_flags & RECEIVED_SIG_XCPU) {
      recvd_signal_flags &= ~RECEIVED_SIG_XCPU;
      pr_trace_msg("signal", 9, "handling SIGXCPU (signal %d)", SIGXCPU);
      handle_xcpu();
    }

    if (recvd_signal_flags & RECEIVED_SIG_ABORT) {
      recvd_signal_flags &= ~RECEIVED_SIG_ABORT;
      pr_trace_msg("signal", 9, "handling SIGABRT (signal %d)", SIGABRT);
      handle_abort();
    }

    if (recvd_signal_flags & RECEIVED_SIG_RESTART) {
      recvd_signal_flags &= ~RECEIVED_SIG_RESTART;
      pr_trace_msg("signal", 9, "handling SIGHUP (signal %d)", SIGHUP);

      /* NOTE: should this be done here, rather than using a schedule? */
      schedule(core_restart_cb, 0, NULL, NULL, NULL, NULL);
    }

    if (recvd_signal_flags & RECEIVED_SIG_EXIT) {
      recvd_signal_flags &= ~RECEIVED_SIG_EXIT;
      pr_trace_msg("signal", 9, "handling SIGUSR1 (signal %d)", SIGUSR1);
      pr_log_pri(PR_LOG_NOTICE, "%s", "Parent process requested shutdown");
      pr_session_disconnect(NULL, PR_SESS_DISCONNECT_SERVER_SHUTDOWN, NULL);
    }

    if (recvd_signal_flags & RECEIVED_SIG_SHUTDOWN) {
      recvd_signal_flags &= ~RECEIVED_SIG_SHUTDOWN;
      pr_trace_msg("signal", 9, "handling SIGUSR1 (signal %d)", SIGUSR1);

      /* NOTE: should this be done here, rather than using a schedule? */
      schedule(shutdown_exit, 0, NULL, NULL, NULL, NULL);
    }
  }

  table_handling_signal(FALSE);
}

/* sig_restart occurs in the master daemon when manually "kill -HUP"
 * in order to re-read configuration files, and is sent to all
 * children by the master.
 */
#ifdef _QNAP_
void sig_restart(int signo, siginfo_t *sinfo, void *p) {
	pr_child_t *ch = child_get_by_pid(sinfo->si_pid);
	g_bHup = TRUE;
	if (ch) {
		//The SIGHUP is sent from one child
		if (ch->next) {
			//Tell next child to reconfig
			ch = ch->next;
			kill(ch->ch_pid, SIGHUP);
		}
		return ;
	}
#else
static RETSIGTYPE sig_restart(int signo) {
#endif
  recvd_signal_flags |= RECEIVED_SIG_RESTART;
#ifndef _QNAP_
  if (signal(SIGHUP, sig_restart) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGHUP (signal %d) handler: %s", SIGHUP,
      strerror(errno));
  }
#endif
}

static RETSIGTYPE sig_evnt(int signo) {
  recvd_signal_flags |= RECEIVED_SIG_EVENT;

  if (signal(SIGUSR2, sig_evnt) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGUSR2 (signal %d) handler: %s", SIGUSR2,
      strerror(errno));
  }
}

/* sig_disconnect is called in children when the parent daemon
 * detects that shutmsg has been created and ftp sessions should
 * be destroyed.  If a file transfer is underway, the process simply
 * dies, otherwise a function is scheduled to attempt to display
 * the shutdown reason.
 */
static RETSIGTYPE sig_disconnect(int signo) {

  /* If this is an anonymous session, or a transfer is in progress,
   * perform the exit a little later...
   */
  if ((session.sf_flags & SF_ANON) ||
      (session.sf_flags & SF_XFER)) {
    recvd_signal_flags |= RECEIVED_SIG_EXIT;

  } else {
    recvd_signal_flags |= RECEIVED_SIG_SHUTDOWN;
  }

  if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGUSR1 (signal %d) handler: %s", SIGUSR1,
      strerror(errno));
  }
}

static RETSIGTYPE sig_child(int signo) {
  recvd_signal_flags |= RECEIVED_SIG_CHLD;

  /* We make an exception here to the synchronous processing that is done
   * for other signals; SIGCHLD is handled asynchronously.  This is made
   * necessary by two things.
   *
   * First, we need to support non-POSIX systems.  Under POSIX, once a
   * signal handler has been configured for a given signal, that becomes
   * that signal's disposition, until explicitly changed later.  Non-POSIX
   * systems, on the other hand, will restore the default disposition of
   * a signal after a custom signal handler has been configured.  Thus,
   * to properly support non-POSIX systems, a call to signal(2) is necessary
   * as one of the last steps in our signal handlers.
   *
   * Second, SVR4 systems differ specifically in their semantics of signal(2)
   * and SIGCHLD.  These systems will check for any unhandled SIGCHLD
   * signals, waiting to be reaped via wait(2) or waitpid(2), whenever
   * the disposition of SIGCHLD is changed.  This means that if our process
   * handles SIGCHLD, but does not call wait(2) or waitpid(2), and then
   * calls signal(2), another SIGCHLD is generated; this loop repeats,
   * until the process runs out of stack space and terminates.
   *
   * Thus, in order to cover this interaction, we'll need to call handle_chld()
   * here, asynchronously.  handle_chld() does the work of reaping dead
   * child processes, and does not seem to call any non-reentrant functions,
   * so it should be safe.
   */

  handle_chld();

  if (signal(SIGCHLD, sig_child) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGCHLD (signal %d) handler: %s", SIGCHLD,
      strerror(errno));
  }
}

#ifdef PR_DEVEL_COREDUMP
static char *prepare_core(void) {
  static char dir[256];

  memset(dir, '\0', sizeof(dir));
  snprintf(dir, sizeof(dir)-1, "%s/proftpd-core-%lu", PR_CORE_DIR,
    (unsigned long) getpid());

  if (mkdir(dir, 0700) < 0) {
    pr_log_pri(PR_LOG_WARNING, "unable to create directory '%s' for "
      "coredump: %s", dir, strerror(errno));

  } else {
    chdir(dir);
  }

  return dir;
}
#endif /* PR_DEVEL_COREDUMP */

static RETSIGTYPE sig_abort(int signo) {
  recvd_signal_flags |= RECEIVED_SIG_ABORT;

  if (signal(SIGABRT, SIG_DFL) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGABRT (signal %d) handler: %s", SIGABRT,
      strerror(errno));
  }

#ifdef PR_DEVEL_COREDUMP
  pr_log_pri(PR_LOG_NOTICE, "ProFTPD received SIGABRT signal, generating core "
    "file in %s", prepare_core());
  pr_session_end(PR_SESS_END_FL_NOEXIT);
  abort();
#endif /* PR_DEVEL_COREDUMP */
}

static void handle_abort(void) {
  pr_log_pri(PR_LOG_NOTICE, "ProFTPD received SIGABRT signal, no core dump");
  finish_terminate();
}

#ifdef PR_DEVEL_STACK_TRACE
static void handle_stacktrace_signal(int signo, siginfo_t *info, void *ptr) {
  register unsigned i;
  ucontext_t *uc = (ucontext_t *) ptr;
  void *trace[PR_TUNABLE_CALLER_DEPTH];
  char **strings;
  int tracesz;

  /* Call the "normal" signal handler. */
  table_handling_signal(TRUE);
  sig_terminate(signo);

  pr_log_pri(PR_LOG_ERR, "-----BEGIN STACK TRACE-----");

  tracesz = backtrace(trace, PR_TUNABLE_CALLER_DEPTH);
  if (tracesz < 0) {
    pr_log_pri(PR_LOG_ERR, "backtrace(3) error: %s", strerror(errno));
  }

  /* Overwrite sigaction with caller's address */
#if defined(REG_EIP)
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];
#elif defined(REG_RIP)
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_RIP];
#endif

  strings = backtrace_symbols(trace, tracesz);
  if (strings == NULL) {
    pr_log_pri(PR_LOG_ERR, "backtrace_symbols(3) error: %s", strerror(errno));
  }

  /* Skip first stack frame; it just points here. */
  for (i = 1; i < tracesz; ++i) {
    pr_log_pri(PR_LOG_ERR, "[%u] %s", i-1, strings[i]);
  }

  pr_log_pri(PR_LOG_ERR, "-----END STACK TRACE-----");

  finish_terminate();
}
#endif /* PR_DEVEL_STACK_TRACE */

static RETSIGTYPE sig_terminate(int signo) {

  /* Capture the signal number for later display purposes. */
  term_signo = signo;

  if (signo == SIGSEGV ||
      signo == SIGXCPU
#ifdef SIGBUS
      || signo == SIGBUS) {
#else
     ) {
#endif /* SIGBUS */

    if (signo == SIGXCPU) {
      recvd_signal_flags |= RECEIVED_SIG_XCPU;

    } else {
      recvd_signal_flags |= RECEIVED_SIG_SEGV;
    }

    /* This is probably not the safest thing to be doing, but since the
     * process is terminating anyway, why not?  It helps when knowing/logging
     * that a segfault happened...
     */
#ifdef _QNAP_
    if(session.user && session.c->remote_name)
      SendConnToLogEngine(EVENT_TYPE_INFO, session.user, (char *)session.c->remote_name, "", CONN_SERV_FTP, CONN_ACTION_LOGOUT, "---");
#endif
    pr_trace_msg("signal", 9, "handling %s (signal %d)",
      signo == SIGSEGV ? "SIGSEGV" : 
        signo == SIGXCPU ? "SIGXCPU" : "SIGBUS", signo);
    pr_log_pri(PR_LOG_NOTICE, "ProFTPD terminating (signal %d)", signo);

    pr_log_pri(PR_LOG_INFO, "%s session closed.",
      pr_session_get_protocol(PR_SESS_PROTO_FL_LOGOUT));

#ifdef PR_DEVEL_STACK_TRACE
    install_stacktrace_handler();
#endif /* PR_DEVEL_STACK_TRACE */

  } else if (signo == SIGTERM) {
    recvd_signal_flags |= RECEIVED_SIG_TERMINATE;

  } else {
    recvd_signal_flags |= RECEIVED_SIG_TERM_OTHER;
  }

  /* Ignore future occurrences of this signal; we'll be terminating anyway. */

  if (signal(signo, SIG_IGN) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install handler for signal %d: %s", signo, strerror(errno));
  }
}

static void handle_chld(void) {
  sigset_t sig_set;
  pid_t pid;

  sigemptyset(&sig_set);
  sigaddset(&sig_set, SIGTERM);
  sigaddset(&sig_set, SIGCHLD);

  pr_alarms_block();

  /* Block SIGTERM in here, so we don't create havoc with the child list
   * while modifying it.
   */
  if (sigprocmask(SIG_BLOCK, &sig_set, NULL) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to block signal set: %s", strerror(errno));
  }

  while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
    if (child_remove(pid) == 0)
      have_dead_child = TRUE;
  }

  if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to unblock signal set: %s", strerror(errno));
  }

  pr_alarms_unblock();
}

static void handle_evnt(void) {
  pr_event_generate("core.signal.USR2", NULL);
}

static void handle_xcpu(void) {
  pr_log_pri(PR_LOG_NOTICE, "ProFTPD CPU limit exceeded (signal %d)", SIGXCPU);
  finish_terminate();
}

static void handle_terminate_other(void) {
  pr_log_pri(PR_LOG_WARNING, "ProFTPD terminating (signal %d)", term_signo);
  finish_terminate();
}

static void handle_terminate(void) {

  /* Do not log if we are a child that has been terminated. */
  if (is_master) {

    /* Send a SIGTERM to all our children */
    if (child_count()) {
      PRIVS_ROOT
      child_signal(SIGTERM);
      PRIVS_RELINQUISH
    }

    pr_log_pri(PR_LOG_NOTICE, "ProFTPD killed (signal %d)", term_signo);
  }

  finish_terminate();
}

static void finish_terminate(void) {

  if (is_master &&
      mpid == getpid()) {
    PRIVS_ROOT

    /* Do not need the pidfile any longer. */
    if (ServerType == SERVER_STANDALONE &&
        !nodaemon)
      pr_pidfile_remove();

    /* Run any exit handlers registered in the master process here, so that
     * they may have the benefit of root privs.  More than likely these
     * exit handlers were registered by modules' module initialization
     * functions, which also occur under root priv conditions.
     *
     * If an exit handler is registered after the fork(), it won't be run here;
     * that registration occurs in a different process space.
     */
    pr_event_generate("core.exit", NULL);
    pr_event_generate("core.shutdown", NULL);

    /* Remove the registered exit handlers now, so that the ensuing
     * pr_session_end() call (outside the root privs condition) does not call
     * the exit handlers for the master process again.
     */
    pr_event_unregister(NULL, "core.exit", NULL);
    pr_event_unregister(NULL, "core.shutdown", NULL);

    PRIVS_RELINQUISH

    if (ServerType == SERVER_STANDALONE) {
      pr_log_pri(PR_LOG_NOTICE, "ProFTPD " PROFTPD_VERSION_TEXT
        " standalone mode SHUTDOWN");

      /* Clean up the scoreboard */
      PRIVS_ROOT
      pr_delete_scoreboard();
      PRIVS_RELINQUISH
    }
  }

  pr_session_disconnect(NULL, PR_SESS_DISCONNECT_SIGNAL, "Killed by signal");
}

#ifdef PR_DEVEL_STACK_TRACE
static void install_stacktrace_handler(void) {
  struct sigaction action;

  memset(&action, 0, sizeof(action));
  action.sa_sigaction = handle_stacktrace_signal;
  action.sa_flags = SA_SIGINFO;

  /* Ideally we would check the return value here. */
  sigaction(SIGSEGV, &action, NULL);
# ifdef SIGBUS
  sigaction(SIGBUS, &action, NULL);
# endif /* SIGBUS */
  sigaction(SIGXCPU, &action, NULL);
}
#endif /* PR_DEVEL_STACK_TRACE */

#ifdef _QNAP_
static char *Qget_fifo_rootdir(pid_t pid, char *rootpath, size_t pathsize)
{
	char linkname[260];
	snprintf(linkname, sizeof(linkname), "/proc/%u/root", pid);
	if (readlink(linkname, rootpath, pathsize) <= 0) {
        if(custom_root)
    		snprintf(rootpath, pathsize, "%s/%s", custom_root_path, QFIFO_PATH);
        else
    		snprintf(rootpath, pathsize, "/share/%s", QFIFO_PATH);
	}
	else {
		int len = strlen(rootpath);
		snprintf(rootpath+len, pathsize-len, "/%s", QFIFO_PATH);
	}
	return rootpath;
}

static char *Qgenetate_fifo_name(pid_t pid, const char *name, char *fifo_name, size_t namelen)
{
	char rootpath[260];
	if (is_master)
		snprintf(fifo_name, namelen, "%s/%s-%u", Qget_fifo_rootdir(pid, rootpath, sizeof(rootpath)), name, pid);
	else
		snprintf(fifo_name, namelen, "/%s/%s-%u", QFIFO_PATH, name, pid);
	pr_log_pri(PR_LOG_DEBUG, "Qgenetate_fifo_name(%s)", fifo_name);
	return fifo_name;
}
int QCreate_FIFO(pid_t pid, const char *name)
{
	char fifo_name[260];
	int ret = mkfifo(Qgenetate_fifo_name(pid, name, fifo_name, sizeof(fifo_name)), 0666);
	pr_log_pri(PR_LOG_DEBUG, "%s(%s) return %d", __FUNCTION__, fifo_name, ret);
	return ret;
}
int QRemove_FIFO(pid_t pid, const char *name)
{
	char fifo_name[260];
	int ret = -1;
#ifndef RPC_PROC
    PRIVS_ROOT
#endif
    ret = qnap_unlink(Qgenetate_fifo_name(pid, name, fifo_name, sizeof(fifo_name)));
#ifndef RPC_PROC
    PRIVS_RELINQUISH
#endif
    return ret;
}
int QOpenFIFO(pid_t pid, const char *name, int open_flag)
{
	char fifo_name[260];
	int ret = open(Qgenetate_fifo_name(pid, name, fifo_name, sizeof(fifo_name)), open_flag);
	if (ret == -1) {
		pr_log_pri(PR_LOG_DEBUG, "%s:%d(%s) return %d(%s)", __FUNCTION__, __LINE__, fifo_name, ret, strerror(errno));
		//Maybe fifo_name not be created
		if (mkfifo(fifo_name, 0666) == 0) {
			//try open fifo again
			ret = open(fifo_name, open_flag);
		}
		else {
			pr_log_pri(PR_LOG_DEBUG, "%s:%d(%s) return %d(%s)", __FUNCTION__, __LINE__, fifo_name, ret, strerror(errno));
			//Maybe the folder not be created.
			char *rootpath = strdup(fifo_name);
			char *ptr = strrchr(rootpath, '/');
			if (ptr) {
				*ptr = 0;
				mkdir(rootpath, 0777);
				chmod(rootpath, 0777);
				if (mkfifo(fifo_name, 0666) == 0) {
					ret = open(fifo_name, open_flag);
				}
				else {
					pr_log_pri(PR_LOG_DEBUG, "%s:%d(%s) return %d(%s)", __FUNCTION__, __LINE__, fifo_name, ret, strerror(errno));
				}
			}
			free(rootpath);
		}
	}
	pr_log_pri(PR_LOG_DEBUG, "%s:%d(%s) return %d", __FUNCTION__, __LINE__, fifo_name, ret);
	return ret;
}

// Jacky20140904 - Modify with some error handling
void sig_pipe_fd(int signo, siginfo_t *sinfo, void *p)
{
	int r_pipe = -1;
	int w_pipe = -1;
	char buffer[1024];

	pr_log_pri(PR_LOG_DEBUG, "PIPEDEBUG:sig_pipe_fd(%u)", sinfo->si_pid);
    if(custom_root)
        PRIVS_ROOT
	r_pipe = QOpenFIFO(sinfo->si_pid, CONF_C2P, O_RDONLY|O_NONBLOCK);
    w_pipe = QOpenFIFO(sinfo->si_pid, CONF_P2C, O_RDWR);

    if(r_pipe < 0 || w_pipe < 0) {
		pr_log_pri(PR_LOG_ERR, "%s:child id=%d, read pipe open failed", __FUNCTION__, sinfo->si_pid);
    }
    else {
        int len = read(r_pipe, buffer, sizeof(buffer));
        if(len <= 0) {
            pr_log_pri(PR_LOG_ERR, "%s:Read rpipe from child(%d) = %d (%s)", __FUNCTION__, sinfo->si_pid, len, strerror(errno));
        }

        if(len <= 0) {
            pr_log_pri(PR_LOG_ERR, "%s:Read pipe cmd from child(%d) failed=%d (%s)", __FUNCTION__, sinfo->si_pid, len, strerror(errno));
        }
        else {
            int fd;
            pr_log_pri(PR_LOG_DEBUG, "%s:child id=%d, name=%s", __FUNCTION__, sinfo->si_pid, buffer);
            fd = open(buffer, O_RDONLY);
            if(fd < 0) {
                write(w_pipe, "FAIL", 4);
                pr_log_pri(PR_LOG_ERR, "%s:Open file (%s) for pipe from child(%d) failed(%d)", __FUNCTION__, buffer, sinfo->si_pid, errno);
            }
            else {
                int total = 0;
                write(w_pipe, "SUCC", 4);
                while((len = read(fd, buffer, sizeof(buffer))) > 0) {
                    int w_total = 0;
                    while(w_total < len) {
                        int wsize = write(w_pipe, buffer+w_total, len-w_total);
                        if(wsize < 0) {
                            if(errno != EAGAIN && errno != EINTR) {
                                pr_log_pri(PR_LOG_ERR, "%s:write file (%s) to pipe from child(%d) failed(%d)", __FUNCTION__, buffer, sinfo->si_pid, errno);
                                break;
                            }
                            usleep(10000);
                            continue;
                        }
                        total += wsize;
                        w_total += wsize;
                    }
                    if(w_total != len)
                        break;
                }
                close(fd);
                pr_log_pri(PR_LOG_DEBUG, "%s:child id=%d, write %d to pipe", __FUNCTION__, sinfo->si_pid, total);
            }
        }
    }

    if(r_pipe >= 0)
        close(r_pipe);
    if(w_pipe >= 0)
        close(w_pipe);
    if(custom_root)
        PRIVS_RELINQUISH
}
void sig_pipe_rpc(int signo, siginfo_t *sinfo, void *p)
{
	char buffer[4096+16], *cmd, *arg;
	int len;
	int ch_pipefd_rpc[2];

	pr_log_pri(PR_LOG_DEBUG, "PIPEDEBUG:sig_pipe_rpc(%u)", sinfo->si_pid);
#ifndef RPC_PROC
    PRIVS_ROOT
#endif
	ch_pipefd_rpc[0] = QOpenFIFO(sinfo->si_pid, RPC_C2P, O_RDONLY|O_NONBLOCK);
	ch_pipefd_rpc[1] = QOpenFIFO(sinfo->si_pid, RPC_P2C, O_RDWR);
	len = read(ch_pipefd_rpc[0], buffer, sizeof(buffer)-1);
	if (len <= 0) {
		pr_log_pri(PR_LOG_ERR, "%s::Read pipe cmd from child(%d) failed (%d)", __FUNCTION__, sinfo->si_pid, errno);
		goto END_PIPE_RPC;
	}
    buffer[len] = 0;
	cmd = buffer;
	if ((arg = strchr(buffer, '\n')) != NULL) {
		*arg = 0;
		arg ++;
	}
	pr_log_pri(PR_LOG_DEBUG, "%s::%s(%s)", __FUNCTION__, cmd, arg);
	if (strcmp(cmd, "getpwnam") == 0) {
		struct passwd *pw = NULL;
		int ret = -1;
		if (arg) {
			pw = getpwnam(arg);
		}
		if (pw) {
			ret = 0;
		}
		write(ch_pipefd_rpc[1], &ret, sizeof(ret));
		if (ret == 0)
			write(ch_pipefd_rpc[1], pw, sizeof(struct passwd));
		goto END_PIPE_RPC;
	}
	else if (strcmp(cmd, "getgrnam") == 0) {
		struct group *grp = NULL;
		int ret = -1;
		if (arg) {
			grp = getgrnam(arg);
		}
		if (grp) {
			ret = 0;
		}
		write(ch_pipefd_rpc[1], &ret, sizeof(ret));
		if (ret == 0)
			write(ch_pipefd_rpc[1], grp, sizeof(struct group));
		goto END_PIPE_RPC;
	}
	else if (strcmp(cmd, "unlink") == 0) {
		int ret = -1;
		if (arg) {
			ret = unlink(arg);
		}
		if(ret != 0){
			pr_log_pri(PR_LOG_DEBUG, "%s %d::%s(%s)", __FUNCTION__, __LINE__, cmd, strerror(errno));
		}
		write(ch_pipefd_rpc[1], &ret, sizeof(int));
		goto END_PIPE_RPC;
	}

END_PIPE_RPC:
	close(ch_pipefd_rpc[0]);
	close(ch_pipefd_rpc[1]);
#ifndef RPC_PROC
    PRIVS_RELINQUISH
#endif
}

static int _pipe_submit(int readfd, int writefd, const char *cmd, void *arg)
{
	char buffer[4096+16];
	int ret = -1;
	int len = 0;
    int retry = 5;
	int count = 0;

	len = snprintf(buffer, sizeof(buffer), "%s\n%s", cmd, (char *)arg)+1;

	write(writefd, buffer, len);

    len = 0;
    while(retry-- > 0 && len <= 0) {
#ifdef RPC_PROC
        kill((rpc_pid > 0) ? rpc_pid : getppid(), SIGUSR2);
#else
        kill(getppid(), SIGUSR2);
#endif
        usleep(2000);

        len = read(readfd, &ret, sizeof(ret));

        count = 0;
        while(len <= 0 && count < 100) {
            usleep(10000);
            len = read(readfd, &ret, sizeof(ret));
            count++;
            if(len < 0 && errno != EAGAIN) {
                retry = 0;
                break;
            }
        }
    }

	if (len <= 0)
		ret = -1;

	pr_log_pri(PR_LOG_DEBUG, "%s:%d %s(%s) return %d", __FUNCTION__, __LINE__, cmd, (char *)arg, ret);

	return ret;
}

void *pipe_cmd(const char *cmd, void *arg)
{
	void *retp = NULL;
	int ch_pipefd_rpc[2];

	pr_log_pri(PR_LOG_DEBUG, "PIPEDEBUG:pipe_cmd(%s)", cmd);
	ch_pipefd_rpc[0] = QOpenFIFO(session.pid, RPC_P2C, O_RDONLY|O_NONBLOCK);
	ch_pipefd_rpc[1] = QOpenFIFO(session.pid, RPC_C2P, O_RDWR);
	if (strcmp(cmd, "getpwnam") == 0) {
		int ret;
		ret = _pipe_submit(ch_pipefd_rpc[0], ch_pipefd_rpc[1], cmd, arg);
		pr_log_pri(PR_LOG_DEBUG, "%s::%s(%s) return %d", __FUNCTION__, cmd, (char *)arg, ret);
		if (ret == 0) {
			struct passwd *pw = malloc(sizeof(struct passwd));
			read(ch_pipefd_rpc[0], pw, sizeof(struct passwd));
			retp = pw;
		}
	}
	else if (strcmp(cmd, "getgrnam") == 0) {
		int ret;
		ret = _pipe_submit(ch_pipefd_rpc[0], ch_pipefd_rpc[1], cmd, arg);
		pr_log_pri(PR_LOG_DEBUG, "%s::%s(%s) return %d", __FUNCTION__, cmd, (char *)arg, ret);
		if (ret == 0) {
			struct group *grp = malloc(sizeof(struct group));
			read(ch_pipefd_rpc[0], grp, sizeof(struct group));
			retp = grp;
		}
	}
	else if (strcmp(cmd, "unlink") == 0) {
		static int return_val = -1;
		return_val = _pipe_submit(ch_pipefd_rpc[0], ch_pipefd_rpc[1], cmd, arg);
		pr_log_pri(PR_LOG_DEBUG, "%s::%s(%s) return %d", __FUNCTION__, cmd, (char *)arg, return_val);
		retp = &return_val;
	}
	close(ch_pipefd_rpc[0]);
	close(ch_pipefd_rpc[1]);
	return retp;
}

#endif

static void install_signal_handlers(void) {
  sigset_t sig_set;

  /* Should the master server (only applicable in standalone mode)
   * kill off children if we receive a signal that causes termination?
   * Hmmmm... maybe this needs to be rethought, but I've done it in
   * such a way as to only kill off our children if we receive a SIGTERM,
   * meaning that the admin wants us dead (and probably our kids too).
   */

  /* The sub-pool for the child list is created the first time we fork
   * off a child.  To conserve memory, the pool and list is destroyed
   * when our last child dies (to prevent the list from eating more and
   * more memory on long uptimes).
   */

  sigemptyset(&sig_set);

  sigaddset(&sig_set, SIGCHLD);
  sigaddset(&sig_set, SIGINT);
  sigaddset(&sig_set, SIGQUIT);
  sigaddset(&sig_set, SIGILL);
  sigaddset(&sig_set, SIGABRT);
  sigaddset(&sig_set, SIGFPE);
  sigaddset(&sig_set, SIGSEGV);
  sigaddset(&sig_set, SIGALRM);
  sigaddset(&sig_set, SIGTERM);
  sigaddset(&sig_set, SIGHUP);
  sigaddset(&sig_set, SIGUSR2);
#ifdef SIGSTKFLT
  sigaddset(&sig_set, SIGSTKFLT);
#endif /* SIGSTKFLT */
#ifdef SIGIO
  sigaddset(&sig_set, SIGIO);
#endif /* SIGIO */
#ifdef SIGBUS
  sigaddset(&sig_set, SIGBUS);
#endif /* SIGBUS */

  if (signal(SIGCHLD, sig_child) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGCHLD (signal %d) handler: %s", SIGCHLD,
      strerror(errno));
  }
#ifdef _QNAP_
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sig_restart;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGHUP, &act, NULL);
	pr_log_pri(PR_LOG_DEBUG, "QNAP SIGHUP signals installed");
}
#else
  if (signal(SIGHUP, sig_restart) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGHUP (signal %d) handler: %s", SIGHUP,
      strerror(errno));
  }
#endif

  if (signal(SIGINT, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGINT (signal %d) handler: %s", SIGINT,
      strerror(errno));
  }

  if (signal(SIGQUIT, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGQUIT (signal %d) handler: %s", SIGQUIT,
      strerror(errno));
  }

  if (signal(SIGILL, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGILL (signal %d) handler: %s", SIGILL,
      strerror(errno));
  }

  if (signal(SIGFPE, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGFPE (signal %d) handler: %s", SIGFPE,
      strerror(errno));
  }

  if (signal(SIGABRT, sig_abort) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGABRT (signal %d) handler: %s", SIGABRT,
      strerror(errno));
  }

#ifdef PR_DEVEL_STACK_TRACE
  /* Installs stacktrace handlers for SIGSEGV, SIGXCPU, and SIGBUS. */
  install_stacktrace_handler();
#else
  if (signal(SIGSEGV, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGSEGV (signal %d) handler: %s", SIGSEGV,
      strerror(errno));
  }

  if (signal(SIGXCPU, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGXCPU (signal %d) handler: %s", SIGXCPU,
      strerror(errno));
  }

# ifdef SIGBUS
  if (signal(SIGBUS, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGBUS (signal %d) handler: %s", SIGBUS,
      strerror(errno));
  }
# endif /* SIGBUS */
#endif /* PR_DEVEL_STACK_TRACE */

  /* Ignore SIGALRM; this will be changed when a timer is registered. But
   * this will prevent SIGALRMs from killing us if we don't currently have
   * any timers registered.
    */
  if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGALRM (signal %d) handler: %s", SIGALRM,
      strerror(errno));
  }

  if (signal(SIGTERM, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGTERM (signal %d) handler: %s", SIGTERM,
      strerror(errno));
  }

  if (signal(SIGURG, SIG_IGN) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGURG (signal %d) handler: %s", SIGURG,
      strerror(errno));
  }

#ifdef SIGSTKFLT
  if (signal(SIGSTKFLT, sig_terminate) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGSTKFLT (signal %d) handler: %s", SIGSTKFLT,
      strerror(errno));
  }
#endif /* SIGSTKFLT */

#ifdef SIGIO
  if (signal(SIGIO, SIG_IGN) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGIO (signal %d) handler: %s", SIGIO,
      strerror(errno));
  }
#endif /* SIGIO */

#ifdef _QNAP_
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sig_pipe_fd;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &act, NULL);

#ifndef RPC_PROC
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sig_pipe_rpc;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR2, &act, NULL);
#endif

	pr_log_pri(PR_LOG_DEBUG, "QNAP SIGUSR* signals installed");
}
#else
  if (signal(SIGUSR2, sig_evnt) == SIG_ERR) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to install SIGUSR2 (signal %d) handler: %s", SIGUSR2,
      strerror(errno));
  }
#endif

  /* In case our parent left signals blocked (as happens under some
   * poor inetd implementations)
   */
  if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "unable to block signal set: %s", strerror(errno));
  }
}

static void daemonize(void) {
#ifndef HAVE_SETSID
  int ttyfd;
#endif

  /* Fork off and have parent exit.
   */
  switch (fork()) {
    case -1:
      perror("fork(2) error");
      exit(1);

    case 0:
      break;

    default: 
      exit(0);
  }

#ifdef HAVE_SETSID
  /* setsid() is the preferred way to disassociate from the
   * controlling terminal
   */
  setsid();
#else
  /* Open /dev/tty to access our controlling tty (if any) */
  if ((ttyfd = open("/dev/tty", O_RDWR)) != -1) {
    if (ioctl(ttyfd, TIOCNOTTY, NULL) == -1) {
      perror("ioctl");
      exit(1);
    }

    close(ttyfd);
  }
#endif /* HAVE_SETSID */

  /* Close the three big boys */
  close(fileno(stdin));
  close(fileno(stdout));
  close(fileno(stderr));

  /* Portable way to prevent re-acquiring a tty in the future */

#ifdef HAVE_SETPGID
  setpgid(0, getpid());
#else
# ifdef SETPGRP_VOID
  setpgrp();
# else
  setpgrp(0, getpid());
# endif
#endif

  /* Reset the cached "master PID" value to that of the daemon process;
   * there are places in the code which check this value to see if they
   * are the daemon process, e.g. at shutdown.
   */
  mpid = getpid();

  pr_fsio_chdir("/", 0);
}

static void inetd_main(void) {
  int res = 0;

  /* Make sure the scoreboard file exists. */
  PRIVS_ROOT
  res = pr_open_scoreboard(O_RDWR);
  if (res < 0) {
    PRIVS_RELINQUISH

    switch (res) {
      case PR_SCORE_ERR_BAD_MAGIC:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: bad/corrupted file");
        return;

      case PR_SCORE_ERR_OLDER_VERSION:
      case PR_SCORE_ERR_NEWER_VERSION:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: wrong version, "
          "writing new scoreboard");

        /* Delete the scoreboard, then open it again (and assume that the
         * open succeeds).
         */
        PRIVS_ROOT
        pr_delete_scoreboard();
        pr_open_scoreboard(O_RDWR);
        break;

      default:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: %s",
          strerror(errno));
        return;
    }
  }
  PRIVS_RELINQUISH
  pr_close_scoreboard(FALSE);

  pr_event_generate("core.startup", NULL);

  init_bindings();

  /* Check our shutdown status */
  if (check_shutmsg(&shut, &deny, &disc, shutmsg, sizeof(shutmsg)) == 1)
    shutdownp = 1;

  /* Finally, call right into fork_server() to start servicing the
   * connection immediately.
   */
  fork_server(STDIN_FILENO, main_server->listen, TRUE);
}

static void standalone_main(void) {
  int res = 0;

  if (nodaemon) {
    log_stderr(quiet ? FALSE : TRUE);
    close(fileno(stdin));
    close(fileno(stdout));

  } else {
    log_stderr(FALSE);
    daemonize();
  }

  PRIVS_ROOT
  pr_delete_scoreboard();
  res = pr_open_scoreboard(O_RDWR);
  if (res < 0) {
    PRIVS_RELINQUISH

    switch (res) {
      case PR_SCORE_ERR_BAD_MAGIC:
        pr_log_pri(PR_LOG_ERR,
          "error opening scoreboard: bad/corrupted file");
        return;

      case PR_SCORE_ERR_OLDER_VERSION:
        pr_log_pri(PR_LOG_ERR,
          "error opening scoreboard: bad version (too old)");
        return;

      case PR_SCORE_ERR_NEWER_VERSION:
        pr_log_pri(PR_LOG_ERR,
          "error opening scoreboard: bad version (too new)");
        return;

      default:
        pr_log_pri(PR_LOG_ERR, "error opening scoreboard: %s", strerror(errno));
        return;
    }
  }
  PRIVS_RELINQUISH
  pr_close_scoreboard(TRUE);

  pr_event_generate("core.startup", NULL);

  init_bindings();

  pr_log_pri(PR_LOG_NOTICE, "ProFTPD %s (built %s) standalone mode STARTUP",
    PROFTPD_VERSION_TEXT " " PR_STATUS, BUILD_STAMP);

  pr_pidfile_write();
  daemon_loop();
}

extern char *optarg;
extern int optind, opterr, optopt;

#ifdef HAVE_GETOPT_LONG
static struct option opts[] = {
  { "nocollision",    0, NULL, 'N' },
  { "nodaemon",	      0, NULL, 'n' },
  { "quiet",	      0, NULL, 'q' },
  { "debug",	      1, NULL, 'd' },
  { "define",	      1, NULL, 'D' },
  { "config",	      1, NULL, 'c' },
  { "persistent",     1, NULL, 'p' },
  { "list",           0, NULL, 'l' },
  { "version",        0, NULL, 'v' },
  { "settings",       0, NULL, 'V' },
  { "version-status", 0, NULL, 1   },
  { "configtest",     0, NULL, 't' },
  { "help",	      0, NULL, 'h' },
  { "ipv4",           0, NULL, '4' },
  { "ipv6",           0, NULL, '6' },
  { NULL,	      0, NULL,  0  }
};
#endif /* HAVE_GETOPT_LONG */

static void show_settings(void) {
#ifdef HAVE_UNAME
  int res;
  struct utsname uts;
#endif /* !HAVE_UNAME */

  printf("%s", "Compile-time Settings:\n");
  printf("%s", "  Version: " PROFTPD_VERSION_TEXT " " PR_STATUS "\n");

#ifdef HAVE_UNAME
  /* We use uname(2) to get the 'machine', which will tell us whether
   * we're a 32- or 64-bit machine.
   */
  res = uname(&uts);
  if (res < 0) {
    printf("%s", "  Platform: " PR_PLATFORM " [unavailable]\n");

  } else {
    printf("  Platform: " PR_PLATFORM " [%s %s %s]\n", uts.sysname,
      uts.release, uts.machine);
  }
#else
  printf("%s", "  Platform: " PR_PLATFORM " [unknown]\n");
#endif /* !HAVE_UNAME */

  printf("%s", "  Built: " BUILD_STAMP "\n");
  printf("%s", "  Built With:\n    configure " PR_BUILD_OPTS "\n\n");

  printf("%s", "  CFLAGS: " PR_BUILD_CFLAGS "\n");
  printf("%s", "  LDFLAGS: " PR_BUILD_LDFLAGS "\n");
  printf("%s", "  LIBS: " PR_BUILD_LIBS "\n");

  printf("%s", "\n  Files:\n");
  printf("%s", "    Configuration File:\n");
  printf("%s", "      " PR_CONFIG_FILE_PATH "\n");
  printf("%s", "    Pid File:\n");
  printf("%s", "      " PR_PID_FILE_PATH "\n");
  printf("%s", "    Scoreboard File:\n");
  printf("%s", "      " PR_RUN_DIR "/proftpd.scoreboard\n");
#ifdef PR_USE_DSO
  printf("%s", "    Header Directory:\n");
  printf("%s", "      " PR_INCLUDE_DIR "/proftpd\n");
  printf("%s", "    Shared Module Directory:\n");
  printf("%s", "      " PR_LIBEXEC_DIR "\n");
#endif /* PR_USE_DSO */

  /* Feature settings */
  printf("%s", "\n  Features:\n");
#ifdef PR_USE_AUTO_SHADOW
  printf("%s", "    + Autoshadow support\n");
#else
  printf("%s", "    - Autoshadow support\n");
#endif /* PR_USE_AUTO_SHADOW */

#ifdef PR_USE_CTRLS
  printf("%s", "    + Controls support\n");
#else
  printf("%s", "    - Controls support\n");
#endif /* PR_USE_CTRLS */

#if defined(PR_USE_CURSES) && defined(HAVE_LIBCURSES)
  printf("%s", "    + curses support\n");
#else
  printf("%s", "    - curses support\n");
#endif /* PR_USE_CURSES && HAVE_LIBCURSES */

#ifdef PR_USE_DEVEL
  printf("%s", "    + Developer support\n");
#else
  printf("%s", "    - Developer support\n");
#endif /* PR_USE_DEVEL */

#ifdef PR_USE_DSO
  printf("%s", "    + DSO support\n");
#else
  printf("%s", "    - DSO support\n");
#endif /* PR_USE_DSO */

#ifdef PR_USE_IPV6
  printf("%s", "    + IPv6 support\n");
#else
  printf("%s", "    - IPv6 support\n");
#endif /* PR_USE_IPV6 */

#ifdef PR_USE_LARGEFILES
  printf("%s", "    + Largefile support\n");
#else
  printf("%s", "    - Largefile support\n");
#endif /* PR_USE_LARGEFILES */

#ifdef PR_USE_LASTLOG
  printf("%s", "    + Lastlog support\n");
#else
  printf("%s", "    - Lastlog support\n");
#endif /* PR_USE_LASTLOG */

#ifdef PR_USE_MEMCACHE
  printf("%s", "    + Memcache support\n");
#else
  printf("%s", "    - Memcache support\n");
#endif /* PR_USE_MEMCACHE */

#if defined(PR_USE_NCURSESW) && defined(HAVE_LIBNCURSESW)
  printf("%s", "    + ncursesw support\n");
#elif defined(PR_USE_NCURSES) && defined(HAVE_LIBNCURSES)
  printf("%s", "    + ncurses support\n");
#else
  printf("%s", "    - ncurses support\n");
#endif

#ifdef PR_USE_NLS
  printf("%s", "    + NLS support\n");
#else
  printf("%s", "    - NLS support\n");
#endif /* PR_USE_NLS */

#ifdef PR_USE_OPENSSL
# ifdef PR_USE_OPENSSL_FIPS
    printf("%s", "    + OpenSSL support (FIPS enabled)\n");
# else
    printf("%s", "    + OpenSSL support\n");
# endif /* PR_USE_OPENSSL_FIPS */
#else
  printf("%s", "    - OpenSSL support\n");
#endif /* PR_USE_OPENSSL */

#ifdef PR_USE_PCRE
  printf("%s", "    + PCRE support\n");
#else
  printf("%s", "    - PCRE support\n");
#endif /* PR_USE_PCRE */

#ifdef PR_USE_FACL
  printf("%s", "    + POSIX ACL support\n");
#else
  printf("%s", "    - POSIX ACL support\n");
#endif /* PR_USE_FACL */

#ifdef PR_USE_SHADOW
  printf("%s", "    + Shadow file support\n");
#else
  printf("%s", "    - Shadow file suppport\n");
#endif /* PR_USE_SHADOW */

#ifdef PR_USE_SENDFILE
  printf("%s", "    + Sendfile support\n");
#else
  printf("%s", "    - Sendfile support\n");
#endif /* PR_USE_SENDFILE */

#ifdef PR_USE_TRACE
  printf("%s", "    + Trace support\n");
#else
  printf("%s", "    - Trace support\n");
#endif /* PR_USE_TRACE */

  /* Tunable settings */
  printf("%s", "\n  Tunable Options:\n");
  printf("    PR_TUNABLE_BUFFER_SIZE = %u\n", PR_TUNABLE_BUFFER_SIZE);
  printf("    PR_TUNABLE_DEFAULT_RCVBUFSZ = %u\n", PR_TUNABLE_DEFAULT_RCVBUFSZ);
  printf("    PR_TUNABLE_DEFAULT_SNDBUFSZ = %u\n", PR_TUNABLE_DEFAULT_SNDBUFSZ);
  printf("    PR_TUNABLE_GLOBBING_MAX_MATCHES = %lu\n", PR_TUNABLE_GLOBBING_MAX_MATCHES);
  printf("    PR_TUNABLE_GLOBBING_MAX_RECURSION = %u\n", PR_TUNABLE_GLOBBING_MAX_RECURSION);
  printf("    PR_TUNABLE_HASH_TABLE_SIZE = %u\n", PR_TUNABLE_HASH_TABLE_SIZE);
  printf("    PR_TUNABLE_NEW_POOL_SIZE = %u\n", PR_TUNABLE_NEW_POOL_SIZE);
  printf("    PR_TUNABLE_SCOREBOARD_BUFFER_SIZE = %u\n",
    PR_TUNABLE_SCOREBOARD_BUFFER_SIZE);
  printf("    PR_TUNABLE_SCOREBOARD_SCRUB_TIMER = %u\n",
    PR_TUNABLE_SCOREBOARD_SCRUB_TIMER);
  printf("    PR_TUNABLE_SELECT_TIMEOUT = %u\n", PR_TUNABLE_SELECT_TIMEOUT);
  printf("    PR_TUNABLE_TIMEOUTIDENT = %u\n", PR_TUNABLE_TIMEOUTIDENT);
  printf("    PR_TUNABLE_TIMEOUTIDLE = %u\n", PR_TUNABLE_TIMEOUTIDLE);
  printf("    PR_TUNABLE_TIMEOUTLINGER = %u\n", PR_TUNABLE_TIMEOUTLINGER);
  printf("    PR_TUNABLE_TIMEOUTLOGIN = %u\n", PR_TUNABLE_TIMEOUTLOGIN);
  printf("    PR_TUNABLE_TIMEOUTNOXFER = %u\n", PR_TUNABLE_TIMEOUTNOXFER);
  printf("    PR_TUNABLE_TIMEOUTSTALLED = %u\n", PR_TUNABLE_TIMEOUTSTALLED);
  printf("    PR_TUNABLE_XFER_SCOREBOARD_UPDATES = %u\n\n",
    PR_TUNABLE_XFER_SCOREBOARD_UPDATES);
}

static struct option_help {
  const char *long_opt, *short_opt, *desc;

} opts_help[] = {
  { "--help", "-h",
    "Display proftpd usage"},

  { "--nocollision", "-N",
    "Disable address/port collision checking" },

  { "--nodaemon", "-n",
    "Disable background daemon mode (and send all output to stderr)" },

  { "--quiet", "-q",
    "Don't send output to stderr when running with -n or --nodaemon" },

  { "--debug", "-d [level]",
    "Set debugging level (0-10, 10 = most debugging)" },

  { "--define", "-D [definition]",
    "Set arbitrary IfDefine definition" },

  { "--config", "-c [config-file]",
    "Specify alternate configuration file" },

  { "--persistent", "-p [0|1]",
    "Enable/disable default persistent passwd support" },

  { "--list", "-l",
    "List all compiled-in modules" },

  { "--serveraddr", "-S",
    "Specify IP address for server config" },

  { "--configtest", "-t",
    "Test the syntax of the specified config" },

  { "--settings", "-V",
    "Print compile-time settings and exit" },

  { "--version", "-v",
    "Print version number and exit" },

  { "--version-status", "-vv",
    "Print extended version information and exit" },

  { "--ipv4", "-4",
    "Support IPv4 connections only" },

  { "--ipv6", "-6",
    "Support IPv6 connections" },

  { NULL, NULL, NULL }
};

static void show_usage(int exit_code) {
  struct option_help *h;

  printf("%s", "usage: proftpd [options]\n");
  for (h = opts_help; h->long_opt; h++) {
#ifdef HAVE_GETOPT_LONG
    printf(" %s, %s\n ", h->short_opt, h->long_opt);
#else /* HAVE_GETOPT_LONG */
    printf(" %s\n", h->short_opt);
#endif /* HAVE_GETOPT_LONG */
    printf("    %s\n", h->desc);
  }

  exit(exit_code);
}
#ifdef _QNAP_
static int qnap_proc_fd = -1;
void QNAP_open_skrcv_proc()
{
   if(qnap_proc_fd > 0)
      return;
   qnap_proc_fd=open("/proc/tsinfo/sk_rcv_port",O_RDWR);
}
void QNAP_write_skrcv_proc(int cmd,int port)
{
   char buff[16];
   if(qnap_proc_fd <= 0 || port <= 0 || port >= 65535 )
        return;
   memset(buff,0,16);
   switch(cmd){
      case 0://set
              break;
      case 1://add
              if(port != 20)//port:20 is active data port
                      sprintf(buff,"add %d",port);
              break;
      case 2://del
              if(port != 20)//port:20 is active data port
                      sprintf(buff,"del %d",port);
              break;
      default:
              break;
   }
   if(strlen(buff) > 0)
      write(qnap_proc_fd,buff,strlen(buff));
}
void QNAP_close_skrcv_proc()
{
   if(qnap_proc_fd <= 0)
      return;
   close(qnap_proc_fd);
}
#else
void QNAP_open_skrcv_proc()
{
}
void QNAP_write_skrcv_proc(int cmd,int port)
{
}
void QNAP_close_skrcv_proc()
{
}
/////////////////////////////////////////////////
#endif


//#define Q_DBG
#ifdef Q_DBG
#include <stdarg.h>
static int debug_flag = 0;
static void mylogmsg(char *msg, ...)
{
    FILE *dout;
    char buf[2048];
    va_list args;

    va_start(args, msg);
    vsnprintf(buf, sizeof(buf) - 1, msg, args);
    va_end(args);
    if (debug_flag == 0)
    	dout = fopen("/tmp/ftp.debug", "w");
    else
    	dout = fopen("/tmp/ftp.debug", "a");

    if (dout) {
        fputs(buf, dout);
        fclose(dout);
    }
}
#endif

#ifdef _QNAP_
#include <sys/prctl.h>
static void rpc_proc(void)
{
#ifdef RPC_PROC
    pid_t pid = fork();
    if(pid == 0) {
        pr_proctitle_set("(RPC)");
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        signal(SIGHUP, SIG_IGN);

        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_sigaction = sig_pipe_rpc;
        act.sa_flags = SA_SIGINFO;
        sigaction(SIGUSR2, &act, NULL);

        while(1)
            sleep(1);
        exit(0);
    }
    else {
        rpc_pid = pid;
        pr_log_debug(DEBUG9, "### RPC task %d ###", pid);
    }
#endif
}

static void wanip_proc(void)
{
// Hugo add. get externel for passive mode
  q_shmid = shmget(Q_KEY, Q_TEXTSIZE, IPC_CREAT | 0644);

  if (q_shmid != -1) {
    pid_t pid = fork();
    if (pid == 0) { // child process
      char buf[128];
      char swanip[128], ip[128];
      int b_userwanip = 0;
      int use_ip = 0;
      char *q_shmaddr;
      int success = 0;
      time_t last = 0;
      time_t now = 0;

      pr_proctitle_set("(WAN IP)");
      prctl(PR_SET_PDEATHSIG, SIGKILL);
      signal(SIGHUP, SIG_IGN);

      ip[0] = 0;
      GetProfileString("FTP", "EnableUserWanIp", "FALSE", buf, sizeof(buf));
      if (strcasecmp(buf, "TRUE")) {
        shmctl(q_shmid, IPC_RMID, NULL);
        exit(0);
      }

      GetProfileString("FTP", "WanIp", "", buf, sizeof(buf));
      if (buf[0])
        b_userwanip = 1;

      q_shmaddr = (char *)shmat(q_shmid, NULL, 0);
      q_shmaddr[0] = 0;
      shmdt(q_shmaddr);

      while (1) {
#ifdef Q_DBG
      debug_flag = 0;
      mylogmsg("before getwanip...b_userwanip=%d\n", b_userwanip);
      debug_flag = 1;
#endif
        if (!b_userwanip) {  // get wan ip automatically
          if (!get_external_ip(swanip)) {
            swanip[0] = 0;
          }
        }
        else { // use user set wan ip
          if (is_domain_name(buf)) { // use domain name
            if (!resolve_ip(swanip, buf)) { // fail to resolve it
              swanip[0] = 0;
            }
          }
          else { // use ip
          	strcpy(swanip, buf);
          	if (!is_valid_ip(buf)) {
           	  swanip[0] = 0;
            }
            use_ip = 1;
          }
        }
        if (swanip[0]) {
          q_shmaddr = (char *)shmat(q_shmid, NULL, 0);
          strcpy(q_shmaddr, swanip);
          shmdt(q_shmaddr);
          success = 1;
        }
        else {
          q_shmaddr = (char *)shmat(q_shmid, NULL, 0);
          q_shmaddr[0] = 0;
          shmdt(q_shmaddr);
          success = 0;
        }
        if (use_ip)
        	break;
#ifdef Q_DBG
mylogmsg("continue...\n");
#endif

        now = time(0);
        if(success)
            sleep(60);
        else {
            if((now - last) > 30)
                sleep(5);
            else
                sleep(30);
        }
        last = now;
      }
#ifdef Q_DBG
mylogmsg("exit ftp thread\n");
#endif
      exit(0);
    }
  }
// End
}
#endif

int main(int argc, char *argv[], char **envp) {
  int optc, show_version = 0;
  const char *cmdopts = "D:NVc:d:hlnp:qS:tv46";
  mode_t *main_umask = NULL;
  socklen_t peerlen;
  struct sockaddr peer;
#ifdef _QNAP_
  char _home[32];
#endif

#ifdef HAVE_SET_AUTH_PARAMETERS
  (void) set_auth_parameters(argc, argv);
#endif

#ifdef HAVE_TZSET
  /* Preserve timezone information in jailed environments.
   */
  tzset();
#endif

#ifdef _QNAP_
  if (Get_Profile_Boolean("FTP", "CreateConf", TRUE)) 
      FTP_First_Create_Conf();
  if(psmb)
    SMB_Free_Share_Info(psmb);
  psmb  = SMB_Get_Share_Info();
  GetProfileString ("FTP", "Home", "no", _home, sizeof(_home));  // chroot to ~
  if (!strcasecmp(_home, "yes"))
	g_home = 1;
  else
	g_home = 0;

  custom_root = (Get_Profile_Boolean("FTP", "Enable Custom Root", FALSE)) ? 1 : 0;
  if(custom_root) 
      GetProfileString ("FTP", "Custom Root Path", "/share", custom_root_path, sizeof(custom_root_path));

  GetProfileString ("Samba", "HomeLink", "FALSE", _home, sizeof(_home));  // make home link
  if (!strcasecmp(_home, "TRUE") && !custom_root)
	l_home = 1;
  else
	l_home = 0;
#endif

#ifdef _NVR_
read_nvr_config();
#endif


  memset(&session, 0, sizeof(session));

  pr_proctitle_init(argc, argv, envp);

  /* Seed rand */
  srand((unsigned int) (time(NULL) * getpid()));

  /* getpeername() fails if the fd isn't a socket */
  peerlen = sizeof(peer);
  memset(&peer, 0, peerlen);
  if (getpeername(fileno(stdin), &peer, &peerlen) != -1)
    log_stderr(FALSE);

  /* Open the syslog */
  log_opensyslog(NULL);

  /* Initialize the memory subsystem here */
  init_pools();

  /* Command line options supported:
   *
   * -D parameter       set run-time configuration parameter
   * --define parameter
   * -V
   * --settings         report compile-time settings
   * -c path            set the configuration path
   * --config path
   * -d n               set the debug level
   * --debug n
   * -q                 quiet mode; don't log to stderr when not daemonized
   * --quiet
   * -N                 disable address/port collision checks
   * --nocollision
   * -n                 standalone server does not daemonize, all logging
   * --nodaemon         redirected to stderr
   * -S                 specify the IP address for the 'server config',
   * --serveraddr       rather than using DNS on the hostname
   * -t                 syntax check of the configuration file
   * --configtest
   * -v                 report version number
   * --version
   * -4                 support IPv4 connections only
   * --ipv4
   * -6                 support IPv6 connections
   * --ipv6
   */

  opterr = 0;
  while ((optc =
#ifdef HAVE_GETOPT_LONG
	 getopt_long(argc, argv, cmdopts, opts, NULL)
#else /* HAVE_GETOPT_LONG */
	 getopt(argc, argv, cmdopts)
#endif /* HAVE_GETOPT_LONG */
	 ) != -1) {
    switch (optc) {

    case 'D':
      if (!optarg) {
        pr_log_pri(PR_LOG_WARNING, "fatal: -D requires definition parameter");
        exit(1);
      }

      pr_define_add(optarg, TRUE);
      break;

    case 'V':
      show_settings();
      exit(0);
      break;

    case 'N':
      AddressCollisionCheck = FALSE;
      break;

    case 'n':
      nodaemon++;
#ifdef PR_USE_DEVEL
      pr_pool_debug_set_flags(PR_POOL_DEBUG_FL_OOM_DUMP_POOLS);
#endif
      break;

    case 'q':
      quiet++;
      break;

    case 'd':
      if (!optarg) {
        pr_log_pri(PR_LOG_WARNING, "fatal: -d requires debug level parameter");
        exit(1);
      }
      pr_log_setdebuglevel(atoi(optarg));
      break;

    case 'c':
      if (!optarg) {
        pr_log_pri(PR_LOG_WARNING,
          "fatal: -c requires configuration path parameter");
        exit(1);
      }

      /* Note: we delay sanity-checking the given path until after the FSIO
       * layer has been initialized.
       */
      config_filename = strdup(optarg);
      break;

    case 'l':
      modules_list(PR_MODULES_LIST_FL_SHOW_STATIC);
      exit(0);
      break;

    case 'S':
      if (!optarg) {
        pr_log_pri(PR_LOG_WARNING, "fatal: -S requires IP address parameter");
        exit(1);
      }

      if (pr_netaddr_set_localaddr_str(optarg) < 0) {
        pr_log_pri(PR_LOG_WARNING,
          "fatal: unable to use '%s' as server address: %s", optarg,
          strerror(errno));
        exit(1);
      }
      break;

    case 't':
      syntax_check = 1;
      printf("%s", "Checking syntax of configuration file\n");
      fflush(stdout);
      break;

    /* Note: This is now unused, and should be deprecated in the next release.
     * See Bug#3952 for details.
     */
    case 'p': {
      if (!optarg ||
          (atoi(optarg) != 1 && atoi(optarg) != 0)) {
        pr_log_pri(PR_LOG_WARNING,
          "fatal: -p requires Boolean (0|1) parameter");
        exit(1);
      }

      break;
    }

    case 'v':
      show_version++;
      break;

    case 1:
      show_version = 2;
      break;

    case 'h':
      show_usage(0);
      break;

    case '4':
      pr_netaddr_disable_ipv6();
      break;

    case '6':
      pr_netaddr_enable_ipv6();
      break;

    case '?':
      pr_log_pri(PR_LOG_WARNING, "unknown option: %c", (char) optopt);
      show_usage(1);
      break;
    }
  }

  /* If we have any leftover parameters, it's an error. */
  if (argv[optind]) {
    pr_log_pri(PR_LOG_WARNING, "fatal: unknown parameter: '%s'", argv[optind]);
    exit(1);
  }

  if (show_version &&
      show_version == 1) {
    printf("%s", "ProFTPD Version " PROFTPD_VERSION_TEXT "\n");
    exit(0);
  }

  mpid = getpid();

  /* Install signal handlers */
  install_signal_handlers();

#ifdef _QNAP_
  rpc_proc();
  wanip_proc();
#endif

  /* Initialize sub-systems */
  init_pools();
  init_privs();
  init_log();
  init_regexp();
  init_inet();
  init_netio();
  init_netaddr();
  init_fs();
  init_class();
  free_bindings();
  init_config();
  init_stash();

#ifdef PR_USE_CTRLS
  init_ctrls();
#endif /* PR_USE_CTRLS */

  var_init();
  modules_init();

#ifdef PR_USE_NLS
# ifdef HAVE_LOCALE_H
  /* Initialize the locale based on environment variables. */
  if (setlocale(LC_ALL, "") == NULL) {
    const char *env_lang;

    env_lang = pr_env_get(permanent_pool, "LANG");
    pr_log_pri(PR_LOG_WARNING, "warning: unknown/unsupported LANG environment "
      "variable '%s', ignoring", env_lang);

    setlocale(LC_ALL, "C");

  } else {
    /* Make sure that LC_NUMERIC is always set to "C", so as not to interfere
     * with formatting of strings (like printing out floats in SQL query
     * strings).
     */
    setlocale(LC_NUMERIC, "C");
  }
# endif /* !HAVE_LOCALE_H */

  encode_init();
#endif /* PR_USE_NLS */

#ifdef _QNAP_
  QNAP_open_skrcv_proc();
#endif
  /* Now, once the modules have had a chance to initialize themselves
   * but before the configuration stream is actually parsed, check
   * that the given configuration path is valid.
   */
  if (pr_fs_valid_path(config_filename) < 0) {
    pr_log_pri(PR_LOG_WARNING, "fatal: -c requires an absolute path");
    exit(1);
  }

  pr_parser_prepare(NULL, NULL);

  pr_event_generate("core.preparse", NULL);

  if (pr_parser_parse_file(NULL, config_filename, NULL, 0) == -1) {
    pr_log_pri(PR_LOG_WARNING,
      "fatal: unable to read configuration file '%s': %s", config_filename,
      strerror(errno));
    exit(1);
  }

  if (pr_parser_cleanup() < 0) {
    pr_log_pri(PR_LOG_WARNING,
      "fatal: error processing configuration file '%s': "
      "unclosed configuration section", config_filename);
    exit(1);
  }

  if (fixup_servers(server_list) < 0) {
    pr_log_pri(PR_LOG_WARNING,
      "fatal: error processing configuration file '%s'", config_filename);
    exit(1);
  }

  pr_event_generate("core.postparse", NULL);

  if (show_version &&
      show_version == 2) {

    printf("ProFTPD Version: %s", PROFTPD_VERSION_TEXT " " PR_STATUS "\n");
    printf("  Scoreboard Version: %08x\n", PR_SCOREBOARD_VERSION); 
    printf("  Built: %s\n\n", BUILD_STAMP);

    modules_list(PR_MODULES_LIST_FL_SHOW_VERSION);
    exit(0);
  }

  /* We're only doing a syntax check of the configuration file. */
  if (syntax_check) {
    printf("%s", "Syntax check complete.\n");
    pr_session_end(PR_SESS_END_FL_SYNTAX_CHECK);
  }

  /* After configuration is complete, make sure that passwd, group
   * aren't held open (unnecessary fds for master daemon)
   */
  endpwent();
  endgrent();

  /* Security */
  {
    uid_t *uid = (uid_t *) get_param_ptr(main_server->conf, "UserID", FALSE);
    gid_t *gid = (gid_t *) get_param_ptr(main_server->conf, "GroupID", FALSE);

    daemon_uid = (uid != NULL ? *uid : PR_ROOT_UID);
    daemon_gid = (gid != NULL ? *gid : PR_ROOT_GID);
  }

  if (daemon_uid != PR_ROOT_UID) {
    /* Allocate space for daemon supplemental groups. */
    daemon_gids = make_array(permanent_pool, 2, sizeof(gid_t));

    if (pr_auth_getgroups(permanent_pool, (const char *) get_param_ptr(
        main_server->conf, "UserName", FALSE), &daemon_gids, NULL) < 0) {
      pr_log_debug(DEBUG2, "unable to retrieve daemon supplemental groups");
    }

    if (set_groups(permanent_pool, daemon_gid, daemon_gids) < 0) {
      pr_log_pri(PR_LOG_WARNING, "unable to set daemon groups: %s",
        strerror(errno));
    }
  }

  main_umask = get_param_ptr(main_server->conf, "Umask", FALSE);
  if (main_umask == NULL) {
    umask((mode_t) 0022);

  } else {
    umask(*main_umask);
  }

  /* Give up root and save our uid/gid for later use (if supported)
   * If we aren't currently root, PRIVS_SETUP will get rid of setuid
   * granted root and prevent further uid switching from being attempted.
   */

  PRIVS_SETUP(daemon_uid, daemon_gid)

#ifndef PR_DEVEL_COREDUMP
  /* Test to make sure that our uid/gid is correct.  Try to do this in
   * a portable fashion *gah!*
   */

  if (geteuid() != daemon_uid) {
    pr_log_pri(PR_LOG_ERR, "unable to set UID to %lu, current UID: %lu",
      (unsigned long) daemon_uid, (unsigned long) geteuid());
    exit(1);
  }

  if (getegid() != daemon_gid) {
    pr_log_pri(PR_LOG_ERR, "unable to set GID to %lu, current GID: %lu",
      (unsigned long) daemon_gid, (unsigned long) getegid());
    exit(1);
  }
#endif /* PR_DEVEL_COREDUMP */

  switch (ServerType) {
    case SERVER_STANDALONE:
      standalone_main();
      break;

    case SERVER_INETD:
      /* Reset the variable containing the pid of the master/daemon process;
       * it should only be non-zero in the case of standalone daemons.
       */
      mpid = 0;
      inetd_main();
      break;
  }

#ifdef PR_DEVEL_NO_DAEMON
  PRIVS_ROOT
  chdir(PR_RUN_DIR);
#endif /* PR_DEVEL_NO_DAEMON */

  return 0;
}

#ifndef _INCLUDES_H
#define _INCLUDES_H
/* 
   Unix SMB/CIFS implementation.
   Machine customisation and include handling
   Copyright (C) Andrew Tridgell 1994-1998
   Copyright (C) 2002 by Martin Pool <mbp@samba.org>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../replace/replace.h"

/* make sure we have included the correct config.h */
#ifndef NO_CONFIG_H /* for some tests */
#ifndef CONFIG_H_IS_FROM_SAMBA
#error "make sure you have removed all config.h files from standalone builds!"
#error "the included config.h isn't from samba!"
#endif
#endif /* NO_CONFIG_H */

/* only do the C++ reserved word check when we compile
   to include --with-developer since too many systems
   still have comflicts with their header files (e.g. IRIX 6.4) */

#if !defined(__cplusplus) && defined(DEVELOPER) && defined(__linux__)
#define class #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define private #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define public #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define protected #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define template #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define this #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define new #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define delete #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#define friend #error DONT_USE_CPLUSPLUS_RESERVED_NAMES
#endif

#include "local.h"

#ifdef SUNOS4
/* on SUNOS4 termios.h conflicts with sys/ioctl.h */
#undef HAVE_TERMIOS_H
#endif

#ifdef RELIANTUNIX
/*
 * <unistd.h> has to be included before any other to get
 * large file support on Reliant UNIX. Yes, it's broken :-).
 */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif /* RELIANTUNIX */

#include "system/dir.h"
#include "system/locale.h"
#include "system/time.h"
#include "system/wait.h"

#ifndef HAVE_KRB5_H
#undef HAVE_KRB5
#endif

#ifndef HAVE_LDAP_H
#undef HAVE_LDAP
#endif

#if HAVE_SYS_ATTRIBUTES_H
#include <sys/attributes.h>
#endif

#ifndef ENOATTR
#if defined(ENODATA)
#define ENOATTR ENODATA
#else
#define ENOATTR ENOENT
#endif
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#if HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#if HAVE_NETGROUP_H
#include <netgroup.h>
#endif

/* Special macros that are no-ops except when run under Valgrind on
 * x86.  They've moved a little bit from valgrind 1.0.4 to 1.9.4 */
#if HAVE_VALGRIND_MEMCHECK_H
        /* memcheck.h includes valgrind.h */
#include <valgrind/memcheck.h>
#elif HAVE_VALGRIND_H
#include <valgrind.h>
#endif

/* we support ADS if we want it and have krb5 and ldap libs */
#if defined(WITH_ADS) && defined(HAVE_KRB5) && defined(HAVE_LDAP)
#define HAVE_ADS
#endif

/*
 * Define additional missing types
 */
#if defined(AIX)
typedef sig_atomic_t SIG_ATOMIC_T;
#else
typedef sig_atomic_t volatile SIG_ATOMIC_T;
#endif

#ifndef uchar
#define uchar unsigned char
#endif

/*
 * Types for devices, inodes and offsets.
 */

#ifndef SMB_DEV_T
# define SMB_DEV_T dev_t
#endif

#ifndef LARGE_SMB_DEV_T
#  if (defined(SIZEOF_DEV_T) && (SIZEOF_DEV_T == 8))
#    define LARGE_SMB_DEV_T 1
#  endif
#endif

#ifdef LARGE_SMB_DEV_T
#define SDEV_T_VAL(p, ofs, v) (SIVAL((p),(ofs),(v)&0xFFFFFFFF), SIVAL((p),(ofs)+4,(v)>>32))
#define DEV_T_VAL(p, ofs) ((SMB_DEV_T)(((uint64_t)(IVAL((p),(ofs))))| (((uint64_t)(IVAL((p),(ofs)+4))) << 32)))
#else 
#define SDEV_T_VAL(p, ofs, v) (SIVAL((p),(ofs),v),SIVAL((p),(ofs)+4,0))
#define DEV_T_VAL(p, ofs) ((SMB_DEV_T)(IVAL((p),(ofs))))
#endif

/*
 * Setup the correctly sized inode type.
 */

#ifndef SMB_INO_T
#    define SMB_INO_T ino_t
#endif

#ifndef LARGE_SMB_INO_T
#  if (defined(SIZEOF_INO_T) && (SIZEOF_INO_T == 8))
#    define LARGE_SMB_INO_T 1
#  endif
#endif

#ifdef LARGE_SMB_INO_T
#define SINO_T_VAL(p, ofs, v) SBVAL(p, ofs, v)
#define INO_T_VAL(p, ofs) ((SMB_INO_T)BVAL(p, ofs))
#else 
#define SINO_T_VAL(p, ofs, v) SBVAL(p, ofs, ((uint64_t)(v)) & UINT32_MAX)
#define INO_T_VAL(p, ofs) ((SMB_INO_T)(IVAL((p),(ofs))))
#endif

/* TODO: remove this macros */
#define SBIG_UINT(p, ofs, v) SBVAL(p, ofs, v)
#define BIG_UINT(p, ofs) BVAL(p, ofs)
#define IVAL2_TO_SMB_BIG_UINT(p, ofs) BVAL(p, ofs)

/* this should really be a 64 bit type if possible */
typedef uint64_t br_off;

#define SMB_OFF_T_BITS (sizeof(off_t)*8)

/*
 * Set the define that tells us if we can do 64 bit
 * NT SMB calls.
 */

#define SOFF_T(p, ofs, v) (SIVAL(p,ofs,(v)&0xFFFFFFFF), SIVAL(p,(ofs)+4,(v)>>32))
#define SOFF_T_R(p, ofs, v) (SIVAL(p,(ofs)+4,(v)&0xFFFFFFFF), SIVAL(p,ofs,(v)>>32))
#define IVAL_TO_SMB_OFF_T(buf,off) ((off_t)(( ((uint64_t)(IVAL((buf),(off)))) & ((uint64_t)0xFFFFFFFF) )))

/*
 * Type for stat structure.
 */

struct stat_ex {
	dev_t		st_ex_dev;
	ino_t		st_ex_ino;
	mode_t		st_ex_mode;
	nlink_t		st_ex_nlink;
	uid_t		st_ex_uid;
	gid_t		st_ex_gid;
	dev_t		st_ex_rdev;
	off_t		st_ex_size;
	struct timespec st_ex_atime;
	struct timespec st_ex_mtime;
	struct timespec st_ex_ctime;
	struct timespec st_ex_btime; /* birthtime */
	/* Is birthtime real, or was it calculated ? */
	bool		st_ex_calculated_birthtime;
	blksize_t	st_ex_blksize;
	blkcnt_t	st_ex_blocks;

	uint32_t	st_ex_flags;
	uint32_t	st_ex_mask;
};

typedef struct stat_ex SMB_STRUCT_STAT;

enum timestamp_set_resolution {
	TIMESTAMP_SET_SECONDS = 0,
	TIMESTAMP_SET_MSEC,
	TIMESTAMP_SET_NT_OR_BETTER
};

#ifdef QNAPNAS  // start

#include <linux/version.h>
#include <asm/unistd.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
#define qnap_unlink(a) syscall(__NR_qnap_unlink, a)
#define qnap_rmdir(a) syscall(__NR_qnap_rmdir, a)
#else // LINUX_VERSION_CODE
static inline _syscall1(int, qnap_unlink, char*, arg1);
static inline _syscall1(int, qnap_rmdir, char*, arg1);
#endif // LINUX_VERSION_CODE
//Patch by QNAP: Scan and compare by case insensitive filename
#ifdef __NR_qnap_find_filename
#define qnap_find_filename(a,b) syscall(__NR_qnap_find_filename,a,b)
#else // __NR_qnap_find_filename
#define qnap_find_filename(a,b) -2
#endif // __NR_qnap_find_filename
/////////////////////////////////////////////////////////////
#ifndef NASLOG_H
#define NASLOG_H

#include <time.h>

#define EVENT_DB_FILE "/etc/logs/event.log"
#define CONN_DB_FILE "/etc/logs/conn.log"

#define USER_LENGTH 64
#define IP_LENGTH 64
#define COMP_LENGTH 64
#define DESC_LENGTH 2048
#define RES_LENGTH 1024
#define APP_LENGTH 256

#define DEFAULT_USER "System"

#define DEFAULT_MAX_LOGS 10000
#define DEL_N_LOGS_WHEN_FULL 1

#define DB_BUSY_TIMEOUT_MS 20000 /* milliseconds of timeout when retry */

/* General errors */
#define ERR_BUFFER_OVERFLOW -10
#define ERR_PARAMETER -11
#define ERR_NULL_STRING -12
/* Database errors */
#define ERR_DB_OPEN -20
#define ERR_DB_CREATE_TBL -21
#define ERR_DB_REMOVE_TBL -22
#define ERR_DB_ADD_LOG -23
#define ERR_DB_QUERY -24
#define ERR_DB_ADD_TRIGGER -25
#define ERR_DB_REMOVE_TRIGGER -26
#define ERR_DB_DEL -27
/* Internal errors */
#define ERR_INTERNAL_GETIP -30
#define ERR_INTERNAL_GETHOST -31

/* naslog event definitions */
#define EVENT_TYPE_MIN 0
#define EVENT_TYPE_INFO 0
#define EVENT_TYPE_WARN 1
#define EVENT_TYPE_ERROR 2
#define EVENT_TYPE_MAX 2

#define EVENT_SORT_MIN 0
#define EVENT_SORT_BY_DATE 0
#define EVENT_SORT_BY_DATE_DESC 1
#define EVENT_SORT_BY_TIME 2
#define EVENT_SORT_BY_TIME_DESC 3
#define EVENT_SORT_BY_TYPE 4
#define EVENT_SORT_BY_TYPE_DESC 5
#define EVENT_SORT_BY_USER 6
#define EVENT_SORT_BY_USER_DESC 7
#define EVENT_SORT_BY_IP 8
#define EVENT_SORT_BY_IP_DESC 9
#define EVENT_SORT_BY_COMP 10
#define EVENT_SORT_BY_COMP_DESC 11
#define EVENT_SORT_BY_ID 12
#define EVENT_SORT_BY_ID_DESC 13
#define EVENT_SORT_BY_MSGID 14
#define EVENT_SORT_BY_MSGID_DESC 15
#define EVENT_SORT_BY_TIMET 16
#define EVENT_SORT_BY_TIMET_DESC 17
#define EVENT_SORT_MAX 17

#define EVENT_FILTER_MIN 0
#define EVENT_FILTER_NONE 0
#define EVENT_FILTER_TYPE_IFNO 1
#define EVENT_FILTER_TYPE_WARN 2
#define EVENT_FILTER_TYPE_ERROR 3
#define EVENT_FILTER_MAX 3

#define EVENT_TBL_FIELD_MIN 0
#define EVENT_TBL_FIELD_ID 0
#define EVENT_TBL_FIELD_TYPE 1
#define EVENT_TBL_FIELD_DATE 2
#define EVENT_TBL_FIELD_TIME 3
#define EVENT_TBL_FIELD_USER 4
#define EVENT_TBL_FIELD_IP 5
#define EVENT_TBL_FIELD_COMP 6
#define EVENT_TBL_FIELD_DESC 7
#define EVENT_TBL_FIELD_MSGID 8
#define EVENT_TBL_FIELD_TIMET 9
#define EVENT_TBL_FIELD_MAX 9

/* naslog conn defitions */
#define CONN_TYPE_MIN 0
#define CONN_TYPE_INFO 0
#define CONN_TYPE_WARN 1
#define CONN_TYPE_ERROR 2
#define CONN_TYPE_MAX 2

#define CONN_SORT_MIN 0
#define CONN_SORT_BY_DATE 0
#define CONN_SORT_BY_DATE_DESC 1
#define CONN_SORT_BY_TIME 2
#define CONN_SORT_BY_TIME_DESC 3
#define CONN_SORT_BY_TYPE 4
#define CONN_SORT_BY_TYPE_DESC 5
#define CONN_SORT_BY_USER 6
#define CONN_SORT_BY_USER_DESC 7
#define CONN_SORT_BY_IP 8
#define CONN_SORT_BY_IP_DESC 9
#define CONN_SORT_BY_COMP 10
#define CONN_SORT_BY_COMP_DESC 11
#define CONN_SORT_BY_ID 12
#define CONN_SORT_BY_ID_DESC 13
#define CONN_SORT_BY_SERV 14
#define CONN_SORT_BY_SERV_DESC 15
#define CONN_SORT_BY_ACTION 16
#define CONN_SORT_BY_ACTION_DESC 17
#define CONN_SORT_BY_APP 18
#define CONN_SORT_BY_APP_DESC 19
#define CONN_SORT_MAX 19

#define CONN_FILTER_MIN 0
#define CONN_FILTER_NONE 0
#define CONN_FILTER_TYPE_IFNO 1
#define CONN_FILTER_TYPE_WARN 2
#define CONN_FILTER_TYPE_ERROR 3
#define CONN_FILTER_MAX 3

#define CONN_SERV_MIN 0
#define CONN_SERV UNKNOWN 0
#define CONN_SERV_SAMBA 1
#define CONN_SERV_FTP 2
#define CONN_SERV_HTTP 3
#define CONN_SERV_NFS 4
#define CONN_SERV_AFP 5
#define CONN_SERV_TELNET 6
#define CONN_SERV_SSH 7
#define CONN_SERV_ISCSI 8
#define CONN_SERV_RADIUS 9
#define CONN_SERV_VPN 10
#define CONN_SERV_MAX 10

#define CONN_ACTION_MIN 0
#define CONN_ACTION_UNKNOWN 0
#define CONN_ACTION_DEL 1
#define CONN_ACTION_READ 2
#define CONN_ACTION_WRITE 3
#define CONN_ACTION_OPEN 4
#define CONN_ACTION_MKDIR 5
#define CONN_ACTION_NFSMOUNT_SUCC 6
#define CONN_ACTION_NFSMOUNT_FAIL 7
#define CONN_ACTION_RENAME 8
#define CONN_ACTION_LOGIN_FAIL 9
#define CONN_ACTION_LOGIN_SUCC 10
#define CONN_ACTION_LOGOUT 11
#define CONN_ACTION_NFSUMOUNT 12
#define CONN_ACTION_COPY 13
#define CONN_ACTION_MOVE 14
#define CONN_ACTION_ADD 15
#define CONN_ACTION_AUTH_FAIL 16
#define CONN_ACTION_AUTH_OK 17
#define CONN_ACTION_TRASH_RECOVERY 18
#define CONN_ACTION_ADD_TRANSCODE 19
#define CONN_ACTION_DEL_TRANSCODE 20
#define CONN_ACTION_WATERMARK	21
#define CONN_ACTION_ROTATE		22
#define CONN_ACTION_MAX 22

#define CONN_TBL_FIELD_MIN 0
#define CONN_TBL_FIELD_ID 0
#define CONN_TBL_FIELD_TYPE 1
#define CONN_TBL_FIELD_DATE 2
#define CONN_TBL_FIELD_TIME 3
#define CONN_TBL_FIELD_USER 4
#define CONN_TBL_FIELD_IP 5
#define CONN_TBL_FIELD_COMP 6
#define CONN_TBL_FIELD_RES 7
#define CONN_TBL_FIELD_SERV 8
#define CONN_TBL_FIELD_ACTION 9
#define CONN_TBL_FIELD_APP 10
#define CONN_TBL_FIELD_MAX 10

struct naslog_event {
	int event_id;
	int type;
	struct tm event_datetime;
	char event_user[USER_LENGTH+1];
	char event_ip[IP_LENGTH+1];
	char event_comp[COMP_LENGTH+1];
	char event_desc[DESC_LENGTH+1];
};

struct naslog_event_ex {
	int event_id;
	int type;
	struct tm event_datetime;
	char event_user[USER_LENGTH+1];
	char event_ip[IP_LENGTH+1];
	char event_comp[COMP_LENGTH+1];
	char event_desc[DESC_LENGTH+1];
	char event_msgid[COMP_LENGTH+1];
	time_t event_timet;
};

struct naslog_conn {
	int conn_id;
	int type;
	struct tm conn_datetime;
	char conn_user[USER_LENGTH+1];
	char conn_ip[IP_LENGTH+1];
	char conn_comp[COMP_LENGTH+1];
	char conn_res[RES_LENGTH+1];
	int conn_serv;
	int conn_action;
	char conn_app[APP_LENGTH+1];
};

#endif // NASLOG_H

#ifndef _QLOGENGINED_H_
#define _QLOGENGINED_H_

#define MSG_SIZE	8
struct engine_entry {
	int command;
	struct naslog_event_ex a_eventlog;
	struct naslog_conn a_connlog;
//	char msg[MSG_SIZE];
};

enum {
	W_EVLOG=1,
	W_CONNLOG
};
#endif // _QLOGENGINED_H_



#endif /* QNAPNAS */ // end

/* Our own fstrings */

/*
                  --------------
                 /              \
                /      REST      \
               /        IN        \
              /       PEACE        \
             /                      \
             | The infamous pstring |
             |                      |
             |                      |
             |      7 December      |
             |                      |
             |         2007         |
            *|     *  *  *          | *
   _________)/\\_//(\/(/\)/\//\/\///|_)_______
*/

#ifndef FSTRING_LEN
#define FSTRING_LEN 256
typedef char fstring[FSTRING_LEN];
#endif

/* debug.h need to be included before samba_util.h for the macro SMB_ASSERT */
#include "../lib/util/debug.h"

/* Lists, trees, caching, database... */
#include "../lib/util/samba_util.h"
#include "../lib/util/util_net.h"
#include "../lib/util/attr.h"
#include "../lib/util/tsort.h"
#include "../lib/util/dlinklist.h"

#include <talloc.h>
#include <tevent.h>
#include "util_event.h"

#include "../lib/util/data_blob.h"
#include "../lib/util/time.h"
#include "../lib/util/debug_s3.h"

#include "../libcli/util/ntstatus.h"
#include "../libcli/util/error.h"
#include "../libcli/auth/netlogon_creds_cli.h"
#include "../lib/util/charset/charset.h"
#include "dynconfig/dynconfig.h"
#include "locking.h"
#include "smb_perfcount.h"
#include "smb.h"
#ifdef QNAPNAS
#include "auth.h"
#endif /* QNAPNAS */
#include "../lib/util/byteorder.h"

#include "../lib/util/samba_modules.h"
#include "../lib/util/talloc_stack.h"
#include "../lib/util/smb_threads.h"
#include "../lib/util/smb_threads_internal.h"

/* samba_setXXid functions. */
#include "../lib/util/setid.h"

/***** prototypes *****/
#ifndef NO_PROTO_H
#include "proto.h"
#endif

#include "lib/param/loadparm.h"

/* String routines */

#include "srvstr.h"
#include "safe_string.h"

#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif

#ifndef SIGRTMIN
#define SIGRTMIN NSIG
#endif


#if defined(HAVE_CRYPT16) && defined(HAVE_GETAUTHUID)
#define ULTRIX_AUTH 1
#endif

/* yuck, I'd like a better way of doing this */
#define DIRP_SIZE (256 + 32)

/* default socket options. Dave Miller thinks we should default to TCP_NODELAY
   given the socket IO pattern that Samba uses */
#ifdef TCP_NODELAY
#define DEFAULT_SOCKET_OPTIONS "TCP_NODELAY"
#else
#define DEFAULT_SOCKET_OPTIONS ""
#endif

/* dmalloc -- free heap debugger (dmalloc.org).  This should be near
 * the *bottom* of include files so as not to conflict. */
#ifdef ENABLE_DMALLOC
#  include <dmalloc.h>
#endif


#define MAX_SEC_CTX_DEPTH 8    /* Maximum number of security contexts */


/* add varargs prototypes with printf checking */
/*PRINTFLIKE2 */
int fdprintf(int , const char *, ...) PRINTF_ATTRIBUTE(2,3);
/*PRINTFLIKE1 */
int d_printf(const char *, ...) PRINTF_ATTRIBUTE(1,2);
/*PRINTFLIKE2 */
int d_fprintf(FILE *f, const char *, ...) PRINTF_ATTRIBUTE(2,3);

/* PRINTFLIKE2 */
int fstr_sprintf(fstring s, const char *fmt, ...) PRINTF_ATTRIBUTE(2,3);

int smb_xvasprintf(char **ptr, const char *format, va_list ap) PRINTF_ATTRIBUTE(2,0);

int asprintf_strupper_m(char **strp, const char *fmt, ...) PRINTF_ATTRIBUTE(2,3);
char *talloc_asprintf_strupper_m(TALLOC_CTX *t, const char *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/*
 * Veritas File System.  Often in addition to native.
 * Quotas different.
 */
#if defined(HAVE_SYS_FS_VX_QUOTA_H)
#define VXFS_QUOTA
#endif

#ifdef TRUE
#undef TRUE
#endif
#define TRUE __ERROR__XX__DONT_USE_TRUE

#ifdef FALSE
#undef FALSE
#endif
#define FALSE __ERROR__XX__DONT_USE_FALSE

/* If we have blacklisted mmap() try to avoid using it accidentally by
   undefining the HAVE_MMAP symbol. */

#ifdef MMAP_BLACKLIST
#undef HAVE_MMAP
#endif

void dump_core(void) _NORETURN_;
void exit_server(const char *const reason) _NORETURN_;
void exit_server_cleanly(const char *const reason) _NORETURN_;

#define BASE_RID (0x000003E8L)

#endif /* _INCLUDES_H */

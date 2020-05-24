/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* Define if account management tools should be installed setuid and
   authenticate the callers */
/* #undef ACCT_TOOLS_SETUID */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#define ENABLE_NLS 1

/* Path for faillog file. */
#define FAILLOG_FILE "/var/log/faillog"

/* Define to the type of elements in the array set by `getgroups'. Usually
   this is either `int' or `gid_t'. */
#define GETGROUPS_T gid_t

/* max group name length */
#define GROUP_NAME_MAX_LENGTH 16

/* Define to 1 if you have the declaration of 'pam_fail_delay' */
/* #undef HAS_PAM_FAIL_DELAY */

/* Define to 1 if you have the MacOS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the MacOS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#define HAVE_DCGETTEXT 1

/* Define to 1 if you have the declaration of `PAM_DATA_SILENT', and to 0 if
   you don't. */
/* #undef HAVE_DECL_PAM_DATA_SILENT */

/* Define to 1 if you have the declaration of `PAM_DELETE_CRED', and to 0 if
   you don't. */
/* #undef HAVE_DECL_PAM_DELETE_CRED */

/* Define to 1 if you have the declaration of `PAM_ESTABLISH_CRED', and to 0
   if you don't. */
/* #undef HAVE_DECL_PAM_ESTABLISH_CRED */

/* Define to 1 if you have the declaration of `PAM_NEW_AUTHTOK_REQD', and to 0
   if you don't. */
/* #undef HAVE_DECL_PAM_NEW_AUTHTOK_REQD */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the `fchmod' function. */
#define HAVE_FCHMOD 1

/* Define to 1 if you have the `fchown' function. */
#define HAVE_FCHOWN 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fsync' function. */
#define HAVE_FSYNC 1

/* Define to 1 if you have the `futimes' function. */
#define HAVE_FUTIMES 1

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `getgrgid_r' function. */
#define HAVE_GETGRGID_R 1

/* Define to 1 if you have the `getgrnam_r' function. */
#define HAVE_GETGRNAM_R 1

/* Define to 1 if you have the `getgroups' function. */
#define HAVE_GETGROUPS 1

/* Define to 1 if you have the `gethostname' function. */
#define HAVE_GETHOSTNAME 1

/* Define to 1 if you have the `getpwnam_r' function. */
#define HAVE_GETPWNAM_R 1

/* Define to 1 if you have the `getpwuid_r' function. */
#define HAVE_GETPWUID_R 1

/* Define to 1 if you have the `getspnam' function. */
#define HAVE_GETSPNAM 1

/* Define to 1 if you have the `getspnam_r' function. */
#define HAVE_GETSPNAM_R 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `getusershell' function. */
#define HAVE_GETUSERSHELL 1

/* Define to 1 if you have the `getutent' function. */
#define HAVE_GETUTENT 1

/* Define to 1 if you have the <gshadow.h> header file. */
#define HAVE_GSHADOW_H 1

/* Define if you have the iconv() function and it works. */
/* #undef HAVE_ICONV */

/* Define to 1 if you have the `initgroups' function. */
#define HAVE_INITGROUPS 1

/* Define to 1 if you have the `innetgr' function. */
#define HAVE_INNETGR 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `l64a' function. */
#define HAVE_L64A 1

/* Define to 1 if you have the <lastlog.h> header file. */
#define HAVE_LASTLOG_H 1

/* Define to 1 if you have the `lchown' function. */
#define HAVE_LCHOWN 1

/* Define to 1 if you have the `lckpwdf' function. */
#define HAVE_LCKPWDF 1

/* Defined if you have libcrack. */
/* #undef HAVE_LIBCRACK */

/* Defined if you have the ts&szs cracklib. */
/* #undef HAVE_LIBCRACK_HIST */

/* Defined if it includes *Pw functions. */
/* #undef HAVE_LIBCRACK_PW */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define if struct lastlog has ll_host */
#define HAVE_LL_HOST 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `lstat' function. */
#define HAVE_LSTAT 1

/* Define to 1 if you have the `lutimes' function. */
#define HAVE_LUTIMES 1

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <paths.h> header file. */
#define HAVE_PATHS_H 1

/* Define to 1 if you have the `putgrent' function. */
#define HAVE_PUTGRENT 1

/* Define to 1 if you have the `putpwent' function. */
#define HAVE_PUTPWENT 1

/* Define to 1 if you have the `putspent' function. */
#define HAVE_PUTSPENT 1

/* Define to 1 if you have the `rename' function. */
#define HAVE_RENAME 1

/* Define to 1 if you have the `rmdir' function. */
#define HAVE_RMDIR 1

/* Define to 1 if you have the <rpc/key_prot.h> header file. */
#define HAVE_RPC_KEY_PROT_H 1

/* Define to 1 if you have the <security/openpam.h> header file. */
/* #undef HAVE_SECURITY_OPENPAM_H */

/* Define to 1 if you have the <security/pam_misc.h> header file. */
/* #undef HAVE_SECURITY_PAM_MISC_H */

/* Define to 1 if you have the <selinux/selinux.h> header file. */
/* #undef HAVE_SELINUX_SELINUX_H */

/* Define to 1 if you have the `setgroups' function. */
#define HAVE_SETGROUPS 1

/* Define to 1 if you have the `sgetgrent' function. */
/* #undef HAVE_SGETGRENT */

/* Define to 1 if you have the `sgetpwent' function. */
/* #undef HAVE_SGETPWENT */

/* Define to 1 if you have the `sgetspent' function. */
#define HAVE_SGETSPENT 1

/* Define to 1 if you have the <sgtty.h> header file. */
#define HAVE_SGTTY_H 1

/* Have working shadow group support in libc */
/* #undef HAVE_SHADOWGRP */

/* Define to 1 if you have the `sigaction' function. */
#define HAVE_SIGACTION 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if `st_atim' is member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_ATIM 1

/* Define to 1 if `st_atimensec' is member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_ATIMENSEC */

/* Define to 1 if `st_mtim' is member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_MTIM 1

/* Define to 1 if `st_mtimensec' is member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_MTIMENSEC */

/* Define to 1 if `st_rdev' is member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_RDEV 1

/* Define to 1 if `ut_addr' is member of `struct utmpx'. */
/* #undef HAVE_STRUCT_UTMPX_UT_ADDR */

/* Define to 1 if `ut_addr_v6' is member of `struct utmpx'. */
#define HAVE_STRUCT_UTMPX_UT_ADDR_V6 1

/* Define to 1 if `ut_host' is member of `struct utmpx'. */
#define HAVE_STRUCT_UTMPX_UT_HOST 1

/* Define to 1 if `ut_name' is member of `struct utmpx'. */
/* #undef HAVE_STRUCT_UTMPX_UT_NAME */

/* Define to 1 if `ut_syslen' is member of `struct utmpx'. */
/* #undef HAVE_STRUCT_UTMPX_UT_SYSLEN */

/* Define to 1 if `ut_time' is member of `struct utmpx'. */
/* #undef HAVE_STRUCT_UTMPX_UT_TIME */

/* Define to 1 if `ut_xtime' is member of `struct utmpx'. */
/* #undef HAVE_STRUCT_UTMPX_UT_XTIME */

/* Define to 1 if `ut_addr' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_ADDR 1

/* Define to 1 if `ut_addr_v6' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_ADDR_V6 1

/* Define to 1 if `ut_host' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_HOST 1

/* Define to 1 if `ut_id' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_ID 1

/* Define to 1 if `ut_name' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_NAME 1

/* Define to 1 if `ut_syslen' is member of `struct utmp'. */
/* #undef HAVE_STRUCT_UTMP_UT_SYSLEN */

/* Define to 1 if `ut_time' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_TIME 1

/* Define to 1 if `ut_tv' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_TV 1

/* Define to 1 if `ut_type' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_TYPE 1

/* Define to 1 if `ut_user' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_USER 1

/* Define to 1 if `ut_xtime' is member of `struct utmp'. */
#define HAVE_STRUCT_UTMP_UT_XTIME 1

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the <termio.h> header file. */
#define HAVE_TERMIO_H 1

/* Define to 1 if you have the <ulimit.h> header file. */
#define HAVE_ULIMIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `updwtmp' function. */
#define HAVE_UPDWTMP 1

/* Define to 1 if you have the `updwtmpx' function. */
#define HAVE_UPDWTMPX 1

/* Define to 1 if you have the <utime.h> header file. */
#define HAVE_UTIME_H 1

/* Define to 1 if `utime(file, NULL)' sets file's timestamp to the present. */
#define HAVE_UTIME_NULL 1

/* Define to 1 if you have the <utmpx.h> header file. */
#define HAVE_UTMPX_H 1

/* Define to 1 if you have the <utmp.h> header file. */
#define HAVE_UTMP_H 1

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Path for lastlog file. */
#define LASTLOG_FILE "/var/log/lastlog"

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Location of system mail spool directory. */
#define MAIL_SPOOL_DIR "/var/mail"

/* Name of user's mail spool file if stored in user's home directory. */
/* #undef MAIL_SPOOL_FILE */

/* Name of package */
#define PACKAGE "shadow"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Path to passwd program. */
#define PASSWD_PROGRAM "/usr/bin/passwd"

/* Define to 1 if the C compiler supports function prototypes. */
#define PROTOTYPES 1

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define if login should support the -r flag for rlogind. */
#define RLOGIN 1

/* Define to the ruserok() "success" return value (0 or 1). */
#define RUSEROK 0

/* Define to 1 if the `setpgrp' function takes no argument. */
#define SETPGRP_VOID 1

/* Define to support the shadow group file. */
/* #undef SHADOWGRP */

/* PAM converstation to use */
/* #undef SHADOW_PAM_CONVERSATION */

/* Define to support S/Key logins. */
/* #undef SKEY */

/* Define to support newer BSD S/Key API */
/* #undef SKEY_BSD_STYLE */

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
/* #undef STAT_MACROS_BROKEN */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to support /etc/suauth su access control. */
#define SU_ACCESS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Define to support flushing of nscd caches */
/* #undef USE_NSCD */

/* Define to support Pluggable Authentication Modules */
/* #undef USE_PAM */

/* Define to allow the SHA256 and SHA512 password encryption algorithms */
/* #undef USE_SHA_CRYPT */

/* Define to use syslog(). */
#define USE_SYSLOG 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Define if utmpx should be used */
/* #undef USE_UTMPX */

/* Version number of package */
#define VERSION "4.1.4"

/* Define if you want to enable Audit messages */
/* #undef WITH_AUDIT */

/* Build shadow with SELinux support */
/* #undef WITH_SELINUX */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Path for utmp file. */
#define _UTMP_FILE "/var/run/utmp"

/* Path for wtmp file. */
#define _WTMP_FILE "/var/log/wtmp"

/* Define like PROTOTYPES; this can be used by system headers. */
#define __PROTOTYPES 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

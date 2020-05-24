/*
 *   stunnel       Universal SSL tunnel
 *   Copyright (C) 1998-2010 Michal Trojnara <Michal.Trojnara@mirt.net>
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the
 *   Free Software Foundation; either version 2 of the License, or (at your
 *   option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, see <http://www.gnu.org/licenses>.
 * 
 *   Linking stunnel statically or dynamically with other modules is making
 *   a combined work based on stunnel. Thus, the terms and conditions of
 *   the GNU General Public License cover the whole combination.
 * 
 *   In addition, as a special exception, the copyright holder of stunnel
 *   gives you permission to combine stunnel with free software programs or
 *   libraries that are released under the GNU LGPL and with code included
 *   in the standard release of OpenSSL under the OpenSSL License (or
 *   modified versions of such code, with unchanged license). You may copy
 *   and distribute such a system following the terms of the GNU GPL for
 *   stunnel and the licenses of the other code concerned.
 * 
 *   Note that people who make modified versions of stunnel are not obligated
 *   to grant this special exception for their modified versions; it is their
 *   choice whether to do so. The GNU General Public License gives permission
 *   to release a modified version without this exception; this exception
 *   also makes it possible to release a modified version which carries
 *   forward this exception.
 */

#include "common.h"
#include "prototypes.h"

static void log_raw(const int, const char *, const char *);
static void get_timestamp(int, char *);

static DISK_FILE *outfile=NULL;
static struct LIST { /* single-linked list of log lines */
    struct LIST *next;
    int level;
    char stamp[STRLEN], text[STRLEN];
} *head=NULL, *tail=NULL;
static LOG_MODE mode=LOG_MODE_NONE;

#if !defined(USE_WIN32) && !defined(__vms)

void syslog_open(void) {
    if(global_options.option.syslog)
#ifdef __ultrix__
        openlog("stunnel", 0);
#else
        openlog("stunnel", LOG_CONS|LOG_NDELAY, global_options.facility);
#endif /* __ultrix__ */
}

void syslog_close(void) { /* this function is not currently used */
    if(global_options.option.syslog)
        closelog();
}

#endif /* !defined(USE_WIN32) && !defined(__vms) */

void log_open(void) {
    if(global_options.output_file) { /* 'output' option specified */
        outfile=file_open(global_options.output_file, 1);
        if(!outfile)
            s_log(LOG_ERR, "Unable to open output file: %s",
                global_options.output_file);
    }
    log_flush(LOG_MODE_FULL);
}

void log_close(void) {
    mode=LOG_MODE_NONE;
    if(outfile) {
        file_close(outfile);
        outfile=NULL;
    }
}

void log_flush(LOG_MODE new_mode) {
    struct LIST *tmp;

    /* prevent changing LOG_MODE_FULL to LOG_MODE_ERROR
     * once stderr file descriptor is closed */
    if(mode==LOG_MODE_NONE)
        mode=new_mode;

    while(head) {
        log_raw(head->level, head->stamp, head->text);
        tmp=head;
        head=head->next;
        free(tmp);
    }
    head=tail=NULL;
}

void s_log(int level, const char *format, ...) {
    va_list arglist;
    char text[STRLEN], stamp[STRLEN];
    struct LIST *tmp;

    va_start(arglist, format);
#ifdef HAVE_VSNPRINTF
    vsnprintf(text, STRLEN, format, arglist);
#else
    vsprintf(text, format, arglist);
#endif
    va_end(arglist);

    if(mode==LOG_MODE_NONE) { /* save the text to log it later */
        tmp=malloc(sizeof(struct LIST));
        if(!tmp) /* out of memory */
            return;
        tmp->next=NULL;
        tmp->level=level;
        get_timestamp(level, tmp->stamp);
        safecopy(tmp->text, text);
        if(tail)
            tail->next=tmp;
        else
            head=tmp;
        tail=tmp;
    } else { /* ready log the text directly */
        get_timestamp(level, stamp);
        log_raw(level, stamp, text);
    }
}

static void log_raw(const int level, const char *stamp, const char *text) {
    char line[STRLEN];

    /* build the line and log it to syslog/file */
    if(mode==LOG_MODE_FULL) { /* configured */
        safecopy(line, stamp);
        safeconcat(line, text);
#if !defined(USE_WIN32) && !defined(__vms)
        if(level<=global_options.debug_level && global_options.option.syslog)
            syslog(level, "LOG%d[%lu:%lu]: %s", level,
                stunnel_process_id(), stunnel_thread_id(), text);
#endif /* USE_WIN32, __vms */
        if(level<=global_options.debug_level && outfile)
            file_putline(outfile, line); /* send log to file */
    } else /* LOG_MODE_ERROR */
        safecopy(line, text); /* don't log the time stamp in error mode */

    /* log the line to GUI/stderr */
#ifdef USE_WIN32
    if(mode==LOG_MODE_ERROR || level<=global_options.debug_level)
        win_log(line); /* always log to the GUI window */
#else /* Unix */
    if(mode==LOG_MODE_ERROR || /* always log LOG_MODE_ERROR to stderr */
            (level<=global_options.debug_level &&
            global_options.option.foreground))
        fprintf(stderr, "%s\n", line); /* send log to stderr */
#endif
}

static void get_timestamp(int level, char *txt) {
    time_t gmt;
    struct tm *timeptr;
#if defined(HAVE_LOCALTIME_R) && defined(_REENTRANT)
    struct tm timestruct;
#endif

    time(&gmt);
#if defined(HAVE_LOCALTIME_R) && defined(_REENTRANT)
    timeptr=localtime_r(&gmt, &timestruct);
#else
    timeptr=localtime(&gmt);
#endif
#ifdef HAVE_SNPRINTF
    snprintf(txt, STRLEN,
#else
    sprintf(txt,
#endif
        "%04d.%02d.%02d %02d:%02d:%02d LOG%d[%lu:%lu]: ",
        timeptr->tm_year+1900, timeptr->tm_mon+1, timeptr->tm_mday,
        timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec,
        level, stunnel_process_id(), stunnel_thread_id());
}

void ioerror(const char *txt) { /* input/output error */
    log_error(LOG_ERR, get_last_error(), txt);
}

void sockerror(const char *txt) { /* socket error */
    log_error(LOG_ERR, get_last_socket_error(), txt);
}

void log_error(int level, int error, const char *txt) { /* generic error */
    s_log(level, "%s: %s (%d)", txt, my_strerror(error), error);
}

char *my_strerror(int errnum) {
    switch(errnum) {
#ifdef USE_WIN32
    case 10004:
        return "Interrupted system call (WSAEINTR)";
    case 10009:
        return "Bad file number (WSAEBADF)";
    case 10013:
        return "Permission denied (WSAEACCES)";
    case 10014:
        return "Bad address (WSAEFAULT)";
    case 10022:
        return "Invalid argument (WSAEINVAL)";
    case 10024:
        return "Too many open files (WSAEMFILE)";
    case 10035:
        return "Operation would block (WSAEWOULDBLOCK)";
    case 10036:
        return "Operation now in progress (WSAEINPROGRESS)";
    case 10037:
        return "Operation already in progress (WSAEALREADY)";
    case 10038:
        return "Socket operation on non-socket (WSAENOTSOCK)";
    case 10039:
        return "Destination address required (WSAEDESTADDRREQ)";
    case 10040:
        return "Message too long (WSAEMSGSIZE)";
    case 10041:
        return "Protocol wrong type for socket (WSAEPROTOTYPE)";
    case 10042:
        return "Bad protocol option (WSAENOPROTOOPT)";
    case 10043:
        return "Protocol not supported (WSAEPROTONOSUPPORT)";
    case 10044:
        return "Socket type not supported (WSAESOCKTNOSUPPORT)";
    case 10045:
        return "Operation not supported on socket (WSAEOPNOTSUPP)";
    case 10046:
        return "Protocol family not supported (WSAEPFNOSUPPORT)";
    case 10047:
        return "Address family not supported by protocol family (WSAEAFNOSUPPORT)";
    case 10048:
        return "Address already in use (WSAEADDRINUSE)";
    case 10049:
        return "Can't assign requested address (WSAEADDRNOTAVAIL)";
    case 10050:
        return "Network is down (WSAENETDOWN)";
    case 10051:
        return "Network is unreachable (WSAENETUNREACH)";
    case 10052:
        return "Net dropped connection or reset (WSAENETRESET)";
    case 10053:
        return "Software caused connection abort (WSAECONNABORTED)";
    case 10054:
        return "Connection reset by peer (WSAECONNRESET)";
    case 10055:
        return "No buffer space available (WSAENOBUFS)";
    case 10056:
        return "Socket is already connected (WSAEISCONN)";
    case 10057:
        return "Socket is not connected (WSAENOTCONN)";
    case 10058:
        return "Can't send after socket shutdown (WSAESHUTDOWN)";
    case 10059:
        return "Too many references, can't splice (WSAETOOMANYREFS)";
    case 10060:
        return "Connection timed out (WSAETIMEDOUT)";
    case 10061:
        return "Connection refused (WSAECONNREFUSED)";
    case 10062:
        return "Too many levels of symbolic links (WSAELOOP)";
    case 10063:
        return "File name too long (WSAENAMETOOLONG)";
    case 10064:
        return "Host is down (WSAEHOSTDOWN)";
    case 10065:
        return "No Route to Host (WSAEHOSTUNREACH)";
    case 10066:
        return "Directory not empty (WSAENOTEMPTY)";
    case 10067:
        return "Too many processes (WSAEPROCLIM)";
    case 10068:
        return "Too many users (WSAEUSERS)";
    case 10069:
        return "Disc Quota Exceeded (WSAEDQUOT)";
    case 10070:
        return "Stale NFS file handle (WSAESTALE)";
    case 10091:
        return "Network SubSystem is unavailable (WSASYSNOTREADY)";
    case 10092:
        return "WINSOCK DLL Version out of range (WSAVERNOTSUPPORTED)";
    case 10093:
        return "Successful WSASTARTUP not yet performed (WSANOTINITIALISED)";
    case 10071:
        return "Too many levels of remote in path (WSAEREMOTE)";
    case 11001:
        return "Host not found (WSAHOST_NOT_FOUND)";
    case 11002:
        return "Non-Authoritative Host not found (WSATRY_AGAIN)";
    case 11003:
        return "Non-Recoverable errors: FORMERR, REFUSED, NOTIMP (WSANO_RECOVERY)";
    case 11004:
        return "Valid name, no data record of requested type (WSANO_DATA)";
#if 0
    case 11004: /* typically, only WSANO_DATA is reported */
        return "No address, look for MX record (WSANO_ADDRESS)";
#endif
#endif /* defined USE_WIN32 */
    default:
        return strerror(errnum);
    }
}

/* end of log.c */

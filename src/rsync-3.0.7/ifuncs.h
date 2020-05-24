/* Inline functions for rsync.
 *
 * Copyright (C) 2007-2008 Wayne Davison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, visit the http://fsf.org website.
 */

static inline void
alloc_xbuf(xbuf *xb, size_t sz)
{
	if (!(xb->buf = new_array(char, sz)))
		out_of_memory("alloc_xbuf");
	xb->size = sz;
	xb->len = xb->pos = 0;
}

static inline void
realloc_xbuf(xbuf *xb, size_t sz)
{
	char *bf = realloc_array(xb->buf, char, sz);
	if (!bf)
		out_of_memory("realloc_xbuf");
	xb->buf = bf;
	xb->size = sz;
}

static inline int
to_wire_mode(mode_t mode)
{
#ifdef SUPPORT_LINKS
#if _S_IFLNK != 0120000
	if (S_ISLNK(mode))
		return (mode & ~(_S_IFMT)) | 0120000;
#endif
#endif
	return mode;
}

static inline mode_t
from_wire_mode(int mode)
{
#if _S_IFLNK != 0120000
	if ((mode & (_S_IFMT)) == 0120000)
		return (mode & ~(_S_IFMT)) | _S_IFLNK;
#endif
	return mode;
}

static inline char *
d_name(struct dirent *di)
{
#ifdef HAVE_BROKEN_READDIR
	return (di->d_name - 2);
#else
	return di->d_name;
#endif
}

static inline int
isDigit(const char *ptr)
{
	return isdigit(*(unsigned char *)ptr);
}

static inline int
isPrint(const char *ptr)
{
	return isprint(*(unsigned char *)ptr);
}

static inline int
isSpace(const char *ptr)
{
	return isspace(*(unsigned char *)ptr);
}

static inline int
isLower(const char *ptr)
{
	return islower(*(unsigned char *)ptr);
}

static inline int
isUpper(const char *ptr)
{
	return isupper(*(unsigned char *)ptr);
}

static inline int
toLower(const char *ptr)
{
	return tolower(*(unsigned char *)ptr);
}

static inline int
toUpper(const char *ptr)
{
	return toupper(*(unsigned char *)ptr);
}

// jeff modify 2011.8.26, for limit band width
#if defined(QNAPNAS) && defined(SUPPORT_LIMITRATE)
typedef struct _T_LIMITRATE_
{
	int idPID;						//<<< The process id of child of rsyncd
	int bSender;					//<<< The flag of send/receive(1: send, 0: receive)
	int cbSBwlimit;					//<<< The band width limit of sending.
	int cbRBwlimit;					//<<< The band width limit of receiving.
}T_LIMITRATE, *PT_LIMITRATE;

#define MSEC_PER_SECOND				1000
#define CN_MAX_JOB 					32							//<<< The max amount of job.
#define IKEY_SHM_LIMITRATE_SERVER 	873							//<<< The key of share memory stores value of band width.
#define CN_BWLIMIT_SLOT				CN_MAX_JOB*2				//<<< The amount of slot stored band width limit data in rsync.bwlimit.
#define NB_SHM_LIMITRATE_SERVER		sizeof(T_LIMITRATE)*CN_BWLIMIT_SLOT		//<<< The size of all band width limit data in rsync.bwlimit.
#define TRUE						1							//<<< The define of 1
#define FALSE						0							//<<< The define of 0

#define	SZP_RSYNCD_CONF			"/etc/config/rsyncd.conf"		//<<< The path of rsyncd.cond

#define SZK_RSYNCD_SLIMITRATE	"SLimitRate"					//<<< The key of band width of sending data.
#define SZK_RSYNCD_RLIMITRATE	"RLimitRate"					//<<< The key of band width of receiving data.

#define SZV_RSYNCD_SLIMITRATE_DEF	"0"							//<<< The default value of band width of sending data.
#define SZV_RSYNCD_RLIMITRATE_DEF	"0"							//<<< The default value of band width of receiving data.
#define SZ_OPT_QNAP_BWLIMIT			"--qnap-bwlimit"			//<<< The option of band width limit of qnap.

#define CB_MIN_LIMITRATE	1024								//<<< The minimum limit rate(1KB/sec).

extern int g_bAbort;
extern int daemon_bwlimit;
extern int bwlimit;
extern size_t bwlimit_writemax;
extern int g_bQnapBwlimit;
extern int g_bQnapSSHBwlimit;

/**
 * \brief	Check whether if there data is available on a file descriptor or not.
 * \param	fdSocket	The file descriptor to read.
 * \param	msWait		The mini-second of timeout to check the file descriptor.
 * \return	> 0 if data is available, 0 if timeout, or -XXX if error.
 */
int Is_FD_Ready_To_Read(int fdSocket, int msWait);
#endif /*QNAPNAS && SUPPORT_LIMITRATE*/

extern int g_bSnapshot;
extern char *g_cProgressLog;

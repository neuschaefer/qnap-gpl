/* smbpwd.c Copyright andy phillips 1999 */

/* This code derived from samba 2.0.5a which is - 

 * Unix SMB/Netbios implementation. Version 1.9. SMB parameters and setup
 * Copyright (C) Andrew Tridgell 1992-1998 Modified by Jeremy Allison 1995.
 */
/* This code is licensed under the GNU GPL version 2 or later */
/* smbpasswd file access routines. basically the {get|set|end|put}pwent
 * interface. Error codes as from the standard pwent interface.
 * 
 * Notes: 
 * The location of the smb password file is defined in the makefile
 * at present. 
 * 
 * Most of this code is lifted from the samba 2.0.5a code base. 
 * 
 * If the passwd entries in the smb_passwd structure are null then 
 * that means that no password has been set. 
 * 
 * Limitations: - The maximum line length for a smbpasswd file entry
 * cannot exceed 256 characters at present. 
 */

#include "includes.h"
/*
#ifndef  _SMBENCRYPT_C
#include "smbencrypt.c"
#endif
*/

/* Private copy of smb file descriptor */
static FILE *__smb_password_fd = NULL;

/* flag for smb path setting */
static int __smbpathisset = 0;

/* Static buffer for buffering lines readfrom/writtento the smbpasswd file */
unsigned char __smblinebuf[256];

/* Static buffer to hold file path to smb password file.
 * Since the smbpasswd file can be in many places, 
 * - /etc/smbpasswd, /usr/local/samba/private/smbpasswd or whatever the
 * - value it is set to in the smb.conf file. 
 * we have to provide a hook so that user can set the path. 
 * However the specs say that getpwent opens the passwd file, hence the
 * apparent duplication of flag variables
 */
static char __smbfilepath[PATH_MAX];

/* Persistent static structure for password details */
static struct smb_passwd __smb_passwd;

/* prototypes */
unsigned short int pdb_decode_acct_ctrl(const char *p);
char *pdb_encode_acct_ctrl(unsigned short int acct_ctrl, size_t length);
int pdb_gethexpwd(char *p, char *pwd);

/* temporary */
//#define DEBUG(a,b)  fprintf(stderr,(b),(a));
#define DEBUG(a,b)

#define MAX(a,b)  ((a)>(b) ? (a) : (b))
#define MIN(a,b)  ((a)>(b) ? (b) : (a))

//define by RichardChang@QNAP for QNAP smbpasswd
#define QNAP 1

/* simple wrapper to return the NT and smb passwords */

void smbcrypt (unsigned char *plain, 
	       unsigned char nt_pass[16], 
	       unsigned char smb_pass[16])
{
   nt_lm_owf_gen(plain,nt_pass,smb_pass);
}

/* open the smb password file. called from getsmbpwent */

FILE *opensmbpw(void)
{

    if (!__smbpathisset)
	setsmbfilepath(SMB_PASSWORD_FILE);

    /* open smbpasswd file */

    __smb_password_fd = fopen(__smbfilepath, "rb");

    if (__smb_password_fd == NULL) {
	/* errno handling */
	return NULL;
    }
    return __smb_password_fd;
}				/* opensmbpw */

struct smb_passwd *getsmbpwent(void)
{
    static unsigned char username[32], lm_hash[17], nt_hash[17], gecos[32];

    int smblinebuflen;
    unsigned int c;
    unsigned char happy;
    uid_t uidval;
    unsigned char *tmpptr;
    unsigned int checkpass;

    /* open the file */
    if (__smb_password_fd == NULL) {
	/* either first time opened, or failed last time */

	if (opensmbpw() == NULL) {
	    return NULL;
	}
    }
    memset(&__smb_passwd, 0, sizeof(struct smb_passwd));

    __smb_passwd.smb_acct_ctrl = ACB_NORMAL;
    memset(username, 0, 32);
    memset(lm_hash, 0, 17);
    memset(nt_hash, 0, 17);
    memset(gecos, 0, 32);

    memset(__smblinebuf, 0, 256);

    smblinebuflen = 0;
    happy = 0;

    while (!happy) {
	while (((c = fgetc(__smb_password_fd)) != EOF) && (smblinebuflen < 255)) {
	    __smblinebuf[smblinebuflen++] = c;
	    if (c == '\n') {
		__smblinebuf[smblinebuflen - 1] = '\0';
		break;
	    }
	}
	/* early bailout */

	if (c == EOF)
	    return NULL;

	if (smblinebuflen >= 255) {
	    /* we overran our buffer */
	    errno = ENOMEM;
	    return NULL;
	}
	/* Check for null or commented lines 
	 */
	if (__smblinebuf[0] == '\0' || __smblinebuf[0] == '\n' || __smblinebuf[0] == '#') {
	    happy = 0;
	    smblinebuflen = 0;
	} else
	    happy = 1;

    }				/* do we have a line */

    __smblinebuf[smblinebuflen] = 0;

    /* scanning code lifted from samba 2.0.5a */
    /*
     * The line we have should be of the form :-
     * 
     * username:uid:32hex bytes:[Account type]:LCT-12345678....other flags presently
     * ignored....
     * 
     * or,
     *
     * username:uid:32hex bytes:32hex bytes:[Account type]:LCT-12345678....ignored....
     *
     * if Windows NT compatible passwords are also present.
     * [Account type] is an ascii encoding of the type of account.
     * LCT-(8 hex digits) is the time_t value of the last change time.
     */

    tmpptr = (unsigned char *) strchr(__smblinebuf, ':');
    if (tmpptr == NULL) {
	DEBUG(0, ("getsmbpwent: malformed password entry (no :)\n"));
	return NULL;
    }
    strncpy(username, __smblinebuf, (tmpptr - __smblinebuf));
    username[(tmpptr - __smblinebuf)] = '\0';

    tmpptr++;

    if (*tmpptr == '-') {
	DEBUG(0, ("getsmbfilepwent: uids in the smbpasswd file must not be negative.\n"));
	return NULL;
    }
    if (!isdigit(*tmpptr)) {
	DEBUG(0, ("getsmbfilepwent: malformed password entry (uid not number)\n"));
	return NULL;
    }
    uidval = atoi((char *) tmpptr);

    while (*tmpptr && isdigit(*tmpptr))
	tmpptr++;

    if (*tmpptr != ':') {
	DEBUG(0, ("getsmbfilepwent: malformed password entry (no : after uid)\n"));
	return NULL;
    }
    __smb_passwd.smb_name = username;
    __smb_passwd.smb_userid = uidval;

    /*
     * Now get the password value - this should be 32 hex digits
     * which are the ascii representations of a 16 byte string.
     * Get two at a time and put them into the password.
     */

    /* Skip the ':' */
    tmpptr++;

    checkpass = 1;

#ifndef QNAP
    if (*tmpptr == '*' || *tmpptr == 'X') {
	/* Password deliberately invalid - end here. */
	DEBUG(10, ("getsmbfilepwent: entry invalidated for user %s\n", username));
	__smb_passwd.smb_nt_passwd = NULL;
	__smb_passwd.smb_passwd = NULL;
	__smb_passwd.smb_acct_ctrl |= ACB_DISABLED;
	tmpptr += 92;
	/* yuk! */
	checkpass = 0;
    }
#endif
    if (checkpass) {
	if (smblinebuflen < (tmpptr - __smblinebuf + 33)) {
	    DEBUG(0, ("getsmbfilepwent: malformed password entry (passwd too short)\n"));
	    return NULL;
	}
	if (tmpptr[32] != ':') {
	    DEBUG(0, ("getsmbfilepwent: malformed password entry (no terminating :)\n"));
	    return NULL;
	}
	if (!strncasecmp((char *) tmpptr, "NO PASSWORD", 11)) {
	    __smb_passwd.smb_passwd = NULL;
	    __smb_passwd.smb_acct_ctrl |= ACB_PWNOTREQ;
	} else {
#ifdef QNAP
		if (*tmpptr == '*' || *tmpptr == 'X') {
			__smb_passwd.smb_passwd = NULL;
		} else {
#endif
	    if (!pdb_gethexpwd((char *) tmpptr, (char *) lm_hash)) {
		DEBUG(0, ("getsmbfilepwent: Malformed Lanman password entry (non hex chars)\n"));
		return NULL;
	    }
	    __smb_passwd.smb_passwd = lm_hash;
#ifdef QNAP
		}
#endif
	}
	/* 
	 * Now check if the NT compatible password is
	 * available.
	 */
	__smb_passwd.smb_nt_passwd = NULL;

	tmpptr += 33;		/* Move to the first character of the line after
				   the lanman password. */
	if ((smblinebuflen >= ((tmpptr - __smblinebuf) + 33)) && (tmpptr[32] == ':')) {
	    if (*tmpptr != '*' && *tmpptr != 'X') {
		if (pdb_gethexpwd((char *) tmpptr, (char *) nt_hash))
		    __smb_passwd.smb_nt_passwd = nt_hash;
	    }
	    tmpptr += 33;	/* Move to the first character of the line after
				   the NT password. */
	}
	if (*tmpptr == '[') {
	    __smb_passwd.smb_acct_ctrl = pdb_decode_acct_ctrl((char *) tmpptr);

	    /* Must have some account type set. */
	    if (__smb_passwd.smb_acct_ctrl == 0)
		__smb_passwd.smb_acct_ctrl = ACB_NORMAL;

	    tmpptr += 12;

	    /* Now try and get the last change time. */
	    if (*tmpptr == ']')
		tmpptr++;

	    if (*tmpptr == ':') {
		tmpptr++;
		if (*tmpptr && (strncasecmp((char *) tmpptr, "LCT-", 4) == 0)) {
		    int i;
		    tmpptr += 4;
		    for (i = 0; i < 8; i++) {
			if (tmpptr[i] == '\0' || !isxdigit(tmpptr[i]))
			    break;
		    }
		    if (i == 8) {
			/*
			 * p points at 8 characters of hex digits - 
			 * read into a time_t as the seconds since
			 * 1970 that the password was last changed.
			 */
			__smb_passwd.smb_pwlst = (time_t) strtol((char *) tmpptr, NULL, 16);
			tmpptr += 8;
		    }
		}
	    }
	} else {
	    /* 'Old' style file. Fake up based on user name. */
	    /*
	     * Currently trust accounts are kept in the same
	     * password file as 'normal accounts'. If this changes
	     * we will have to fix this code. JRA.
	     */
	    if (__smb_passwd.smb_name[strlen(__smb_passwd.smb_name) - 1] == '$') {
		__smb_passwd.smb_acct_ctrl &= ~ACB_NORMAL;
		__smb_passwd.smb_acct_ctrl |= ACB_WSTRUST;
	    }
	}
    }
    if (*tmpptr == ':')
	tmpptr++;

    if (*tmpptr != '\0') {
	/* may have a gecos field */
	int tlen = 0;

	tlen = strlen(tmpptr);
	if (tlen > 0)
	    strncpy(gecos, tmpptr, MIN(tlen, 32));

	__smb_passwd.smb_gecos = gecos;

    }
    return (&__smb_passwd);
}

void setsmbpwent(void)
{
    if (__smb_password_fd != NULL)
	fseek(__smb_password_fd, 0, SEEK_SET);
}

void endsmbpwent(void)
{
    if (__smb_password_fd != NULL) {
	fclose(__smb_password_fd);
	__smb_password_fd = NULL;
    }
}

int putsmbpwent(const struct smb_passwd *smbpw, FILE * stream)
{
    int entrylen = 0, i = 0;
    char *p;

    if (!stream || !smbpw) {
	errno = EINVAL;
	return -1;
    }
    memset(__smblinebuf, 0, 255);

    p = __smblinebuf;

    sprintf(p, "%s:%u:", smbpw->smb_name, smbpw->smb_userid);
    entrylen = strlen(p);
    p += entrylen;

    if (smbpw->smb_passwd != NULL) {
	for (i = 0; i < 16; i++) {
	    sprintf((char *) &p[i * 2], "%02X", smbpw->smb_passwd[i]);
	}

    } else {
	i = 0;
	if (smbpw->smb_acct_ctrl & ACB_PWNOTREQ)
	    strncpy((char *) p, "NO PASSWORDXXXXXXXXXXXXXXXXXXXXX", 32);
	else
	    strncpy((char *) p, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 32);
    }

    p += 32;
    *p++ = ':';
    entrylen += 33;

    if (smbpw->smb_nt_passwd != NULL) {
	for (i = 0; i < 16; i++) {
	    sprintf((char *) &p[i * 2], "%02X", smbpw->smb_nt_passwd[i]);
	}
    } else {
	if (smbpw->smb_acct_ctrl & ACB_PWNOTREQ)
	    strncpy((char *) p, "NO PASSWORDXXXXXXXXXXXXXXXXXXXXX", 32);
	else
	    strncpy((char *) p, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 32);
    }

    p += 32;
    *p++ = ':';
    entrylen += 33;

    /* Add the account encoding and the last change time. */
    sprintf((char *) p, "%s:LCT-%08lX:",
       pdb_encode_acct_ctrl(smbpw->smb_acct_ctrl, 14), smbpw->smb_pwlst);

    p += 27;
    entrylen += 27;
    /* add the gecos field */
    if (smbpw->smb_gecos != NULL) {
	sprintf((char *) p, "%s", smbpw->smb_gecos);
	entrylen += strlen(smbpw->smb_gecos);
    }
    *(__smblinebuf + entrylen) = '\n';
    entrylen++;

    fprintf(stderr, "|%s|", __smblinebuf);
    /* write it to the file */
    i = fwrite(__smblinebuf, 1, entrylen, stream);

    if (i < entrylen)
	return -1;

    return 0;
}

/* routine to allow people to supply the path to the smbpasswd file 
 * The default is the redhat value of /etc/smbpasswd set in smbpwd.h */

int setsmbfilepath(char *suggestedpath)
{
    int len;

    memset(__smbfilepath, 0, PATH_MAX);

    len = strlen(suggestedpath);

    if (len >= PATH_MAX)
	return -1;

    strncpy(__smbfilepath, suggestedpath, len);

    __smbpathisset = 1;

    return 1;
}

/* Helper functions from samba-2.0.5a/source/passdb */

/*************************************************************
 Routine to set 32 hex password characters from a 16 byte array.
**************************************************************/

void pdb_sethexpwd(char *p, char *pwd, unsigned short int acct_ctrl)
{
    if (pwd != NULL) {
	int i;
	for (i = 0; i < 16; i++) {
	    snprintf(&p[i * 2], 33, "%02X", pwd[i]);
	}
    } else {
	if ((acct_ctrl && ACB_PWNOTREQ)) {
	    strncpy(p, "NO PASSWORDXXXXXXXXXXXXXXXXXXXXX", 33);
	} else {
	    strncpy(p, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 33);
	}
    }
}

/*************************************************************
 Routine to get the 32 hex characters and turn them
 into a 16 byte array.
**************************************************************/

int pdb_gethexpwd(char *p, char *pwd)
{
    int i;
    unsigned char lonybble, hinybble;
    char *hexchars = "0123456789ABCDEF";
    char *p1, *p2;

    for (i = 0; i < 32; i += 2) {
	hinybble = toupper(p[i]);
	lonybble = toupper(p[i + 1]);

	p1 = strchr(hexchars, hinybble);
	p2 = strchr(hexchars, lonybble);

	if (!p1 || !p2) {
	    return (0);
	}
	hinybble = (p1 - hexchars);
	lonybble = (p2 - hexchars);

	pwd[i / 2] = (hinybble << 4) | lonybble;
    }
    return (1);
}

/**********************************************************
 Decode the account control bits from a string.

 this function breaks coding standards minimum line width of 80 chars.
 reason: vertical line-up code clarity - all case statements fit into
 15 lines, which is more important.
 **********************************************************/

unsigned short int pdb_decode_acct_ctrl(const char *p)
{
    unsigned short int acct_ctrl = 0;
    char finished = 0;

    /*
     * Check if the account type bits have been encoded after the
     * NT password (in the form [NDHTUWSLXI]).
     */

    if (*p != '[')
	return 0;

    for (p++; *p && !finished; p++) {
	switch (*p) {
	case 'N':{
		acct_ctrl |= ACB_PWNOTREQ;
		break;		/* 'N'o password. */
	    }
	case 'D':{
		acct_ctrl |= ACB_DISABLED;
		break;		/* 'D'isabled. */
	    }
	case 'H':{
		acct_ctrl |= ACB_HOMDIRREQ;
		break;		/* 'H'omedir required. */
	    }
	case 'T':{
		acct_ctrl |= ACB_TEMPDUP;
		break;		/* 'T'emp account. */
	    }
	case 'U':{
		acct_ctrl |= ACB_NORMAL;
		break;		/* 'U'ser account (normal). */
	    }
	case 'M':{
		acct_ctrl |= ACB_MNS;
		break;		/* 'M'NS logon user account. What is this ? */
	    }
	case 'W':{
		acct_ctrl |= ACB_WSTRUST;
		break;		/* 'W'orkstation account. */
	    }
	case 'S':{
		acct_ctrl |= ACB_SVRTRUST;
		break;		/* 'S'erver account. */
	    }
	case 'L':{
		acct_ctrl |= ACB_AUTOLOCK;
		break;		/* 'L'ocked account. */
	    }
	case 'X':{
		acct_ctrl |= ACB_PWNOEXP;
		break;		/* No 'X'piry on password */
	    }
	case 'I':{
		acct_ctrl |= ACB_DOMTRUST;
		break;		/* 'I'nterdomain trust account. */
	    }
	case ' ':{
		break;
	    }
	case ':':
	case '\n':
	case '\0':
	case ']':
	default:{
		finished = 1;
	    }
	}
    }

    return acct_ctrl;
}

/*******************************************************************
 gets password-database-format time from a string.
 ********************************************************************/

static time_t get_time_from_string(const char *p)
{
    int i;

    for (i = 0; i < 8; i++) {
	if (p[i] == '\0' || !isxdigit((int) (p[i] & 0xFF)))
	    break;
    }
    if (i == 8) {
	/*
	 * p points at 8 characters of hex digits - 
	 * read into a time_t as the seconds since
	 * 1970 that the password was last changed.
	 */
	return (time_t) strtol(p, NULL, 16);
    }
    return (time_t) - 1;
}

/*******************************************************************
 gets password last set time
 ********************************************************************/

time_t pdb_get_last_set_time(const char *p)
{
    if (*p && strncasecmp(p, "LCT-", 4)) {
	return get_time_from_string(p + 4);
    }
    return (time_t) - 1;
}


/**********************************************************
 Encode the account control bits into a string.
 length = length of string to encode into (including terminating
 null). length *MUST BE MORE THAN 2* !
 **********************************************************/

char *pdb_encode_acct_ctrl(unsigned short int acct_ctrl, size_t length)
{
    static char acct_str[20];
    size_t i = 0;

    memset(acct_str, 0, 20);
    acct_str[i++] = '[';

    if (acct_ctrl & ACB_PWNOTREQ)
	acct_str[i++] = 'N';
    if (acct_ctrl & ACB_DISABLED)
	acct_str[i++] = 'D';
    if (acct_ctrl & ACB_HOMDIRREQ)
	acct_str[i++] = 'H';
    if (acct_ctrl & ACB_TEMPDUP)
	acct_str[i++] = 'T';
    if (acct_ctrl & ACB_NORMAL)
	acct_str[i++] = 'U';
    if (acct_ctrl & ACB_MNS)
	acct_str[i++] = 'M';
    if (acct_ctrl & ACB_WSTRUST)
	acct_str[i++] = 'W';
    if (acct_ctrl & ACB_SVRTRUST)
	acct_str[i++] = 'S';
    if (acct_ctrl & ACB_AUTOLOCK)
	acct_str[i++] = 'L';
    if (acct_ctrl & ACB_PWNOEXP)
	acct_str[i++] = 'X';
    if (acct_ctrl & ACB_DOMTRUST)
	acct_str[i++] = 'I';

    for (; i < length - 2; i++) {
	acct_str[i] = ' ';
    }

    i = length - 2;
    acct_str[i++] = ']';
    acct_str[i++] = '\0';

    return acct_str;
}

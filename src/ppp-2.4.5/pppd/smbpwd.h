/* SMB password reading library  atp 1999, based on code from samba 2.0.5a */
/* This code is licensed under the GPL Version 2 or later (at your option),
 * and constitutes a derived work of the original samba code.
 * 
 * oct 12 1999 - initial revision. atp.
 * 
 * Include this file in your program to use this library.
 * 
 */

#ifndef  _SMBPWD_H
#define _SMBPWD_H	1

/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS    
}
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

__BEGIN_DECLS

/* Allowable account control bits */
#define ACB_DISABLED   0x0001  /* 1 = User account disabled */
#define ACB_HOMDIRREQ  0x0002  /* 1 = Home directory required */
#define ACB_PWNOTREQ   0x0004  /* 1 = User password not required */
#define ACB_TEMPDUP    0x0008  /* 1 = Temporary duplicate account */
#define ACB_NORMAL     0x0010  /* 1 = Normal user account */
#define ACB_MNS        0x0020  /* 1 = MNS logon user account */
#define ACB_DOMTRUST   0x0040  /* 1 = Interdomain trust account */
#define ACB_WSTRUST    0x0080  /* 1 = Workstation trust account */
#define ACB_SVRTRUST   0x0100  /* 1 = Server trust account */
#define ACB_PWNOEXP    0x0200  /* 1 = User password does not expire */
#define ACB_AUTOLOCK   0x0400  /* 1 = Account auto locked */
 
#define MAX_HOURS_LEN 32

struct smb_passwd
{
	uid_t smb_userid;     /* this is actually the unix uid_t */
	char *smb_name;     /* username string */

	unsigned char *smb_passwd; /* Null if no password */
	unsigned char *smb_nt_passwd; /* Null if no password */

	unsigned short int smb_acct_ctrl; /* account info (ACB_xxxx bit-mask) */
	time_t smb_pwlst;    /* password last set time */
        char *smb_gecos;	      /* realname */
};

/* prototypes and forward declarations */  
struct smb_passwd *getsmbpwent(void);
void setsmbpwent(void);
void endsmbpwent(void);
#if 0
int putpwent(const struct smb_passwd *p, FILE *stream);
#endif

/* Function to explicity set the path to the smbpasswd file */
int setsmbfilepath(char *path);

/* Encryption stubs */

void smbcrypt (unsigned char *plain, unsigned char nt_pass[16], unsigned char smb_pass[16]);

/* redhat */
#define SMB_PASSWORD_FILE	  "/etc/smbpasswd"


__END_DECLS

#endif /* smbpwd.h */

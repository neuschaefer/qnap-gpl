/*

 $Id: ssmtp.h,v 1.3 2014/03/31 09:16:30 samsonchan Exp $

 See COPYRIGHT for the license

*/
#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>

#define BUF_SZ  (1024 * 2)	/* A pretty large buffer, but not outrageous */

#define MAXWAIT (10 * 60)	/* Maximum wait between commands, in seconds */
#define MEDWAIT (5 * 60)

#define MAXSYSUID 999		/* Highest UID which is a system account */


// Samson Modified 2014.02.25
#define SAFE_FREE(x)                do{if(x){free(x); x=NULL;}}while(0)
#define SSMTP_eAUTHPASS             "enAuthPass"
#define SSMTP_AUTHPASS              "AuthPass"
#define SSMTP_CONF                  "/etc/config/ssmtp/ssmtp.conf"




#ifndef _POSIX_ARG_MAX
#define MAXARGS 4096
#else
#define MAXARGS  _POSIX_ARG_MAX
#endif

typedef enum {False, True} bool_t;

struct string_list {
	char *string;
	struct string_list *next;
};

typedef struct string_list headers_t;
typedef struct string_list rcpt_t;


/* arpadate.c */
void get_arpadate(char *);

/* base64.c */
void to64frombits(unsigned char *, const unsigned char *, int);
int from64tobits(char *, const char *);

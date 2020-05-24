#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

/*---------------------------------------------------------------------
  type definitions
  */
#ifndef _LIBSMB_TYPEDEFS_
#define _LIBSMB_TYPEDEFS_	1

/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS    }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif
 
__BEGIN_DECLS

#ifdef LINUX
typedef unsigned int  uint32;
typedef unsigned char uchar;
typedef short int     int16;
typedef unsigned short int uint16;
#endif /* LINUX */

#ifdef SOLARIS
typedef unsigned int  uint32;
typedef unsigned char uchar;
typedef short int     int16;
typedef unsigned short int uint16;
#endif /* SOLARIS */

#endif /* _LIBSMB_TYPEDEFS_ */

/*---------------------------------------------------------------------
  function definitions
  */
/* md4.c: */
void mdfour(unsigned char *out, unsigned char *in, int n);

/* smbdes.c: */
void E_P16(unsigned char *p14,unsigned char *p16);
void E_P24(unsigned char *p21, unsigned char *c8, unsigned char *p24);
void D_P16(unsigned char *p14, unsigned char *in, unsigned char *out);
void E_old_pw_hash( unsigned char *p14, unsigned char *in, unsigned char *out);
void cred_hash1(unsigned char *out,unsigned char *in,unsigned char *key);
void cred_hash2(unsigned char *out,unsigned char *in,unsigned char *key);
void cred_hash3(unsigned char *out,unsigned char *in,unsigned char *key, int forw);
void SamOEMhash( unsigned char *data, unsigned char *key, int val);

/* smbencrypt.c: */
void SMBOWFencrypt(uchar passwd[16], uchar *c8, uchar p24[24]);
void strupper (unsigned char *string);
void SMBencrypt(uchar *passwd, uchar *c8, uchar *p24);
void E_md4hash(uchar *passwd, uchar *p16);
void nt_lm_owf_gen(char *pwd, uchar nt_p16[16], uchar p16[16]);
void SMBOWFencrypt(uchar passwd[16], uchar *c8, uchar p24[24]);
void NTLMSSPOWFencrypt(uchar passwd[8], uchar *ntlmchalresp, uchar p24[24]);
void SMBNTencrypt(uchar *passwd, uchar *c8, uchar *p24);
void strupper (unsigned char *string);

/* smbpwd.c: */
unsigned short int pdb_decode_acct_ctrl(const char *p);
char *pdb_encode_acct_ctrl(unsigned short int acct_ctrl, size_t length);
int pdb_gethexpwd(char *p, char *pwd);
FILE *opensmbpw(void);
struct smb_passwd *getsmbpwent(void);
void setsmbpwent(void);
void endsmbpwent(void);
int putsmbpwent(const struct smb_passwd *smbpw, FILE * stream);
int setsmbfilepath(char *suggestedpath);
void pdb_sethexpwd(char *p, char *pwd, unsigned short int acct_ctrl);
int pdb_gethexpwd(char *p, char *pwd);
unsigned short int pdb_decode_acct_ctrl(const char *p);
time_t pdb_get_last_set_time(const char *p);
char *pdb_encode_acct_ctrl(unsigned short int acct_ctrl, size_t length);


#ifndef  _BYTEORDER_H
#include "byteorder.h"
#endif

__END_DECLS

#ifndef  _SMBPWD_H
#include "smbpwd.h"
#endif

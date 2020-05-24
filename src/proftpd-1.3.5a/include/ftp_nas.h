// 2004.11.03, Johnson Cheng marked
// _NAS is declared in Make.rules
// add by Kent 2002/08/20
//#define _NAS            1
#include "NAS.h"
#define STATISTICS      1
#define	SMB_CONFIG_FILE			"/etc/config/smb.conf"

extern SMB_SHARE_INFO *psmb;
int Is_NAS_Default_Root(char *);
int Get_Exact_NAS_User_Name(char *username);
int Check_NAS_User_Password(char *username, char *password);
// end

// add by Johnson Cheng 2004/11/03
#define	SMB_CONFIG_FILE			"/etc/config/smb.conf"
int Get_Private_Profile_String( char* lpAppName,char* lpKeyName,char* lpDefault,char* lpReturnedString,int nSize,char* lpFileName);
// end


//**************************************************************************
//
//	Copyright (c) 2011  QNAP Systems, Inc. All Rights Reserved.
//
//	FILE:
//		rutil.c
//
//	Abstract: 
//		implementation for Rsync utility functions.
//
//	HISTORY:
//			
//      2011/02/15	Jeff Chang create.
//
//**************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <sys/vfs.h>
#include "rsync.h"
#include "Util.h"

#define CBMAX_CMDSIZE			512
#define CBMAX_USERNMAE			32
#define CBMAX_PASSWD			64
#define CBMAX_CMDRET			64
#define SZ_CMD_RDONLY			"r"
#define SZ_EXE_GETCFG			"/sbin/getcfg"
#define SZ_EXE_SETCFG			"/sbin/setcfg"
#define SZ_EXE_RMCFG			"/sbin/rmcfg"
#define SZ_EXE_RUTIL			"/sbin/rsync_util"
#define SZ_EXEARG_CONF			"-f"
#define SZ_EXEARG_RUTIL_USER	"-u"
#define SZ_EXEARG_RUTIL_PASS	"-p"
#define SZ_EXEARG_RUTIL_SHARE	"-s"
#define	DEFAULT_CFG_FILE		"/etc/config/uLinux.conf"

//#define _DEBUGGING_
#ifdef _DEBUGGING_
#define DEBUG_LOG				"/share/Public/rsync.log"
#endif //_DEBUGGING_

FILE *g_pfLog;


/**
 * \brief	Use back slash to encode path.
 * \param	pszPath		The raw path string.
 * \param	pszCode		The encoded path string.
 */
void Path_Encode(const char *pszPath, char *pszCode)
{
	while (*pszPath)
	{
		if (('\\' == *pszPath) || ('\"' == *pszPath) || ('$' == *pszPath))  *pszCode++ = '\\';
		*pszCode++ = *pszPath++;
	}
	*pszCode = '\0';
}

/**
 * \brief	Get value of specific field and section from some config file.
 * \param	pszpConffile		The path string of config file.
 * \param	pszSection			The section name.
 * \param	pszField			The field name.
 * \param	pszResult			The buffer which stores value of field.
 * \param	cbBuf				The buffer length.
 * \return	0 if successful, or -XXX if error.
 */
int Conf_Get_Field(char *pszpConffile, char *pszSection, char *pszField, char *pszResult, int cbBuf)
{
	int iRet = SUCCESS;
	FILE *pfCmdRet = NULL;
	char *pszTmp;
	char szCmd[CBMAX_CMDSIZE];

	if(0 >= cbBuf)
	{
		iRet = -1;
		goto exit_conf_get_field;
	}
	snprintf(szCmd, CBMAX_CMDSIZE, "%s \"%s\" \"%s\" %s %s", SZ_EXE_GETCFG, pszSection, pszField, SZ_EXEARG_CONF, pszpConffile);
	if (NULL != (pfCmdRet = popen(szCmd, SZ_CMD_RDONLY))){
		int i=0;
		do {
			fgets(pszResult, cbBuf, pfCmdRet);
			if (i++ > 50) {
				rprintf(FERROR, "@@ %s:L%d fgets over [%d]th\n",  __FUNCTION__, __LINE__, i);
				iRet = ERROR_NOT_FOUND;
				goto exit_conf_get_field;
			}
			usleep(100000);
		} while (!feof(pfCmdRet));
		if(NULL != (pszTmp = strchr(pszResult, '\n')))
			*pszTmp = 0;
	} else {
		rprintf(FERROR, "@@ %s:L%d popen fail\n",  __FUNCTION__, __LINE__);
		iRet = ERROR_NOT_FOUND;
	}

exit_conf_get_field:

	#ifdef _DEBUGGING_
	g_pfLog = fopen(DEBUG_LOG, "a+");
	fprintf(g_pfLog, "Conf_Get_Field ==> Config: %s, Section: %s, Field: %s\n", pszpConffile, pszSection, pszField);
	fprintf(g_pfLog, "Conf_Get_Field ==> Return: %d,  Result: %s\n", iRet, pszResult);
	fprintf(g_pfLog, "===========================================\n");
	fclose(g_pfLog);
	#endif
	
	if(pfCmdRet)
		pclose(pfCmdRet);
	return iRet;
}

/**
 * \brief	Set value of specific field and section in some config file.
 * \param	pszpConffile		The path string of config file.
 * \param	pszSection			The section name.
 * \param	pszField			The field name.
 * \param	pszFieldData		The data which set to field.
 * \return	0 if successful, or -XXX if error.
 */
int Conf_Set_Field(char *pszpConffile, char *pszSection, char *pszField, char *pszFieldData)
{
	int iRet = SUCCESS;
	FILE *pfCmdRet = NULL;
	char szCmd[CBMAX_CMDSIZE];

	snprintf(szCmd, CBMAX_CMDSIZE, "%s \"%s\" \"%s\" \"%s\" %s %s", SZ_EXE_SETCFG, pszSection, pszField, pszFieldData, SZ_EXEARG_CONF, pszpConffile);
	if(NULL == (pfCmdRet = popen(szCmd, SZ_CMD_RDONLY)))
	{
		iRet = ERROR_NOT_FOUND;
		goto exit_conf_set_field;
	}

exit_conf_set_field:

	#ifdef _DEBUGGING_
	g_pfLog = fopen(DEBUG_LOG, "a+");
	fprintf(g_pfLog, "Conf_Set_Field ==> Config: %s, Section: %s, Field: %s, Data: %s\n", pszpConffile, pszSection, pszField, pszFieldData);
	fprintf(g_pfLog, "Conf_Set_Field ==> Return: %d\n", iRet);
	fprintf(g_pfLog, "===========================================\n");
	fclose(g_pfLog);
	#endif
	
	if(pfCmdRet)
		pclose(pfCmdRet);
	return iRet;
}

/**
 * \brief	Change the value type to string and set it to specific field and section in some config file.
 * \param	pszAppName			The section name.
 * \param	pszKeyName			The field name.
 * \param	iValue				The data which set to field.
 * \return	0 if successful, or -XXX if error.
 */
int  Set_Profile_Integer(char *pszAppName, char *pszKeyName, int iValue)
{
	int iRet = SUCCESS;
	char szValue[64];
	
	sprintf(szValue, "%d", iValue);
	iRet = Conf_Set_Field(DEFAULT_CFG_FILE, pszAppName, pszKeyName, szValue);

	#ifdef _DEBUGGING_
	g_pfLog = fopen(DEBUG_LOG, "a+");
	fprintf(g_pfLog, "Set_Profile_Integer ==> App: %s, Key: %s, Value: %d\n", pszAppName, pszKeyName, iValue);
	fprintf(g_pfLog, "Set_Profile_Integer ==> Return: %d\n", iRet);
	fprintf(g_pfLog, "===========================================\n");
	fclose(g_pfLog);
	#endif

	return iRet;
}

/**
 * \brief	Remove value of specific field and section in some config file.
 * \param	pszpConffile		The path string of config file.
 * \param	pszSection			The section name.
 * \param	pszField			The field name.
 * \return	0 if successful, or -XXX if error.
 */
int Conf_Remove_Field(char *pszpConffile, char *pszSection, char *pszField)
{
	int iRet = SUCCESS;

	iRet = Conf_Set_Field(pszpConffile, pszSection, pszField, "");

	#ifdef _DEBUGGING_
	g_pfLog = fopen(DEBUG_LOG, "a+");
	fprintf(g_pfLog, "Conf_Remove_Field ==> Config: %s, Section: %s, Field: %s\n", pszpConffile, pszSection, pszField);
	fprintf(g_pfLog, "Conf_Remove_Field ==> Return: %d\n", iRet);
	fprintf(g_pfLog, "===========================================\n");
	fclose(g_pfLog);
	#endif
	
	return iRet;
}

/**
 * \brief	Check the username & password to see if it's valid for the local system.
 * \param	pszUserName			The username used to check.
 * \param	pszPassword			The password used to check.
 * \return	0 if successful
 * \		ERROR_BAD_USERNAME	not valid username
 * \		ERROR_BAD_PASSWORD	not correct password
 */
int Check_System_User_Password(char *pszUserName, char *pszPassword)
{
	int iRet = SUCCESS;
	FILE *pfCmdRet = NULL;
	char *pszTmp;
	char szCmd[CBMAX_CMDSIZE], szUserCode[CBMAX_USERNMAE*2+1], szPassCode[CBMAX_PASSWD*2+1], szResult[CBMAX_CMDRET+1];

	Path_Encode(pszUserName, szUserCode);
	Path_Encode(pszPassword, szPassCode);
	
	sprintf(szCmd, "%s %s \"%s\" %s \"%s\"", SZ_EXE_RUTIL, SZ_EXEARG_RUTIL_USER, szUserCode, SZ_EXEARG_RUTIL_PASS, pszPassword);
	if(NULL == (pfCmdRet = popen(szCmd, SZ_CMD_RDONLY)))
	{
		iRet = ERROR_NOT_FOUND+1;
		goto exit_check_system_user_passwd;
	}

	if(NULL == fgets(szResult, CBMAX_CMDRET, pfCmdRet))
		iRet = ERROR_NOT_FOUND+2;
	else
	{
		if(NULL != (pszTmp = strchr(szResult, '\n')))
			*pszTmp = 0;
		iRet = atoi(szResult);
	}

exit_check_system_user_passwd:

	#ifdef _DEBUGGING_
	g_pfLog = fopen(DEBUG_LOG, "a+");
	fprintf(g_pfLog, "Check_System_User_Password ==> Username: %s, Password: %s, Re-coded-Pass: %s\n", pszUserName, pszPassword, szPassCode);
	fprintf(g_pfLog, "Check_System_User_Password ==> Return: %d\n", iRet);
	fprintf(g_pfLog, "===========================================\n");
	fclose(g_pfLog);
	#endif
	
	if(pfCmdRet)
		pclose(pfCmdRet);
	return iRet;
}

/**
 * \brief	Add for other protocol security check requirement. It will check the user is belonged the group which has setted permission on specficed share DIR.
 * \param	pszUserName			The username used to check.
 * \param	pszShareName		The share folder used to check.
 * \return	what permission of the user on the specificed share DIR.
 * \ 		SHARE_NOACCESS		If the user hasn't setted has any permission or invalid user
 * \ 		SHARE_READWRITE		If write list
 * \ 		SHARE_READONLY		If read list
 */
int Get_NAS_User_Security_For_Share(char *pszUserName, char *pszShareName)
{
	int iRet = SUCCESS;
	FILE *pfCmdRet = NULL;
	char *pszTmp;
	char szCmd[CBMAX_CMDSIZE], szUserCode[CBMAX_USERNMAE*2+1], szShareCode[CBMAX_USERNMAE*2+1], szResult[CBMAX_CMDRET+1];

	Path_Encode(pszUserName, szUserCode);
	Path_Encode(pszShareName, szShareCode);

	sprintf(szCmd, "%s %s \"%s\" %s \"%s\"", SZ_EXE_RUTIL, SZ_EXEARG_RUTIL_USER, szUserCode, SZ_EXEARG_RUTIL_SHARE, szShareCode);
	if(NULL == (pfCmdRet = popen(szCmd, SZ_CMD_RDONLY)))
	{
		iRet = ERROR_NOT_FOUND;
		goto exit_nas_user_security_share;
	}

	if(NULL == fgets(szResult, CBMAX_CMDRET, pfCmdRet))
		iRet = ERROR_NOT_FOUND;
	else
	{
		if(NULL != (pszTmp = strchr(szResult, '\n')))
			*pszTmp = 0;
		iRet = atoi(szResult);
	}

exit_nas_user_security_share:

	#ifdef _DEBUGGING_
	g_pfLog = fopen(DEBUG_LOG, "a+");
	fprintf(g_pfLog, "Get_NAS_User_Security_For_Share ==> Username: %s, ShareName: %s\n", pszUserName, pszShareName);
	fprintf(g_pfLog, "Get_NAS_User_Security_For_Share ==> Return: %d\n", iRet);
	fprintf(g_pfLog, "===========================================\n");
	fclose(g_pfLog);
	#endif
	
	if(pfCmdRet)
		pclose(pfCmdRet);
	return iRet;
}

/**
 * \brief	Check the share name is encryption share folder
 * \param	pszShareName	The share folder used to check that only shared name without any slash, path
 * \return	It is encryption share folder.
 * \ 		1		Yes
 * \ 		0		No
 * \ 		Less 0	fail of error no
 */
int Is_EncryptionFolder_For_Share(const char *pszShareName)
{
	char szSharePath[256]={0};
	char szRealPath[256]={0};
	char szEncFlagPath[256]={0};
	struct stat st;

	if (pszShareName == NULL || strlen(pszShareName) == 0)
		return -EINVAL;

	if (strstr(pszShareName,"/"))
		return -EINVAL;

	snprintf(szSharePath, sizeof(szSharePath), "/share/%s", pszShareName);
	if (-1 == readlink(szSharePath, szRealPath, sizeof(szRealPath)-1)) {
		return -errno;
	}

	snprintf(szEncFlagPath, sizeof(szEncFlagPath), "/share/%s/.__eN__%s", dirname(szRealPath), pszShareName);
	if (stat(szEncFlagPath, &st) == 0)
		return 1;

	return 0;
}

/**
 * \brief	Get the Maximum length of filenames in the share name
 * \param	pszShareName	The share folder used to check that only shared name without any slash, path
 * \return	Maximum length of filenames.
 * \ 		On error, -errno is returned.
 */
int Get_FileSystem_Namelen(const char *pszShareName)
{
	char szSharePath[256]={0};
	struct statfs st;

	if (pszShareName == NULL || strlen(pszShareName) == 0)
		return -EINVAL;

	if (strstr(pszShareName,"/"))
		return -EINVAL;

	snprintf(szSharePath, sizeof(szSharePath), "/share/%s", pszShareName);
	if (statfs(szSharePath, &st) == 0)
		return (int)st.f_namelen;
	else
		return -errno;
}
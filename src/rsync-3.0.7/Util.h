//**************************************************************************
//
//	Copyright (c) 2011  QNAP Systems, Inc. All Rights Reserved.
//
//	FILE:
//		rutil.h
//
//	Abstract: 
//		Header for Rsync utility functions.
//
//	HISTORY:
//			
//      2011/02/15	Jeff Chang create.
//
//**************************************************************************
#ifndef	__RUTIL_H__
#define	__RUTIL_H__

#define STATUS_FIELD			"status"

#define S_READY					0
#define	SUCCESS					0
#define	SHARE_NOACCESS			0
#define	SHARE_READWRITE			2

#define	ERROR_NOT_FOUND			-6	// cannot find the specified file or target

#define RSYNC_DEFAULT_SCHEDULE_CONF		"/etc/config/rsync_schedule.conf"
#define RSYNC_SCHEDULE_CONF	rsync_conf
char* rsync_conf;


#define	SZK_RSYNC_PID			"PID"
#define	SZK_RSYNC_PROGRESS		"Progress"
#define	SZK_RSYNC_REMAIN_TIME	"RemainTime"
#define	SZK_RSYNC_PAUSED		"Paused"

//--req#1535, Myron Su, v4.1.2, 2014/09/08
//Write speed data to schedule job config
//Working with '--schedule=' & '-P' progress option
#define	SZK_RSYNC_SPEED			"Speed"
//[ok]--req#1535, Myron Su, v4.1.2, 2014/09/16

/**
 * \brief	Get value of specific field and section from some config file.
 * \param	pszpConffile		The path string of config file.
 * \param	pszSection			The section name.
 * \param	pszField			The field name.
 * \param	pszResult			The buffer which stores value of field.
 * \param	cbBuf				The buffer length.
 * \return	0 if successful, or -XXX if error.
 */
int Conf_Get_Field(char *pszpConffile, char *pszSection, char *pszField, char *pszResult, int cbBuf);

/**
 * \brief	Set value of specific field and section in some config file.
 * \param	pszpConffile		The path string of config file.
 * \param	pszSection			The section name.
 * \param	pszField			The field name.
 * \param	pszFieldData		The data which set to field.
 * \return	0 if successful, or -XXX if error.
 */
int Conf_Set_Field(char *pszpConffile, char *pszSection, char *pszField, char *pszFieldData);

/**
 * \brief	Change the value type to string and set it to specific field and section in some config file.
 * \param	pszAppName			The section name.
 * \param	pszKeyName			The field name.
 * \param	iValue				The data which set to field.
 * \return	0 if successful, or -XXX if error.
 */
int  Set_Profile_Integer(char *pszAppName, char *pszKeyName, int iValue);

/**
 * \brief	Remove value of specific field and section in some config file.
 * \param	pszpConffile		The path string of config file.
 * \param	pszSection			The section name.
 * \param	pszField			The field name.
 * \return	0 if successful, or -XXX if error.
 */
int Conf_Remove_Field(char *pszpConffile, char *pszSection, char *pszField);

/**
 * \brief	Check the username & password to see if it's valid for the local system.
 * \param	pszUserName			The username used to check.
 * \param	pszPassword			The password used to check.
 * \return	0 if successful
 * \		ERROR_BAD_USERNAME	not valid username
 * \		ERROR_BAD_PASSWORD	not correct password
 */
int Check_System_User_Password(char *pszUserName, char *pszPassword);

/**
 * \brief	Add for other protocol security check requirement. It will check the user is belonged the group which has setted permission on specficed share DIR.
 * \param	pszUserName			The username used to check.
 * \param	pszShareName		The share folder used to check.
 * \return	what permission of the user on the specificed share DIR.
 * \ 		SHARE_NOACCESS		If the user hasn't setted has any permission or invalid user
 * \ 		SHARE_READWRITE		If write list
 * \ 		SHARE_READONLY		If read list
 */
int Get_NAS_User_Security_For_Share(char *pszUserName, char *pszShareName);

/**
 * \brief	Check the share name is encryption share folder
 * \param	pszShareName	The share folder used to check that only shared name without any slash, path
 * \return	It is encryption share folder.
 * \ 		1		Yes
 * \ 		0		No
 * \ 		Less 0	fail of error no
 */
int Is_EncryptionFolder_For_Share(const char *pszShareName);

/**
 * \brief	Get the Maximum length of filenames in the share name
 * \param	pszShareName	The share folder used to check that only shared name without any slash, path
 * \return	Maximum length of filenames.
 * \ 		On error, -errno is returned.
 */
int Get_FileSystem_Namelen(const char *pszShareName);
#endif //__RUTIL_H__

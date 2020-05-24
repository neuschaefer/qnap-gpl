/*
 * Support rsync daemon authentication.
 *
 * Copyright (C) 1998-2000 Andrew Tridgell
 * Copyright (C) 2002-2009 Wayne Davison
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

#include "rsync.h"

extern char *password_file;
#ifdef QNAPNAS
#include "Util.h"

#define	SZK_RSYNCD_AUTH			"Rsync Model"
#define	SZK_RSYNCD_USER			"rsync user"
#define	SZK_RSYNCD_PSWD			"rsync pswd"
#define	SZK_RSYNCD_ENPSWD		"rsync enpswd"
#define	SZV_RSYNCD_AUTH			"NORMAL"			// "QNAP", "NORMAL"
#define	SZV_RSYNCD_USER_DEF		"rsync"
#define	SZV_RSYNCD_PSWD_DEF		"rsync"
#define	SZP_RSYNCD_CONF			"/etc/config/rsyncd.conf"
#define	SZP_ULINUX_CONF			"/etc/config/uLinux.conf"
#define	SZP_RSYNC_SCHEDULE_CONF	"/etc/config/rsync_schedule.conf"

extern char *password;	//Richard 20070605 add
extern int sever_mode;//Richard 20080111 add
extern char *pszSchedule;	// current schedule name
extern int iForceRsyncUser;
extern char *config_file;
#endif

/***************************************************************************
encode a buffer using base64 - simple and slow algorithm. null terminates
the result.
  ***************************************************************************/
void base64_encode(const char *buf, int len, char *out, int pad)
{
	char *b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int bit_offset, byte_offset, idx, i;
	const uchar *d = (const uchar *)buf;
	int bytes = (len*8 + 5)/6;

	for (i = 0; i < bytes; i++) {
		byte_offset = (i*6)/8;
		bit_offset = (i*6)%8;
		if (bit_offset < 3) {
			idx = (d[byte_offset] >> (2-bit_offset)) & 0x3F;
		} else {
			idx = (d[byte_offset] << (bit_offset-2)) & 0x3F;
			if (byte_offset+1 < len) {
				idx |= (d[byte_offset+1] >> (8-(bit_offset-2)));
			}
		}
		out[i] = b64[idx];
	}

	while (pad && (i % 4))
		out[i++] = '=';

	out[i] = '\0';
}

/* Generate a challenge buffer and return it base64-encoded. */
static void gen_challenge(const char *addr, char *challenge)
{
	char input[32];
	char digest[MAX_DIGEST_LEN];
	struct timeval tv;
	int len;

	memset(input, 0, sizeof input);

	strlcpy(input, addr, 17);
	sys_gettimeofday(&tv);
	SIVAL(input, 16, tv.tv_sec);
	SIVAL(input, 20, tv.tv_usec);
	SIVAL(input, 24, getpid());

	sum_init(0);
	sum_update(input, sizeof input);
	len = sum_end(digest);

	base64_encode(digest, len, challenge, 0);
}


/* Return the secret for a user from the secret file, null terminated.
 * Maximum length is len (not counting the null). */
static int get_secret(int module, const char *user, char *secret, int len)
{
	const char *fname = lp_secrets_file(module);
	STRUCT_STAT st;
	int fd, ok = 1;
	const char *p;
	char ch, *s;

	if (!fname || !*fname)
		return 0;

	if ((fd = open(fname, O_RDONLY)) < 0)
		return 0;

	if (do_stat(fname, &st) == -1) {
		rsyserr(FLOG, errno, "stat(%s)", fname);
		ok = 0;
	} else if (lp_strict_modes(module)) {
		if ((st.st_mode & 06) != 0) {
			rprintf(FLOG, "secrets file must not be other-accessible (see strict modes option)\n");
			ok = 0;
		} else if (MY_UID() == 0 && st.st_uid != 0) {
			rprintf(FLOG, "secrets file must be owned by root when running as root (see strict modes)\n");
			ok = 0;
		}
	}
	if (!ok) {
		rprintf(FLOG, "continuing without secrets file\n");
		close(fd);
		return 0;
	}

	if (*user == '#') {
		/* Reject attempt to match a comment. */
		close(fd);
		return 0;
	}

	/* Try to find a line that starts with the user name and a ':'. */
	p = user;
	while (1) {
		if (read(fd, &ch, 1) != 1) {
			close(fd);
			return 0;
		}
		if (ch == '\n')
			p = user;
		else if (p) {
			if (*p == ch)
				p++;
			else if (!*p && ch == ':')
				break;
			else
				p = NULL;
		}
	}

	/* Slurp the secret into the "secret" buffer. */
	s = secret;
	while (len > 0) {
		if (read(fd, s, 1) != 1 || *s == '\n')
			break;
		if (*s == '\r')
			continue;
		s++;
		len--;
	}
	*s = '\0';
	close(fd);

	return 1;
}

static const char *getpassf(const char *filename)
{
	STRUCT_STAT st;
	char buffer[512], *p;
	int fd, n, ok = 1;
	const char *envpw = getenv("RSYNC_PASSWORD");

	if (!filename)
		return NULL;

	if ((fd = open(filename,O_RDONLY)) < 0) {
		rsyserr(FWARNING, errno, "could not open password file \"%s\"",
			filename);
		if (envpw)
			rprintf(FINFO, "falling back to RSYNC_PASSWORD environment variable.\n");
		return NULL;
	}

	if (do_stat(filename, &st) == -1) {
		rsyserr(FWARNING, errno, "stat(%s)", filename);
		ok = 0;
	} else if ((st.st_mode & 06) != 0) {
		rprintf(FWARNING, "password file must not be other-accessible\n");
		ok = 0;
	} else if (MY_UID() == 0 && st.st_uid != 0) {
		rprintf(FWARNING, "password file must be owned by root when running as root\n");
		ok = 0;
	}
	if (!ok) {
		close(fd);
		rprintf(FWARNING, "continuing without password file\n");
		if (envpw)
			rprintf(FINFO, "falling back to RSYNC_PASSWORD environment variable.\n");
		return NULL;
	}

	n = read(fd, buffer, sizeof buffer - 1);
	close(fd);
	if (n > 0) {
		buffer[n] = '\0';
		if ((p = strtok(buffer, "\n\r")) != NULL)
			return strdup(p);
	}

	return NULL;
}

/* Generate an MD4 hash created from the combination of the password
 * and the challenge string and return it base64-encoded. */
static void generate_hash(const char *in, const char *challenge, char *out)
{
	char buf[MAX_DIGEST_LEN];
	int len;

	sum_init(0);
	sum_update(in, strlen(in));
	sum_update(challenge, strlen(challenge));
	len = sum_end(buf);

	base64_encode(buf, len, out, 0);
}

/* Possibly negotiate authentication with the client.  Use "leader" to
 * start off the auth if necessary.
 *
 * Return NULL if authentication failed.  Return "" if anonymous access.
 * Otherwise return username.
 */
char *auth_server(int f_in, int f_out, int module, const char *host,
		  const char *addr, const char *leader)
{
	char *users = lp_auth_users(module);
	char challenge[MAX_DIGEST_LEN*2];
	char line[BIGPATHBUFLEN];
	char secret[512];
	char pass2[MAX_DIGEST_LEN*2];
	char *tok, *pass;
#ifdef QNAPNAS
	//Richard 20070921 add check user for share
	char *name = lp_name(module);
	int g_access=SHARE_NOACCESS;
#endif
#ifdef QNAPNAS
	if(sever_mode == 0){
#endif
	/* if no auth list then allow anyone in! */
	if (!users || !*users)
		return "";
#ifdef QNAPNAS
	}
#endif
	gen_challenge(addr, challenge);

	io_printf(f_out, "%s%s\n", leader, challenge);

	if (!read_line_old(f_in, line, sizeof line)
	 || (pass = strchr(line, ' ')) == NULL) {
		rprintf(FLOG, "auth failed on module %s from %s (%s): "
			"invalid challenge response\n",
			lp_name(module), host, addr);
		return NULL;
	}
	*pass++ = '\0';
	
#ifdef QNAPNAS
	if(sever_mode == 1){
		char	szAuth[64]="", szUser[64]="", szPswd[128]="";
		
		char	regPw[512]={0},
				regEnPw[512]={0},
				cmd[576] ={0}; // 512+64
		
		FILE	*fp_encode= NULL,
				*fp_decode= NULL;


		// Check if the rsyncd is accepting the connections from the orginal rsync clients.
		if ((SUCCESS == Conf_Get_Field(SZP_ULINUX_CONF, "System", SZK_RSYNCD_AUTH, szAuth, sizeof(szAuth))) && !strcmp(SZV_RSYNCD_AUTH, szAuth) || iForceRsyncUser )
		{
			char conf_path[128] = {0};
			if(config_file){
			    strncpy(conf_path, config_file, sizeof(conf_path));
			}
			else{
			    strncpy(conf_path, SZP_RSYNCD_CONF, sizeof(conf_path));
			}
			if (SUCCESS != Conf_Get_Field(conf_path, "", SZK_RSYNCD_USER, szUser, sizeof(szUser)))  strcpy(szUser, SZV_RSYNCD_USER_DEF);
			// Samson 20140828 add encode/decode mechanism for password
			if (SUCCESS != Conf_Get_Field(conf_path, "", SZK_RSYNCD_ENPSWD, regEnPw, sizeof(regEnPw))){ // "rsync enpswd" doesn't exist
				if (SUCCESS != Conf_Get_Field(conf_path, "", SZK_RSYNCD_PSWD, regPw, sizeof(regPw))){ // "rsync pswd" doesn't exist
					strncpy(szPswd, SZV_RSYNCD_PSWD_DEF, sizeof(szPswd));
				}else{ // "rsync pswd" exists
					// Encode then write "rsync enpswd" to config and remove "rsync pswd"
					sprintf(cmd, "/sbin/get_encstr %s e 2>/dev/null", regPw);
					if (NULL != (fp_encode = popen(cmd, "r"))){
						fgets(regEnPw, sizeof(regEnPw), fp_encode);
						pclose(fp_encode);
					}
					regEnPw[strlen(regEnPw)] = '\0'; // result no new line
					Conf_Set_Field(conf_path, "", SZK_RSYNCD_ENPSWD, regEnPw);
					Conf_Remove_Field(conf_path, "", SZK_RSYNCD_PSWD); // not really remove, just set null string
					// Decode to use
					memset(cmd, 0, sizeof(cmd));
					sprintf(cmd, "/sbin/get_encstr %s d 2>/dev/null", regEnPw);
					if (NULL != (fp_decode = popen(cmd, "r"))){
						fgets(szPswd, sizeof(szPswd), fp_decode);
						pclose(fp_decode);
					}
					szPswd[strlen(szPswd)] = '\0'; // result no new line
				}
			}else{ // "rsync enpswd" exists, Decode to use
				sprintf(cmd, "/sbin/get_encstr %s d 2>/dev/null", regEnPw);
				if (NULL != (fp_decode = popen(cmd, "r"))){
					fgets(szPswd, sizeof(szPswd), fp_decode);
					pclose(fp_decode);
				}
				szPswd[strlen(szPswd)] = '\0'; // result no new line
			}
			memset(pass2, 0, sizeof(pass2));
			memset(secret, 0, sizeof(secret));
			strcpy(secret, szPswd);
			generate_hash(secret, challenge, pass2);
			if (!strcmp(szUser, line) && !strcmp(pass2, pass))
			{
				//printf("The user:password pairs are matched!\n");
				goto pass_rsync_auth;
			}
		}
		
		// Take the first 16 characters.
//		if (16 < strlen(pass))  pass[16] = 0;

		//Richard 20070605 add check user for system
		if(Check_System_User_Password(line, pass) != 0){
			return NULL;
		}
		// Albert 20090924: Don't check security for share if the path is "".
		if (*name)
		{
			//Richard 20070921 add check user for share
			g_access = Get_NAS_User_Security_For_Share(line, name);
			if (g_access!=SHARE_READWRITE){
				return NULL;
			}
		}
	}else{
#endif
	if (!(users = strdup(users)))
		out_of_memory("auth_server");

	for (tok = strtok(users, " ,\t"); tok; tok = strtok(NULL, " ,\t")) {
		if (wildmatch(tok, line))
			break;
	}
	free(users);

	if (!tok) {
		rprintf(FLOG, "auth failed on module %s from %s (%s): "
			"unauthorized user\n",
			lp_name(module), host, addr);
		return NULL;
	}

	memset(secret, 0, sizeof secret);
	if (!get_secret(module, line, secret, sizeof secret - 1)) {
		memset(secret, 0, sizeof secret);
		rprintf(FLOG, "auth failed on module %s from %s (%s): "
			"missing secret for user \"%s\"\n",
			lp_name(module), host, addr, line);
		return NULL;
	}

	generate_hash(secret, challenge, pass2);
	memset(secret, 0, sizeof secret);

	if (strcmp(pass, pass2) != 0) {
		rprintf(FLOG, "auth failed on module %s from %s (%s): "
			"password mismatch\n",
			lp_name(module), host, addr);
		return NULL;
	}
#ifdef QNAPNAS
	}
pass_rsync_auth:
#endif
	return strdup(line);
}

#ifdef QNAPNAS
static int rsync_passwd_transfer(char *in_str, char *out_str, int buf_size)
{
	int i = 0, j = 0;

	for (i=0; in_str[i]!='\0'; i++) {
		if ((in_str[i] == '`') || (in_str[i] == '$') || (in_str[i] == '"') || (in_str[i] == '\\')) {
			if ( (j+1) < buf_size) {
				out_str[j] = '\\';
				out_str[j+1] = in_str[i];
				j = j+2;
			}
			else
				break;
		}
		else {
			out_str[j] = in_str[i];
			j++;
		}
	}
	out_str[j]=0;
	return 0;
}
#endif

void auth_client(int fd, const char *user, const char *challenge)
{
	const char *pass;
	char pass2[MAX_DIGEST_LEN*2];

	if (!user || !*user)
		user = "nobody";
#ifdef QNAPNAS
	char szPswd[128]= "";
	char changePswd[128]= "";

	char	regPw[512]={0},
			regEnPw[512]={0},
			cmd[576] ={0}; // 512+64
	
	FILE	*fp_encode= NULL,
			*fp_decode= NULL;
	
	if(sever_mode == 1){
		//Richard 20120914 add get password form file
		if(pszSchedule){
			// Samson 20140828 add encode/decode mechanism for password
			if (SUCCESS != Conf_Get_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "EnPassword", regEnPw, sizeof(regEnPw))) { // "EnPassword" doesn't exist
				if (SUCCESS != Conf_Get_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "Password", regPw, sizeof(regPw))) { // "Password" doesn't exist
					rsync_passwd_transfer(password, changePswd, sizeof(changePswd));
				}else{ //"Password" exists
					// Encode then write "EnPassword" to config and remove "Password"
					sprintf(cmd, "/sbin/get_encstr %s e 2>/dev/null", regPw);
					if (NULL != (fp_encode = popen(cmd, "r"))){
						fgets(regEnPw, sizeof(regEnPw), fp_encode);
						pclose(fp_encode);
					}
					regEnPw[strlen(regEnPw)] = '\0'; // result no new line
					Conf_Set_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "EnPassword", regEnPw);
					Conf_Remove_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "Password"); // not really remove, just set null string
					// Decode to use
					memset(cmd, 0, sizeof(cmd));
					sprintf(cmd, "/sbin/get_encstr %s d 2>/dev/null", regEnPw);
					if (NULL != (fp_decode = popen(cmd, "r"))){
						fgets(szPswd, sizeof(szPswd), fp_decode);
						pclose(fp_decode);
					}
					szPswd[strlen(szPswd)] = '\0'; // result no new line
					rsync_passwd_transfer(szPswd, changePswd, sizeof(changePswd));
				}
			}
			else{ // "EnPassword" exists
				sprintf(cmd, "/sbin/get_encstr %s d 2>/dev/null", regEnPw);
				if (NULL != (fp_decode = popen(cmd, "r"))){
					int i=0;
					do {
						fgets(szPswd, sizeof(szPswd), fp_decode);
						if (i++ > 20) {
							rprintf(FLOG, "@@ %s:L%d fgets szPswd NULL over [%d]th\n",  __FUNCTION__, __LINE__, i);
							break;
						}
					} while (!feof(fp_decode));
					pclose(fp_decode);
				}
				szPswd[strlen(szPswd)] = '\0'; // result no new line
				rsync_passwd_transfer(szPswd, changePswd, sizeof(changePswd));
			}
		}
		else{
			//Richard 20070605 add for rysnc can send password for system user
			//pass = password;
			rsync_passwd_transfer(password, changePswd, sizeof(changePswd));
		}
		pass = changePswd;
		if (!pass)
			pass = "";
		
		io_printf(fd, "%s %s\n", user, pass);
	}else{
#else
	if (!(pass = getpassf(password_file))
	 && !(pass = getenv("RSYNC_PASSWORD"))) {
		/* XXX: cyeoh says that getpass is deprecated, because
		 * it may return a truncated password on some systems,
		 * and it is not in the LSB.
                 *
                 * Andrew Klein says that getpassphrase() is present
                 * on Solaris and reads up to 256 characters.
                 *
                 * OpenBSD has a readpassphrase() that might be more suitable.
                 */
		pass = getpass("Password: ");
	}
#endif
#ifdef QNAPNAS
	//Richard 20120914 add get password form file
	if(pszSchedule){
		// Samson 20140828 add encode/decode mechanism for password
		if (SUCCESS != Conf_Get_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "EnPassword", regEnPw, sizeof(regEnPw))){ // "EnPassword" doesn't exist
			if (SUCCESS != Conf_Get_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "Password", regPw, sizeof(regPw))){ // "Password" doesn't exist
				pass = password;
			}else{ // "Password" exists
				// Encode then write "EnPassword" to config and remove "Password"
				sprintf(cmd, "/sbin/get_encstr %s e 2>/dev/null", regPw);
				if (NULL != (fp_encode = popen(cmd, "r"))){
					fgets(regEnPw, sizeof(regEnPw), fp_encode);
					pclose(fp_encode);
				}
				regEnPw[strlen(regEnPw)] = '\0'; // result no new line
				Conf_Set_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "EnPassword", regEnPw);
				Conf_Remove_Field(SZP_RSYNC_SCHEDULE_CONF, pszSchedule, "Password"); // not really remove, just set null string
				// Decode to use
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "/sbin/get_encstr %s d 2>/dev/null", regEnPw);
				if (NULL != (fp_decode = popen(cmd, "r"))){
					fgets(szPswd, sizeof(szPswd), fp_decode);
					pclose(fp_decode);
				}
				szPswd[strlen(szPswd)] = '\0'; // result no new line
				pass = szPswd;
			}
		}
		else{ // "EnPassword" exists
			sprintf(cmd, "/sbin/get_encstr %s d 2>/dev/null", regEnPw);
			if (NULL != (fp_decode = popen(cmd, "r"))){
				int i=0;
				do {
					fgets(szPswd, sizeof(szPswd), fp_decode);
					if (i++ > 20) {
						rprintf(FLOG, "@@ %s:L%d fgets szPswd NULL over [%d]th\n",  __FUNCTION__, __LINE__, i);
						break;
					}
				} while (!feof(fp_decode));
				pclose(fp_decode);
			}
			szPswd[strlen(szPswd)] = '\0'; // result no new line
			pass = szPswd;
		}
	}
	else{
		pass = password;
	}
#endif
	if (!pass)
		pass = "";

	generate_hash(pass, challenge, pass2);
	io_printf(fd, "%s %s\n", user, pass2);
#ifdef QNAPNAS
	}
#endif
}

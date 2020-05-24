/*
 * utils/mountd/auth.c
 *
 * Authentication procedures for mountd.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "misc.h"
#include "nfslib.h"
#include "exportfs.h"
#include "mountd.h"
#include "xmalloc.h"

#ifdef _QNAP_NFS_
#include <unistd.h>
#include <sys/types.h>
#include <qLogEngine.h>
#include <naslog.h>
#include <NAS.h>
#define STATISTICS      1
#ifdef STATISTICS
#include <userManager.h>
#endif /* STATISTICS */
#include <debug.h>
#endif
enum auth_error
{
  bad_path,
  unknown_host,
  no_entry,
  not_exported,
  illegal_port,
  success
};

static void		auth_fixpath(char *path);
static char	*export_file = NULL;
static nfs_export my_exp;
static nfs_client my_client;

extern int new_cache;
//Richard Chang 20070911 add for onlineuser
#ifdef STATISTICS
void add_nfs_user(char *username, char *sharename, char *addr)
{
	USER_INFO user;
	
	strcpy(user.username, username);
	strcpy(user.sharename, sharename);
	strncpy(user.ip, addr, sizeof(user.ip));
	user.app_id = shm_get_app_id("nasnfsd");
	user.schedule = SCHEDULE_NO_TIMEOUT;
	user.pid = getpid();
	shm_add_user(&user);
}

void del_nfs_user(char *username, char *addr)
{
	shm_del_user(username, addr, shm_get_app_id("nasnfsd"));
}
#endif
extern int use_ipaddr;

void
auth_init(char *exports)
{

	export_file = exports;
	auth_reload();
	xtab_mount_write();
}

/*
 * A client can match many different netgroups and it's tough to know
 * beforehand whether it will. If the concatenated string of netgroup
 * m_hostnames is >512 bytes, then enable the "use_ipaddr" mode. This
 * makes mountd change how it matches a client ip address when a mount
 * request comes in. It's more efficient at handling netgroups at the
 * expense of larger kernel caches.
 */
static void
check_useipaddr(void)
{
	nfs_client *clp;
	int old_use_ipaddr = use_ipaddr;
	unsigned int len = 0;

	/* add length of m_hostname + 1 for the comma */
	for (clp = clientlist[MCL_NETGROUP]; clp; clp = clp->m_next)
		len += (strlen(clp->m_hostname) + 1);

	if (len > (NFSCLNT_IDMAX / 2))
		use_ipaddr = 1;
	else
		use_ipaddr = 0;

	if (use_ipaddr != old_use_ipaddr)
		cache_flush(1);
}

unsigned int
auth_reload()
{
	struct stat		stb;
	static ino_t		last_inode;
	static int		last_fd;
	static unsigned int	counter;
	int			fd;

	if ((fd = open(_PATH_ETAB, O_RDONLY)) < 0) {
		xlog(L_FATAL, "couldn't open %s", _PATH_ETAB);
	} else if (fstat(fd, &stb) < 0) {
		xlog(L_FATAL, "couldn't stat %s", _PATH_ETAB);
	} else if (stb.st_ino == last_inode) {
		close(fd);
		return counter;
	} else {
		close(last_fd);
		last_fd = fd;
		last_inode = stb.st_ino;
	}

	export_freeall();
	memset(&my_client, 0, sizeof(my_client));
	xtab_export_read();
	check_useipaddr();
	++counter;

	return counter;
}

static nfs_export *
auth_authenticate_internal(char *what, struct sockaddr_in *caller,
			   char *path, struct hostent *hp,
			   enum auth_error *error)
{
	nfs_export		*exp;

	if (new_cache) {
		int i;
		/* return static nfs_export with details filled in */
		char *n;
		free(my_client.m_hostname);
		if (use_ipaddr) {
			my_client.m_hostname =
				strdup(inet_ntoa(caller->sin_addr));
		} else {
			n = client_compose(hp);
			*error = unknown_host;
			if (!n)
				my_client.m_hostname = NULL;
			else if (*n)
				my_client.m_hostname = n;
			else {
				free(n);
				my_client.m_hostname = strdup("DEFAULT");
			}
		}
		if (my_client.m_hostname == NULL)
			return NULL;
		my_client.m_naddr = 1;
		my_client.m_addrlist[0] = caller->sin_addr;
		my_exp.m_client = &my_client;

		exp = NULL;
		for (i = 0; !exp && i < MCL_MAXTYPES; i++) 
			for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
				if (strcmp(path, exp->m_export.e_path))
					continue;
				if (!use_ipaddr && !client_member(my_client.m_hostname, exp->m_client->m_hostname))
					continue;
				if (use_ipaddr && !client_check(exp->m_client, hp))
					continue;
				break;
			}
		*error = not_exported;
		if (!exp)
			return exp;

		my_exp.m_export = exp->m_export;
		exp = &my_exp;

	} else {
		if (!(exp = export_find(hp, path))) {
			*error = no_entry;
			return NULL;
		}
		if (!exp->m_mayexport) {
			*error = not_exported;
			return NULL;
		}
	}
	if (!(exp->m_export.e_flags & NFSEXP_INSECURE_PORT) &&
		    (ntohs(caller->sin_port) <  IPPORT_RESERVED/2 ||
		     ntohs(caller->sin_port) >= IPPORT_RESERVED)) {
		*error = illegal_port;
		return NULL;
	}
	*error = success;

	return exp;
}

nfs_export *
auth_authenticate(char *what, struct sockaddr_in *caller, char *path)
{
	nfs_export	*exp = NULL;
	char		epath[MAXPATHLEN+1];
	char		*p = NULL;
	struct hostent	*hp = NULL;
	struct in_addr	addr = caller->sin_addr;
	enum auth_error	error = bad_path;

	if (path [0] != '/') {
		xlog(L_WARNING, "bad path in %s request from %s: \"%s\"",
		     what, inet_ntoa(addr), path);
		return exp;
	}

	strncpy(epath, path, sizeof (epath) - 1);
	epath[sizeof (epath) - 1] = '\0';
	auth_fixpath(epath); /* strip duplicate '/' etc */

	hp = client_resolve(caller->sin_addr);
	if (!hp)
		return exp;

	/* Try the longest matching exported pathname. */
	while (1) {
		exp = auth_authenticate_internal(what, caller, epath,
						 hp, &error);
		if (exp || (error != not_exported && error != no_entry))
			break;
		/* We have to treat the root, "/", specially. */
		if (p == &epath[1]) break;
		p = strrchr(epath, '/');
		if (p == epath) p++;
		*p = '\0';
	}

	switch (error) {
	case bad_path:
		xlog(L_WARNING, "bad path in %s request from %s: \"%s\"",
		     what, inet_ntoa(addr), path);
		break;

	case unknown_host:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): unmatched host",
		     what, inet_ntoa(addr), path, epath);
		break;

	case no_entry:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): no export entry",
		     what, hp->h_name, path, epath);
		break;

	case not_exported:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): not exported",
		     what, hp->h_name, path, epath);
		break;

	case illegal_port:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): illegal port %d",
		     what, hp->h_name, path, epath, ntohs(caller->sin_port));
		break;

	case success:
		xlog(L_NOTICE, "authenticated %s request from %s:%d for %s (%s)",
		     what, hp->h_name, ntohs(caller->sin_port), path, epath);
#ifdef _QNAP_NFS_
		if(!strcmp(what,"mount")){
/* KenChen@QNAP 2009/Mar/25
			if(path && (!strncmp(path, "/share", 6)))
				SendConnToLogEngine(EVENT_TYPE_INFO, "", hp->h_name, "---", CONN_SERV_NFS, CONN_ACTION_NFSMOUNT_SUCC, path+16);
			else if(path)
				SendConnToLogEngine(EVENT_TYPE_INFO, "", hp->h_name, "---", CONN_SERV_NFS, CONN_ACTION_NFSMOUNT_SUCC, path);
			else
				SendConnToLogEngine(EVENT_TYPE_INFO, "", hp->h_name, "---", CONN_SERV_NFS, CONN_ACTION_NFSMOUNT_SUCC, "--");
*/
	#ifdef STATISTICS
		if(hp->h_name)
			add_nfs_user("---", "---", hp->h_name);
	#endif
		}else if(!strcmp(what,"unmount")){
/* KenChen@QNAP 2009/Mar/25
			if(path && (!strncmp(path, "/share", 6)))
				SendConnToLogEngine(EVENT_TYPE_INFO, "", hp->h_name, "---", CONN_SERV_NFS, CONN_ACTION_NFSUMOUNT, path+16);
			else if(path)
				SendConnToLogEngine(EVENT_TYPE_INFO, "", hp->h_name, "---", CONN_SERV_NFS, CONN_ACTION_NFSUMOUNT, path);
			else
				SendConnToLogEngine(EVENT_TYPE_INFO, "", hp->h_name, "---", CONN_SERV_NFS, CONN_ACTION_NFSUMOUNT, "--");
*/
	#ifdef STATISTICS
		if(hp->h_name)
			del_nfs_user("---", hp->h_name);
	#endif
		}
		else{
	#ifdef STATISTICS
		if(hp->h_name)
			del_nfs_user("---", hp->h_name);
	#endif
		}
#endif

		break;
	default:
		xlog(L_NOTICE, "%s request from %s:%d for %s (%s) gave %d",
		     what, hp->h_name, ntohs(caller->sin_port), path, epath, error);
	}

	if (hp)
		free (hp);

	return exp;
}

static void
auth_fixpath(char *path)
{
	char	*sp, *cp;

	for (sp = cp = path; *sp; sp++) {
		if (*sp != '/' || sp[1] != '/')
			*cp++ = *sp;
	}
	while (cp > path+1 && cp[-1] == '/')
		cp--;
	*cp = '\0';
}

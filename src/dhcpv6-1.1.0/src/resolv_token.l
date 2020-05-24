/*
 * Copyright (C) International Business Machines  Corp., 2003
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 */

%option noyywrap
%option nounput

%{
#include "config.h"

#include <fcntl.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include "dhcp6.h"
#include "cfg.h"
#include "common.h"

#undef yywrap

#define YY_NO_UNPUT 1
#define ECHO if (fwrite(yytext, yyleng, 1, yyout) != 1) { \
                 YY_FATAL_ERROR("error performing ECHO on yytext"); \
             }

#define YYABORT(msg) dhcpv6_dprintf(LOG_ERR, msg " %s lineno %d.", \
	yytext, num_lines)

#define ABORT   do {    \
	YYABORT("resolv parse error");   \
	remove(RESOLV_CONF_DHCPV6_FILE); \
	exit(1);        \
} while (0)


extern struct dhcp6_if *dhcp6_if;
static int num_lines = 0;
static struct dns_list *new_dns_list;
static FILE *dhcp6_resolv_file;
static int oldfd, newfd;

static int yywrap __P(());
int resolv_parse __P((struct dns_list *));
static int update_resolver __P((struct dns_list *));

%}

digit           [0-9]
number          ({digit})+
hexdigit        ([a-f]|[A-F]|[0-9])
ipv4addr        ({digit}{1,3}"."{digit}{1,3}"."{digit}{1,3}"."{digit}{1,3})
addr_head       ("::"|{hexdigit}{1,4}(":"|"::"))
addr_tail       ({hexdigit}{1,4}|({hexdigit}{1,4}"::")|{ipv4addr})?
addr_body       ({hexdigit}{1,4}(":"|"::"))*
ipv6addr        {addr_head}{addr_body}{addr_tail}
whitespace      ([ \t])+
domainname	[a-zA-Z0-9:\._-]+
nl              \n


%s S_ADDR S_DOMAIN

%%

{nl}            {fprintf(dhcp6_resolv_file, yytext);
		num_lines++;}
{whitespace}    {fprintf(dhcp6_resolv_file, yytext);}

"nameserver"	{fprintf(dhcp6_resolv_file, yytext); 
		BEGIN S_ADDR;}
"search"	{fprintf(dhcp6_resolv_file, yytext); 
		BEGIN S_DOMAIN;}

<S_DOMAIN>{domainname} { struct domain_list *domainname, *temp;
		domainname = (struct domain_list *)malloc(sizeof(*domainname));
		if (domainname == NULL)
			ABORT;
		if (strlen(yytext) > MAXDNAME)
			ABORT;
		fprintf(dhcp6_resolv_file, yytext);
		strcpy(domainname->name, yytext);
		dhcpv6_dprintf(LOG_DEBUG, "parse domain name %s", domainname->name);
		domainname->next = NULL;
		if (dhcp6_if->dnslist.domainlist == NULL)
			dhcp6_if->dnslist.domainlist = domainname;
		else {
			for (temp = dhcp6_if->dnslist.domainlist; temp; temp = temp->next) {
				if (temp->next == NULL) {
					temp->next = domainname;
					break;
				}
			}
		}
		BEGIN S_DOMAIN;}

<S_DOMAIN>. {fprintf(dhcp6_resolv_file, yytext); BEGIN INITIAL;}

<S_ADDR>{ipv6addr} {struct in6_addr addr;
		fprintf(dhcp6_resolv_file, yytext);
		if (inet_pton(AF_INET6, yytext, &addr) < 1) 
			ABORT;
		dhcpv6_dprintf(LOG_DEBUG, "parse name server %s", in6addr2str(&addr, 0));
		if (dhcp6_add_listval(&dhcp6_if->dnslist.addrlist,
		    &addr, DHCP6_LISTVAL_ADDR6) == NULL) {
			dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a DNS server", FNAME);
			ABORT;
		}
		BEGIN S_ADDR;}
<S_ADDR>. {fprintf(dhcp6_resolv_file, yytext); BEGIN INITIAL;}
.	{fprintf(dhcp6_resolv_file, yytext);}

%%
int change_resolv_conf
(   char *new_rscf
)
{
    char *argv[] = { "/bin/bash", "-c", 0L, 0L };
    char *cmd;
    int l, pid, status;
    extern char **environ;

    if( new_rscf == 0L )
	return -1;
    
    if( getuid() != 0 )
	return 1;
    
    l = strlen(NETWORK_FUNCTIONS_SCRIPT) + strlen(new_rscf) + 27;
    cmd = (char*)malloc(l);
    snprintf(cmd, l, ". %s ; change_resolv_conf %s",
		    NETWORK_FUNCTIONS_SCRIPT, new_rscf);
    argv[2] = cmd;
    
    if( (pid = fork()) < 0 )
	return -1;
    else
    if( pid > 0 )
    {
	status = 0;
	if(waitpid(pid, &status, 0L) == pid)
	    return(   ((status & 0x7f)==0) 
		    ? ((status >> 8) & 0xff)
		    : -(status & 0x7f)
		  );
    }else
	execve( argv[0], argv, environ );

    return -1;
}

int
backup_resolv_conf()
{
	struct stat buf;
	mode_t st_mode;
	char str[BUFSIZ];
	size_t rlen, wlen;

	/* Do not overwrite if a backup file already exists */
	if (!lstat(RESOLV_CONF_BAK_FILE, &buf)) {
		dhcpv6_dprintf(LOG_DEBUG, "%s"
			       "backup file for resolv.conf exists", FNAME);
		return 0;
	}

	if (lstat(RESOLV_CONF_FILE, &buf) < 0) {
		dhcpv6_dprintf(LOG_ERR, "%s" "Failed to stat %s : %s",
			       FNAME, RESOLV_CONF_FILE, strerror(errno));
	}
	st_mode = buf.st_mode;

	if ((oldfd = open(RESOLV_CONF_FILE, O_RDONLY)) < 0) {
		dhcpv6_dprintf(LOG_ERR, "%s" "Failed to open %s : %s",
			       FNAME, RESOLV_CONF_FILE, strerror(errno));
		return -1;
	}

	if ((newfd = open(resolv_dhcpv6_file,
			  O_WRONLY | O_CREAT | O_EXCL, st_mode)) < 0) {
		dhcpv6_dprintf(LOG_ERR, "%s" "Failed to open %s : %s",
			       FNAME, resolv_dhcpv6_file, strerror(errno));
		close(oldfd);
		return -1;
	}

	while(1) {
		rlen = read(oldfd, str, sizeof(str));
		if (rlen < 0) {
			dhcpv6_dprintf(LOG_ERR, "%s" "Failed to read %s :%s ",
				       FNAME, RESOLV_CONF_FILE,
				       strerror(errno));
			close(oldfd);
			close(newfd);
			return -1;
		} else if (rlen == 0) {
			break;
		}

		wlen = write(newfd, str, rlen);
		if (wlen < rlen) {
			dhcpv6_dprintf(LOG_ERR, "%s" "Failed to write %s :%s ",
				       FNAME, resolv_dhcpv6_file,
				       strerror(errno));
			close(oldfd);
			close(newfd);
			return -1;
		}
	}

	close(oldfd);
	close(newfd);

	if (rename(RESOLV_CONF_FILE, RESOLV_CONF_BAK_FILE)) {
		dhcpv6_dprintf(LOG_ERR, "%s" "Failed to rename %s : %s",
			       FNAME, RESOLV_CONF_FILE, strerror(errno));
		return -1;
	}

	if (rename(resolv_dhcpv6_file, RESOLV_CONF_FILE)) {
		dhcpv6_dprintf(LOG_ERR, "%s" "Failed to rename %s : %s",
			       FNAME, resolv_dhcpv6_file, strerror(errno));
		/* Try to take back original resolv.conf */
		rename(RESOLV_CONF_BAK_FILE, RESOLV_CONF_FILE);
		return -1;
	}

	return 0;
}

/* parse resolv.conf
 * create a new resolv.conf.dhcpv6
 * mv /etc/resolv.conf to /etc/resolv.conf.V6BAK
 * link resolv.conf to resolv.conf.dhcpv6
 */
int
resolv_parse(struct dns_list *dnslist)
{
	struct stat buf;
	char pidstr[128];
	sprintf(pidstr, "%d", getpid());
	strcpy(resolv_dhcpv6_file, RESOLV_CONF_DHCPV6_FILE);
	strcat(resolv_dhcpv6_file, pidstr);
	dhcp6_clear_list(&dhcp6_if->dnslist.addrlist);
	TAILQ_INIT(&dhcp6_if->dnslist.addrlist);
	dhcp6_if->dnslist.domainlist = NULL;
	new_dns_list = dnslist;

	/* Back up resolv.conf */
	if (!lstat(RESOLV_CONF_FILE, &buf)) {
		if (backup_resolv_conf() < 0) {
			dhcpv6_dprintf(LOG_ERR, "%s"
				" backup failed for resolv.conf file", FNAME);
			return -1;
		}
	}

	while ((oldfd = open(RESOLV_CONF_BAK_FILE, O_EXCL)) < 0) {
		switch (errno) {
		case ENOENT:
			/* exclusive open of new resolv.conf.dhcpv6 file */
			if ((newfd = open(resolv_dhcpv6_file, 
			    O_CREAT|O_EXCL|O_WRONLY|O_TRUNC, 0644))
			    < 0) {
				dhcpv6_dprintf(LOG_ERR, "%s" 
					" failed to open %s file", FNAME, resolv_dhcpv6_file);
				return (-1);
			}
			if ((dhcp6_resolv_file = fdopen(newfd, "w")) == NULL) {
				dhcpv6_dprintf(LOG_ERR, "%s" 
					" failed to fdopen %s file", FNAME, resolv_dhcpv6_file);
				return (-1);
			}
			return(update_resolver(dnslist));
		case EACCES: 
			sleep(1);
			continue;
		default:
			dhcpv6_dprintf(LOG_ERR, "resolv_parse: fopen(%s): %s",
				RESOLV_CONF_BAK_FILE, strerror(errno));
			return (-1);
		}
	}
	if (lstat(RESOLV_CONF_BAK_FILE, &buf)) {
		dhcpv6_dprintf(LOG_ERR, "lstat %s file failed",
			RESOLV_CONF_BAK_FILE);
		return (-1);
	}
	if ((yyin = fdopen(oldfd, "r")) == NULL) {
		dhcpv6_dprintf(LOG_ERR, "resolv_parse: fdopen(%s): %s",
			RESOLV_CONF_BAK_FILE, strerror(errno));
		return (-1);
	}
	/* exclusive open of new resolv.conf.dhcpv6 file */
	if ((newfd = open(resolv_dhcpv6_file, 
	    O_CREAT|O_EXCL|O_WRONLY|O_TRUNC, buf.st_mode)) < 0) {
		dhcpv6_dprintf(LOG_ERR, "%s" 
			" failed to open resolv.conf.dhcpv6 file", FNAME);
		return (-1);
	}
	if ((dhcp6_resolv_file = fdopen(newfd, "w")) == NULL) {
		dhcpv6_dprintf(LOG_ERR, "%s" 
			" failed to fdopen resolv.conf.dhcpv6 file", FNAME);
		return (-1);
	}
	return yylex();
}

static int
update_resolver(struct dns_list *dns_list)
{
	struct domain_list *oldlist, *dprev, *dlist, *dlist_next;
	struct dhcp6_listval *d, *d_next;
	struct stat buf;
	int i = 0;
	int ret = 0;

	fseek(dhcp6_resolv_file, 0, SEEK_CUR);
	if (!TAILQ_EMPTY(&dns_list->addrlist)) {
		for (d = TAILQ_FIRST(&dns_list->addrlist); d; d = d_next, i++) {
			struct dhcp6_listval *lv;
			dhcpv6_dprintf(LOG_DEBUG, "%s" " received nameserver[%d] %s",
				FNAME, i, in6addr2str(&d->val_addr6, 0));
			d_next = TAILQ_NEXT(d, link);
			for (lv = TAILQ_FIRST(&dhcp6_if->dnslist.addrlist); lv;
			     lv = TAILQ_NEXT(lv, link)) {
				if (IN6_ARE_ADDR_EQUAL(&lv->val_addr6, &d->val_addr6)) { 
					dhcpv6_dprintf(LOG_DEBUG, "%s" 
						"nameserver %s found in resolv.conf",
						FNAME, in6addr2str(&d->val_addr6, 0));
					TAILQ_REMOVE(&dns_list->addrlist, d, link);
					continue;
				}
			}
		}
		if (!TAILQ_EMPTY(&dns_list->addrlist)) {
			for (d = TAILQ_FIRST(&dns_list->addrlist); d;
				d = d_next, i++) {
				d_next = TAILQ_NEXT(d, link);
				fprintf(dhcp6_resolv_file, "nameserver %s\n", 
					in6addr2str(&d->val_addr6, 0));
			}
		}
	}
			
	if (dns_list->domainlist != NULL) {
		i = 0;
		for (dlist = dns_list->domainlist, dprev = dns_list->domainlist;
		     dlist; i++, dlist = dlist_next) {
			int found = 0;
			dhcpv6_dprintf(LOG_DEBUG, "%s" " received domainname[%d] %s",
				FNAME, i, dlist->name);
			if (dhcp6_if->dnslist.domainlist == NULL) 
				break;
			for (oldlist = dhcp6_if->dnslist.domainlist; oldlist; 
			     oldlist = oldlist->next) {
				if (strcasecmp(oldlist->name, dlist->name) == 0) {
					found = 1;
					dhcpv6_dprintf(LOG_DEBUG, "%s" "domain name %s found in " 
						"resolv.conf", FNAME, dlist->name);
					if (dprev == dlist) {
						dprev = dlist->next;
						dns_list->domainlist = dlist->next;
					} else
						dprev->next = dlist->next;
					break;
				}
			}
			dlist_next = dlist->next;
			if (found == 0) 
				dprev = dlist;
			else {
				free(dlist);
			}
		}
		if (dns_list->domainlist != NULL) {
			fprintf(dhcp6_resolv_file, "search ");
			for (dlist = dns_list->domainlist; dlist; dlist = dlist->next) {
				dhcpv6_dprintf(LOG_DEBUG, "%s" "domain name %s added in resolv.conf", 
					FNAME, dlist->name);
				fprintf(dhcp6_resolv_file, "%s", dlist->name);
				if (dlist->next != NULL)
					fprintf(dhcp6_resolv_file, " ");
				else
					fprintf(dhcp6_resolv_file, "\n");
			}
		}
	}
	if (fflush(dhcp6_resolv_file) == EOF) {
		dhcpv6_dprintf(LOG_ERR, "%s" "write resolv.conf.dhcpv6 file fflush failed %s", 
			FNAME, strerror(errno));
		ret = -1;
		goto out;
	}
	if (fsync(newfd) < 0) {
		dhcpv6_dprintf(LOG_ERR, "%s" "write resolv.conf.dhcpv6 file failed %s", 
			FNAME, strerror(errno));
		ret = -1;
		goto out;
	}
	fclose(dhcp6_resolv_file);
	close(newfd);

	if (!lstat(NETWORK_FUNCTIONS_SCRIPT, &buf)) {
		if (change_resolv_conf(resolv_dhcpv6_file)!=0) {
//			dhcpv6_dprintf(LOG_ERR, "%s" " rename failed for resolv.conf file", FNAME);
//			needs initscripts patch!		return(-1);
		}
	} else {
		if (rename(resolv_dhcpv6_file, RESOLV_CONF_FILE)) {
			dhcpv6_dprintf(LOG_ERR, "%s" " rename failed for resolv.conf file", FNAME);
			ret = -1;
			goto out;
		}
        }

out:
	if (!lstat(resolv_dhcpv6_file, &buf))
		unlink(resolv_dhcpv6_file);
	return ret;
}

static int 
yywrap()
{
        update_resolver(new_dns_list);
	fclose(yyin);
	close(oldfd);
	return 1;
}

/* ported from KAME: cftoken.l,v 1.15 2002/09/24 14:20:49 itojun Exp */

/*
 * Copyright (C) 2002 WIDE Project.
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
 * SUCH DAMAGE.
 */

%option noyywrap
%option nounput

%{
#include "config.h"

#include <stdarg.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "dhcp6.h"
#include "cfg.h"
#include "common.h"
#include "client6_parse.h"

const char *configfilename;
int lineno = 1;

static int cpyy_first_time = 1;
static int client6errorcount = 0;

#define YY_NO_UNPUT 1
#define ECHO if (fwrite(yytext, yyleng, 1, yyout) != 1) { \
                 YY_FATAL_ERROR("error performing ECHO on yytext"); \
             }

#ifndef NOCONFIG_DEBUG
#define YYDEBUG 1

int cfdebug = 1;
#else
int cfdebug = 0;
#endif

static void cfdebug_print __P((char *, char *, int));
extern int cf_post_config __P((void));

extern int client6parse(void);
extern int cf_post_config(void);

#define DP(str) if (cfdebug) cfdebug_print(str, client6text, client6leng)
#define DECHO if (cfdebug) cfdebug_print(NULL, client6text, client6leng);
%}

/* abbreviations */
nl		\n
ws		[ \t]+
comma		,
comment		\#.*
semi		\;
string		[a-zA-Z0-9:\._-][a-zA-Z0-9:\._-]*
digit		[0-9]
number	 	{digit}+
hexdigit	[0-9A-Fa-f]
hexpair		{hexdigit}{hexdigit}
hexstring	0[xX]{hexpair}+
duid 		{hexpair}(:{hexpair})*
ipv4addr        ({digit}{1,3}"."{digit}{1,3}"."{digit}{1,3}"."{digit}{1,3})
addr_head       ("::"|{hexdigit}{1,4}(":"|"::"))
addr_tail       ({hexdigit}{1,4}|({hexdigit}{1,4}"::")|{ipv4addr})
addr_body       ({hexdigit}{1,4}(":"|"::"))*
ipv6addr        {addr_head}{addr_body}{addr_tail}

ifname		({string})+
slash		\/
bcl		\{
ecl		\}

%s S_CNF
%s S_IFACE
%s S_ADDR
%s S_RNT
%s S_RBT

%%
%{
	if (cpyy_first_time) {
		BEGIN S_CNF;
		cpyy_first_time = 0;
	}
%}
	/* interface configuration */
<S_CNF>interface { DECHO; BEGIN S_IFACE; return (INTERFACE); }
<S_IFACE>{ifname} {
	DECHO;
	client6lval.str = strdup(client6text);
	BEGIN S_CNF;
	return (IFNAME);
}

<S_CNF>address { DECHO; BEGIN S_ADDR; return (ADDRESS); }

<S_CNF>prefix { DECHO; BEGIN S_ADDR; return (PREFIX); }
<S_ADDR>{ipv6addr} {
	struct in6_addr addr;
	DECHO;
	if (inet_pton(AF_INET6, client6text, &addr) < 1) {
		dhcpv6_dprintf(LOG_ERR, "invalid address in line %d", lineno);
		return (-1);
	}
	client6lval.addr = addr;
	BEGIN S_CNF;
	return (IPV6ADDR);
}
<S_CNF>iaid { DECHO; return (IAID); }
<S_CNF>valid-life-time { DECHO; return (V_TIME); }
<S_CNF>prefer-life-time { DECHO; return (P_TIME); }
<S_CNF>renew-time { DECHO; return (RENEW_TIME); }
<S_CNF>rebind-time { DECHO; return (REBIND_TIME); }
<S_CNF>prefix-delegation-interface { DECHO; return (PREFIX_DELEGATION_INTERFACE); }
	/* request */
<S_CNF>request { DECHO; return (REQUEST); }

	/* send */
<S_CNF>send { DECHO; return (SEND); }
	/* DHCP options */
<S_CNF>option { DECHO; return (OPTION); }

<S_CNF>rapid-commit { DECHO; return (RAPID_COMMIT); }
<S_CNF>prefix-delegation { DECHO; return (PREFIX_DELEGATION); }
<S_CNF>domain-name-servers { DECHO; return (DNS_SERVERS); }
<S_CNF>domain-search-list { DECHO; return (DOMAIN_LIST); }
	/* generic options */
<S_CNF>information-only { DECHO; return (INFO_ONLY); }
<S_CNF>default-information-refresh-time { DECHO; return (DEFAULT_IRT); }
<S_CNF>maximum-information-refresh-time { DECHO; return (MAXIMUM_IRT); }
<S_CNF>temp-address { DECHO; return (TEMP_ADDR); }
	/* duration */
<S_CNF>infinity { DECHO; return (INFINITY); }

	/* misc */
{ws}		{ ; }
{nl}		{ lineno++; }
{comment}	{ DP("comment"); }
{number} 	{
	DECHO;
	client6lval.num = strtoll(client6text, NULL, 10);
	return (NUMBER);
}
{slash} { DECHO; return (SLASH); }
{comma} { DECHO; return (COMMA); }
{semi} { DP("end of sentence"); return (EOS); }
{bcl} { DP("begin of closure"); return (BCL); }
{ecl} { DP("end of closure"); return (ECL); }

	/* generic string */
{string} {
		DECHO;
		client6lval.str = strdup(client6text);
		return (STRING);
	}

%%
static void
cfdebug_print(w, t, l)
	char *w, *t;
	int l;
{
	if (w)
		dhcpv6_dprintf(LOG_DEBUG, "<%d>%s [%s] (%d)", yy_start, w, t, l);
	else
		dhcpv6_dprintf(LOG_DEBUG, "<%d>[%s] (%d)", yy_start, t, l);
}

static void
client6error0(int level, char *s, va_list ap)
{
	char ebuf[BUFSIZ], *bp, *ep;

	bp = ebuf;
	ep = ebuf + sizeof(ebuf);
	bp += snprintf(bp, ep - bp, "%s %d: ", configfilename, lineno);
	if (bp < ep)
		bp += vsnprintf(bp, ep - bp, s, ap);

	dhcpv6_dprintf(level, ebuf);
}

void
client6error(char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	client6error0(LOG_ERR, s, ap);
	va_end(ap);
	client6errorcount++;
}

void
cpyywarn(char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	client6error0(LOG_WARNING, s, ap);
	va_end(ap);
}

int
cfparse(const char *conf)
{
	configfilename = conf;
	if ((client6in = fopen(configfilename, "r")) == NULL) {
		if (errno == ENOENT)
			return (0);
		dhcpv6_dprintf(LOG_ERR, "cfparse: fopen(%s): %s",
			configfilename, strerror(errno));
		return (-1);
	}

	if (client6parse() || client6errorcount) {
		if (client6errorcount) {
			client6error("fatal parse failure: exiting (%d errors)",
				client6errorcount);
		} else
			client6error("fatal parse failure: exiting");
		return (-1);
	}

	return (cf_post_config());
}

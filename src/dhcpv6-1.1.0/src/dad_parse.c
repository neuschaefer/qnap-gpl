/*
 * dad_parse.c
 * Reads current interface state from /proc/net/if_inet6 and does things.
 *
 * Copyright (C) 2008  Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author(s): David Cantrell <dcantrell@redhat.com>
 */


/*
 * (Adapted from dad_token.l, which is BSD licensed code from IBM.)
 *
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

#include "config.h"

#include <stdlib.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>

#include "dhcp6.h"
#include "cfg.h"
#include "common.h"
#include "lease.h"

extern struct dhcp6_if *dhcp6_if;

#define DAD_FLAGS 0xC0

struct ifproc_info {
    struct ifproc_info *next;
    struct in6_addr addr;
    char name[IF_NAMESIZE];
    int index;
    int plen;
    int scope;
    int flags;
};

int dad_parse(const char *file, struct dhcp6_list *dad_list) {
    int i = 0;
    int len = 0;
    int ret = 0;
    FILE *fp = NULL;
    char buf[55];               /* max line length in /proc/net/if_inet6 */
    char addrbuf[64];
    char *tmp = NULL;
    struct in6_addr addr6;
    struct ifproc_info *ifinfo = NULL;

    if (file == NULL) {
        dhcpv6_dprintf(LOG_ERR, "dad_parse: NULL filename");
        return -1;
    }

    memset(&buf, '\0', sizeof(buf));
    memset(&addr6, 0, sizeof(addr6));

    if ((fp = fopen(file, "r")) == NULL) {
        if (errno == ENOENT) {
            return 0;
        }

        dhcpv6_dprintf(LOG_ERR, "dad_parse: fopen(%s): %s", file,
                       strerror(errno));
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        /* read address */
        if ((tmp = strtok(buf, " \n")) == NULL) {
            continue;
        }

        len = 0;

        for (i = 0; i < 32; i += 4) {
            strncpy(addrbuf + len, &tmp[i], 4);
            len += 4;

            if (i < 28) {
                strcpy(addrbuf + len, ":");
                len += 1;
            } else {
                strcpy(addrbuf + len, "\0");
            }
        }

        if (inet_pton(AF_INET6, addrbuf, &addr6) < 1) {
            dhcpv6_dprintf(LOG_ERR, "failed to parse %s from %s",
                           addrbuf, file);
            abort();
        }

        if ((ifinfo = malloc(sizeof(*ifinfo))) == NULL) {
            dhcpv6_dprintf(LOG_ERR, "memory allocation failure");
            abort();
        }

        memcpy(&ifinfo->addr, &addr6, sizeof(ifinfo->addr));

        /* read the index */
        if ((tmp = strtok(NULL, " \n")) == NULL) {
            continue;
        }

        ifinfo->index = strtol(tmp, NULL, 16);
        if ((errno == EINVAL) || (errno == ERANGE)) {
            dhcpv6_dprintf(LOG_ERR, "error reading index from %s", file);
            goto fail;
        }

        /* read the prefix length */
        if ((tmp = strtok(NULL, " \n")) == NULL) {
            continue;
        }

        ifinfo->plen = strtol(tmp, NULL, 16);
        if ((errno == EINVAL) || (errno == ERANGE)) {
            dhcpv6_dprintf(LOG_ERR, "error reading prefix length from %s", file);
            goto fail;
        }

        /* read the scope */
        if ((tmp = strtok(NULL, " \n")) == NULL) {
            continue;
        }

        ifinfo->scope = strtol(tmp, NULL, 16);
        if ((errno == EINVAL) || (errno == ERANGE)) {
            dhcpv6_dprintf(LOG_ERR, "error reading scope from %s", file);
            goto fail;
        }

        /* read the flags */
        if ((tmp = strtok(NULL, " \n")) == NULL) {
            continue;
        }

        ifinfo->flags = strtol(tmp, NULL, 16);
        if ((errno == EINVAL) || (errno == ERANGE)) {
            dhcpv6_dprintf(LOG_ERR, "error reading flags from %s", file);
            goto fail;
        }

        if (ifinfo->flags == DAD_FLAGS) {
            dhcpv6_dprintf(LOG_INFO, "duplicated IPv6 address %s detected",
                           in6addr2str(&ifinfo->addr, 0));
        } else {
            free(ifinfo);
            ifinfo = NULL;
            continue;
        }

        /* read the interface name */
        if ((tmp = strtok(NULL, " \n")) == NULL) {
            continue;
        }

        if (strcmp(tmp, dhcp6_if->ifname)) {
            free(ifinfo);
            ifinfo = NULL;
            continue;
        } else {
            struct dhcp6_listval *lv;

            strncpy(ifinfo->name, tmp, IF_NAMESIZE);
            ifinfo->next = NULL;

            /* check address on client6_iaidaddr list */
            if ((lv = malloc(sizeof(*lv))) == NULL) {
                dhcpv6_dprintf(LOG_ERR, "memory allocation failure");
                abort();
            }

            memcpy(&lv->val_dhcp6addr.addr, &ifinfo->addr,
                   sizeof(lv->val_dhcp6addr.addr));
            lv->val_dhcp6addr.type = IANA;
            lv->val_dhcp6addr.plen = ifinfo->plen;
            lv->val_dhcp6addr.status_code = DH6OPT_STCODE_UNDEFINE;
            lv->val_dhcp6addr.preferlifetime = 0;
            lv->val_dhcp6addr.validlifetime = 0;
            TAILQ_INSERT_TAIL(dad_list, lv, link);
        }
    }

out:
    if (fclose(fp) == EOF) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        abort();
    }

    return ret;

fail:
    dhcp6_clear_list(dad_list);
    ret = -1;
    goto out;
}

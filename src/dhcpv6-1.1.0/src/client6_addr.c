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
 * SUCH DAMAGE.
 */

/* Author: Shirley Ma, xma@us.ibm.com */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# include <time.h>
#endif

#ifdef HAVE_LINUX_SOCKIOS_H
# include <linux/sockios.h>
#endif

#ifndef HAVE_STRUCT_IN6_IFREQ_IFR6_ADDR
#ifdef HAVE_LINUX_IPV6_H
# include <linux/ipv6.h>
#endif
#endif

#ifdef HAVE_NET_IF_VAR_H
# include <net/if_var.h>
#endif

#ifdef HAVE_NETINET6_IN6_VAR_H
# include <netinet6/in6_var.h>
#endif

#include "dhcp6.h"
#include "cfg.h"
#include "common.h"
#include "timer.h"
#include "lease.h"

#ifdef LIBDHCP
#include "libdhcp_control.h"
#endif

static int dhcp6_update_lease
__P((struct dhcp6_addr *, struct dhcp6_lease *));
static int dhcp6_add_lease __P((struct dhcp6_addr *));
struct dhcp6_lease *dhcp6_find_lease __P((struct dhcp6_iaidaddr *,
                                          struct dhcp6_addr *));
int dhcp6_get_prefixlen __P((struct in6_addr *, struct dhcp6_if *));
int client6_ifaddrconf __P((ifaddrconf_cmd_t, struct dhcp6_addr *));
u_int32_t get_min_preferlifetime __P((struct dhcp6_iaidaddr *));
u_int32_t get_max_validlifetime __P((struct dhcp6_iaidaddr *));
struct dhcp6_timer *dhcp6_iaidaddr_timo __P((void *));
struct dhcp6_timer *dhcp6_lease_timo __P((void *));

extern struct dhcp6_iaidaddr client6_iaidaddr;
extern struct dhcp6_timer *client6_timo __P((void *));
extern void client6_send __P((struct dhcp6_event *));
extern void free_servers __P((struct dhcp6_if *));
extern ssize_t gethwid __P((unsigned char *, int, const char *, u_int16_t *));

extern int nlsock;
extern FILE *client6_lease_file;
extern struct dhcp6_iaidaddr client6_iaidaddr;
extern struct dhcp6_list request_list;

void dhcp6_init_iaidaddr(void) {
    memset(&client6_iaidaddr, 0, sizeof(client6_iaidaddr));
    TAILQ_INIT(&client6_iaidaddr.lease_list);
}

int dhcp6_add_iaidaddr(struct dhcp6_optinfo *optinfo, struct ia_listval *ia) {
    struct dhcp6_listval *lv, *lv_next = NULL;
    struct timeval timo;
    struct dhcp6_lease *cl_lease;
    double d;

    /* ignore IA with T1 > T2 */
    if (ia->iaidinfo.renewtime > ia->iaidinfo.rebindtime) {
        dhcpv6_dprintf(LOG_INFO, " T1 time is greater than T2 time");
        return (0);
    }

    memcpy(&client6_iaidaddr.client6_info.iaidinfo, &ia->iaidinfo,
           sizeof(client6_iaidaddr.client6_info.iaidinfo));
    client6_iaidaddr.client6_info.type = ia->type;
    duidcpy(&client6_iaidaddr.client6_info.clientid, &optinfo->clientID);

    if (duidcpy(&client6_iaidaddr.client6_info.serverid, &optinfo->serverID)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy server ID %s",
                       FNAME, duidstr(&optinfo->serverID));
        return (-1);
    }

    /* add new address */
    for (lv = TAILQ_FIRST(&ia->addr_list); lv; lv = lv_next) {
        lv_next = TAILQ_NEXT(lv, link);

        if (lv->val_dhcp6addr.type != IAPD) {
            lv->val_dhcp6addr.plen =
                dhcp6_get_prefixlen(&lv->val_dhcp6addr.addr, dhcp6_if);

            if (lv->val_dhcp6addr.plen == PREFIX_LEN_NOTINRA) {
                dhcpv6_dprintf(LOG_WARNING,
                               "assigned address %s prefix len is not in any RAs"
                               " prefix length using 64 bit instead",
                               in6addr2str(&lv->val_dhcp6addr.addr, 0));
            }
        }

        if ((cl_lease = dhcp6_find_lease(&client6_iaidaddr,
                                         &lv->val_dhcp6addr)) != NULL) {
            dhcp6_update_lease(&lv->val_dhcp6addr, cl_lease);
            continue;
        }

        if (dhcp6_add_lease(&lv->val_dhcp6addr)) {
            dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a new addr lease %s",
                           FNAME, in6addr2str(&lv->val_dhcp6addr.addr, 0));
            continue;
        }
    }

    if (TAILQ_EMPTY(&client6_iaidaddr.lease_list)) {
        return 0;
    }

    /* set up renew T1, rebind T2 timer renew/rebind based on iaid */
    /* Should we process IA_TA, IA_NA differently */
    if (client6_iaidaddr.client6_info.iaidinfo.renewtime == 0 ||
        client6_iaidaddr.client6_info.iaidinfo.renewtime >
        client6_iaidaddr.client6_info.iaidinfo.rebindtime) {
        u_int32_t min_plifetime;
        min_plifetime = get_min_preferlifetime(&client6_iaidaddr);

        if (min_plifetime == DHCP6_DURATITION_INFINITE) {
            client6_iaidaddr.client6_info.iaidinfo.renewtime = min_plifetime;
        } else {
            client6_iaidaddr.client6_info.iaidinfo.renewtime =
                min_plifetime / 2;
        }
    }

    if (client6_iaidaddr.client6_info.iaidinfo.rebindtime == 0 ||
        client6_iaidaddr.client6_info.iaidinfo.renewtime >
        client6_iaidaddr.client6_info.iaidinfo.rebindtime) {
        client6_iaidaddr.client6_info.iaidinfo.rebindtime =
            get_min_preferlifetime(&client6_iaidaddr) * 4 / 5;
    }

    dhcpv6_dprintf(LOG_INFO, "renew time %d, rebind time %d",
                   client6_iaidaddr.client6_info.iaidinfo.renewtime,
                   client6_iaidaddr.client6_info.iaidinfo.rebindtime);

    if (client6_iaidaddr.client6_info.iaidinfo.renewtime == 0) {
        return (0);
    }

    if (client6_iaidaddr.client6_info.iaidinfo.renewtime ==
        DHCP6_DURATITION_INFINITE) {
        client6_iaidaddr.client6_info.iaidinfo.rebindtime =
            DHCP6_DURATITION_INFINITE;
        return (0);
    }

    /* set up start date, and renew timer */
    if ((client6_iaidaddr.timer =
         dhcp6_add_timer(dhcp6_iaidaddr_timo, &client6_iaidaddr)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a timer for iaid %u",
                       FNAME, client6_iaidaddr.client6_info.iaidinfo.iaid);
        return (-1);
    }

    time(&client6_iaidaddr.start_date);
    client6_iaidaddr.state = ACTIVE;
    d = client6_iaidaddr.client6_info.iaidinfo.renewtime;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcp6_set_timer(&timo, client6_iaidaddr.timer);

    return 0;
}

int dhcp6_add_lease(addr)
     struct dhcp6_addr *addr;
{
    struct dhcp6_lease *sp;
    struct timeval timo;
    double d;

    dhcpv6_dprintf(LOG_DEBUG, "%s" "try to add address %s", FNAME,
                   in6addr2str(&addr->addr, 0));

    /* ignore meaningless address */
    if (addr->status_code != DH6OPT_STCODE_SUCCESS &&
        addr->status_code != DH6OPT_STCODE_UNDEFINE) {
        dhcpv6_dprintf(LOG_ERR,
                       "%s" "not successful status code for %s is %s", FNAME,
                       in6addr2str(&addr->addr, 0),
                       dhcp6_stcodestr(addr->status_code));
        return (0);
    }
    if (addr->validlifetime == 0 || addr->preferlifetime == 0 ||
        addr->preferlifetime > addr->validlifetime) {
        dhcpv6_dprintf(LOG_ERR, "%s" "invalid address life time for %s",
                       FNAME, in6addr2str(&addr->addr, 0));
        return (0);
    }
    if ((sp = dhcp6_find_lease(&client6_iaidaddr, addr)) != NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "duplicated address: %s",
                       FNAME, in6addr2str(&addr->addr, 0));
        return (-1);
    }
    if ((sp = (struct dhcp6_lease *) malloc(sizeof(*sp))) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to allocate memory"
                       " for a addr", FNAME);
        return (-1);
    }
    memset(sp, 0, sizeof(*sp));
    memcpy(&sp->lease_addr, addr, sizeof(sp->lease_addr));
    sp->iaidaddr = &client6_iaidaddr;
    time(&sp->start_date);
    sp->state = ACTIVE;
    if (client6_lease_file && (write_lease(sp, client6_lease_file) != 0)) {
        dhcpv6_dprintf(LOG_ERR,
                       "%s"
                       "failed to write a new lease address %s to lease file",
                       FNAME, in6addr2str(&sp->lease_addr.addr, 0));
        if (sp->timer)
            dhcp6_remove_timer(sp->timer);
        free(sp);
        return (-1);
    }
    if (sp->lease_addr.type == IAPD) {
        dhcpv6_dprintf(LOG_INFO, "request prefix is %s/%d",
                       in6addr2str(&sp->lease_addr.addr, 0),
                       sp->lease_addr.plen);
#ifdef LIBDHCP
    } else if (libdhcp_control
               && (libdhcp_control->capability & DHCP_CONFIGURE_ADDRESSES)) {
#else
    } else
#endif
    if (client6_ifaddrconf(IFADDRCONF_ADD, addr) != 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "adding address failed: %s",
                       FNAME, in6addr2str(&addr->addr, 0));
        if (sp->timer)
            dhcp6_remove_timer(sp->timer);
        free(sp);
        return (-1);
    }
#ifdef LIBDHCP
}
#endif
TAILQ_INSERT_TAIL(&client6_iaidaddr.lease_list, sp, link);
        /* for infinite lifetime don't do any timer */
if (sp->lease_addr.validlifetime == DHCP6_DURATITION_INFINITE ||
    sp->lease_addr.preferlifetime == DHCP6_DURATITION_INFINITE) {
    dhcpv6_dprintf(LOG_INFO, "%s" "infinity address life time for %s",
                   FNAME, in6addr2str(&addr->addr, 0));
    return (0);
}
        /* set up expired timer for lease */
if ((sp->timer = dhcp6_add_timer(dhcp6_lease_timo, sp)) == NULL) {
    dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a timer for lease %s",
                   FNAME, in6addr2str(&addr->addr, 0));
    free(sp);
    return (-1);
}
d = sp->lease_addr.preferlifetime;
timo.tv_sec = (long) d;
timo.tv_usec = 0;
dhcp6_set_timer(&timo, sp->timer);
return 0;
}

int dhcp6_remove_iaidaddr(struct dhcp6_iaidaddr *iaidaddr) {
    struct dhcp6_lease *lv, *lv_next;

    for (lv = TAILQ_FIRST(&iaidaddr->lease_list); lv; lv = lv_next) {
        lv_next = TAILQ_NEXT(lv, link);
        (void) dhcp6_remove_lease(lv);
    }
    /* 
       if (iaidaddr->client6_info.serverid.duid_id != NULL)
       duidfree(&iaidaddr->client6_info.serverid); */
    if (iaidaddr->timer)
        dhcp6_remove_timer(iaidaddr->timer);
    TAILQ_INIT(&iaidaddr->lease_list);
    return 0;
}

int dhcp6_remove_lease(struct dhcp6_lease *sp) {
    dhcpv6_dprintf(LOG_DEBUG, "%s" "removing address %s", FNAME,
                   in6addr2str(&sp->lease_addr.addr, 0));
    sp->state = INVALID;
#ifdef LIBDHCP
    if (libdhcp_control
        && (libdhcp_control->capability & DHCP_USE_LEASE_DATABASE))
#endif
        if (write_lease(sp, client6_lease_file) != 0) {
            dhcpv6_dprintf(LOG_INFO, "%s"
                           "failed to write removed lease address %s to lease file",
                           FNAME, in6addr2str(&sp->lease_addr.addr, 0));
            return (-1);
        }
    /* XXX: ToDo: prefix delegation for client */
    if (sp->lease_addr.type == IAPD) {
        dhcpv6_dprintf(LOG_INFO, "request prefix is %s/%d",
                       in6addr2str(&sp->lease_addr.addr, 0),
                       sp->lease_addr.plen);
        /* XXX: remove from the update prefix list */

    } else
#ifdef LIBDHCP
    if (libdhcp_control
            && (libdhcp_control->capability & DHCP_CONFIGURE_ADDRESSES))
#endif
        if (client6_ifaddrconf(IFADDRCONF_REMOVE, &sp->lease_addr) != 0) {
            dhcpv6_dprintf(LOG_INFO, "%s" "removing address %s failed",
                           FNAME, in6addr2str(&sp->lease_addr.addr, 0));
        }
    /* remove expired timer for this lease. */
    if (sp->timer)
        dhcp6_remove_timer(sp->timer);
    TAILQ_REMOVE(&client6_iaidaddr.lease_list, sp, link);
    free(sp);
    /* can't remove expired iaidaddr even there is no lease in this iaidaddr
       since the rebind->solicit timer uses this iaidaddr
       if(TAILQ_EMPTY(&client6_iaidaddr.lease_list)) dhcp6_remove_iaidaddr(); */
    return 0;
}

int dhcp6_update_iaidaddr(struct dhcp6_optinfo *optinfo,
                          struct ia_listval *ia, int flag) {
    struct dhcp6_listval *lv, *lv_next = NULL;
    struct dhcp6_lease *cl;
    struct timeval timo;
    double d;

    if (client6_iaidaddr.client6_info.iaidinfo.renewtime >
        client6_iaidaddr.client6_info.iaidinfo.rebindtime) {
        dhcpv6_dprintf(LOG_INFO, " T1 time is greater than T2 time");
        return (0);
    }

    if (flag == ADDR_REMOVE) {
        for (lv = TAILQ_FIRST(&ia->addr_list); lv; lv = lv_next) {
            lv_next = TAILQ_NEXT(lv, link);
            cl = dhcp6_find_lease(&client6_iaidaddr, &lv->val_dhcp6addr);

            if (cl) {
                /* remove leases */
                dhcp6_remove_lease(cl);
            }
        }

        return 0;
    }

    /* flag == ADDR_UPDATE */
    for (lv = TAILQ_FIRST(&ia->addr_list); lv; lv = lv_next) {
        lv_next = TAILQ_NEXT(lv, link);

        if (lv->val_dhcp6addr.type != IAPD) {
            lv->val_dhcp6addr.plen =
                dhcp6_get_prefixlen(&lv->val_dhcp6addr.addr, dhcp6_if);

            if (lv->val_dhcp6addr.plen == PREFIX_LEN_NOTINRA) {
                dhcpv6_dprintf(LOG_WARNING,
                               "assigned address %s is not in any RAs"
                               " prefix length using 64 bit instead",
                               in6addr2str(&lv->val_dhcp6addr.addr, 0));
            }
        }

        if ((cl = dhcp6_find_lease(&client6_iaidaddr,
                                   &lv->val_dhcp6addr)) != NULL) {
            /* update leases */
            dhcp6_update_lease(&lv->val_dhcp6addr, cl);
            continue;
        }

        /* need to add the new leases */
        if (dhcp6_add_lease(&lv->val_dhcp6addr)) {
            dhcpv6_dprintf(LOG_INFO, "%s" "failed to add a new addr lease %s",
                           FNAME, in6addr2str(&lv->val_dhcp6addr.addr, 0));
            continue;
        }

        continue;
    }

    /* update server id */
    if (client6_iaidaddr.state == REBIND) {
        if (duidcpy
            (&client6_iaidaddr.client6_info.serverid, &optinfo->serverID)) {
            dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy server ID", FNAME);
            return (-1);
        }
    }

    if (TAILQ_EMPTY(&client6_iaidaddr.lease_list)) {
        return 0;
    }

    /* set up renew T1, rebind T2 timer renew/rebind based on iaid */
    /* Should we process IA_TA, IA_NA differently */
    if (client6_iaidaddr.client6_info.iaidinfo.renewtime == 0) {
        u_int32_t min_plifetime;
        min_plifetime = get_min_preferlifetime(&client6_iaidaddr);

        if (min_plifetime == DHCP6_DURATITION_INFINITE) {
            client6_iaidaddr.client6_info.iaidinfo.renewtime = min_plifetime;
        } else {
            client6_iaidaddr.client6_info.iaidinfo.renewtime =
                min_plifetime / 2;
        }
    }

    if (client6_iaidaddr.client6_info.iaidinfo.rebindtime == 0) {
        client6_iaidaddr.client6_info.iaidinfo.rebindtime =
            get_min_preferlifetime(&client6_iaidaddr) * 4 / 5;
    }

    dhcpv6_dprintf(LOG_INFO, "renew time %d, rebind time %d",
                   client6_iaidaddr.client6_info.iaidinfo.renewtime,
                   client6_iaidaddr.client6_info.iaidinfo.rebindtime);

    if (client6_iaidaddr.client6_info.iaidinfo.renewtime == 0) {
        return 0;
    }

    if (client6_iaidaddr.client6_info.iaidinfo.renewtime ==
        DHCP6_DURATITION_INFINITE) {
        client6_iaidaddr.client6_info.iaidinfo.rebindtime =
            DHCP6_DURATITION_INFINITE;

        if (client6_iaidaddr.timer) {
            dhcp6_remove_timer(client6_iaidaddr.timer);
        }

        return 0;
    }

    /* update the start date and timer */
    if (client6_iaidaddr.timer == NULL) {
        if ((client6_iaidaddr.timer =
            dhcp6_add_timer(dhcp6_iaidaddr_timo,
                            &client6_iaidaddr)) == NULL) {
            dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a timer for iaid %u",
                           FNAME,
                           client6_iaidaddr.client6_info.iaidinfo.iaid);
            return -1;
        }
    }

    time(&client6_iaidaddr.start_date);
    client6_iaidaddr.state = ACTIVE;
    d = client6_iaidaddr.client6_info.iaidinfo.renewtime;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcp6_set_timer(&timo, client6_iaidaddr.timer);

    return 0;
}

static int dhcp6_update_lease(struct dhcp6_addr *addr, struct dhcp6_lease *sp) {
    struct timeval timo;
    double d;

    if (addr->status_code != DH6OPT_STCODE_SUCCESS &&
        addr->status_code != DH6OPT_STCODE_UNDEFINE) {
        dhcpv6_dprintf(LOG_ERR,
                       "%s" "not successful status code for %s is %s", FNAME,
                       in6addr2str(&addr->addr, 0),
                       dhcp6_stcodestr(addr->status_code));
        dhcp6_remove_lease(sp);
        return 0;
    }

    /* remove leases with validlifetime == 0, and preferlifetime == 0 */
    if (addr->validlifetime == 0 || addr->preferlifetime == 0 ||
        addr->preferlifetime > addr->validlifetime) {
        dhcpv6_dprintf(LOG_ERR, "%s" "invalid address life time for %s",
                       FNAME, in6addr2str(&addr->addr, 0));
        dhcp6_remove_lease(sp);
        return 0;
    }

    memcpy(&sp->lease_addr, addr, sizeof(sp->lease_addr));
    sp->state = ACTIVE;
    time(&sp->start_date);
#ifdef LIBDHCP
    if (libdhcp_control
        && (libdhcp_control->capability & DHCP_USE_LEASE_DATABASE))
#endif
        if (write_lease(sp, client6_lease_file) != 0) {
            dhcpv6_dprintf(LOG_ERR, "%s"
                           "failed to write an updated lease address %s to lease file",
                           FNAME, in6addr2str(&sp->lease_addr.addr, 0));
            return -1;
        }

    if (sp->lease_addr.validlifetime == DHCP6_DURATITION_INFINITE ||
        sp->lease_addr.preferlifetime == DHCP6_DURATITION_INFINITE) {
        dhcpv6_dprintf(LOG_INFO, "%s" "infinity address life time for %s",
                       FNAME, in6addr2str(&addr->addr, 0));

        if (sp->timer) {
            dhcp6_remove_timer(sp->timer);
        }

        return 0;
    }

    if (sp->timer == NULL) {
        if ((sp->timer = dhcp6_add_timer(dhcp6_lease_timo, sp)) == NULL) {
            dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a timer for lease %s",
                           FNAME, in6addr2str(&addr->addr, 0));
            return -1;
        }
    }

    d = sp->lease_addr.preferlifetime;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcp6_set_timer(&timo, sp->timer);

    return 0;
}

struct dhcp6_lease *dhcp6_find_lease(struct dhcp6_iaidaddr *iaidaddr,
                                     struct dhcp6_addr *ifaddr) {
    struct dhcp6_lease *sp;

    for (sp = TAILQ_FIRST(&iaidaddr->lease_list); sp;
         sp = TAILQ_NEXT(sp, link)) {
        /* sp->lease_addr.plen == ifaddr->plen */
        dhcpv6_dprintf(LOG_DEBUG, "%s" "get address is %s/%d ", FNAME,
                       in6addr2str(&ifaddr->addr, 0), ifaddr->plen);
        dhcpv6_dprintf(LOG_DEBUG, "%s" "lease address is %s/%d ", FNAME,
                       in6addr2str(&sp->lease_addr.addr, 0), ifaddr->plen);

        if (IN6_ARE_ADDR_EQUAL(&sp->lease_addr.addr, &ifaddr->addr)) {
            if (sp->lease_addr.type == IAPD) {
                if (sp->lease_addr.plen == ifaddr->plen) {
                    return sp;
                }
            } else if (sp->lease_addr.type == IANA ||
                       sp->lease_addr.type == IATA) {
                return sp;
            }
        }
    }

    return NULL;
}

static struct dhcp6_event *dhcp6_iaidaddr_find_event(struct dhcp6_iaidaddr
                                                     *sp, int state) {
    struct dhcp6_event *ev;

    TAILQ_FOREACH(ev, &sp->ifp->event_list, link) {
        if (ev->state == state)
            return ev;
    }

    return NULL;
}

struct dhcp6_timer *dhcp6_iaidaddr_timo(void *arg) {
    struct dhcp6_iaidaddr *sp = (struct dhcp6_iaidaddr *) arg;
    struct dhcp6_event *ev, *prev_ev = NULL;
    struct timeval timeo;
    int dhcpstate;
    double d = 0;

    dhcpv6_dprintf(LOG_DEBUG, "client6_iaidaddr timeout for %d, state=%d",
                   client6_iaidaddr.client6_info.iaidinfo.iaid, sp->state);

    dhcp6_clear_list(&request_list);
    TAILQ_INIT(&request_list);

    /* ToDo: what kind of opiton Request value, client would like to pass? */
    switch (sp->state) {
        case ACTIVE:
            sp->state = RENEW;
            dhcpstate = DHCP6S_RENEW;
            d = sp->client6_info.iaidinfo.rebindtime -
                sp->client6_info.iaidinfo.renewtime;
            timeo.tv_sec = (long) d;
            timeo.tv_usec = 0;
            break;
        case RENEW:
            sp->state = REBIND;
            dhcpstate = DHCP6S_REBIND;
            prev_ev = dhcp6_iaidaddr_find_event(sp, DHCP6S_RENEW);
            d = get_max_validlifetime(&client6_iaidaddr) -
                sp->client6_info.iaidinfo.rebindtime;
            timeo.tv_sec = (long) d;
            timeo.tv_usec = 0;
            if (sp->client6_info.serverid.duid_id != NULL)
                duidfree(&sp->client6_info.serverid);
            break;
        case REBIND:
            dhcpv6_dprintf(LOG_INFO,
                           "%s" "failed to rebind a client6_iaidaddr %d"
                           " go to solicit and request new ipv6 addresses",
                           FNAME,
                           client6_iaidaddr.client6_info.iaidinfo.iaid);
            sp->state = INVALID;
            dhcpstate = DHCP6S_SOLICIT;
            prev_ev = dhcp6_iaidaddr_find_event(sp, DHCP6S_REBIND);
            free_servers(sp->ifp);
            break;
        default:
            return NULL;
    }

    /* Remove the event for the previous state */
    if (prev_ev) {
        dhcpv6_dprintf(LOG_DEBUG, "%s" "remove previous event for state=%d",
                       FNAME, prev_ev->state);
        dhcp6_remove_event(prev_ev);
    }

    /* Create a new event for the new state */
    if ((ev = dhcp6_create_event(sp->ifp, dhcpstate)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to create a new event", FNAME);
        return NULL;          /* XXX: should try to recover reserve
                                 memory?? */
    }

    switch (sp->state) {
        case RENEW:
            if (duidcpy(&ev->serverid, &sp->client6_info.serverid)) {
                dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy server ID",
                               FNAME);
                free(ev);
                return (NULL);
            }
        case REBIND:
            /* BUG: d not set! */
            ev->max_retrans_dur = d;
            break;
        default:
            break;
    }

    if ((ev->timer = dhcp6_add_timer(client6_timo, ev)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to create a new event timer",
                       FNAME);
        if (sp->state == RENEW) {
            duidfree(&ev->serverid);
        }
        free(ev);
        return NULL;          /* XXX */
    }

    TAILQ_INSERT_TAIL(&sp->ifp->event_list, ev, link);

    if (sp->state != INVALID) {
        struct dhcp6_lease *cl;

        /* create an address list for renew and rebind */
        for (cl = TAILQ_FIRST(&client6_iaidaddr.lease_list); cl;
             cl = TAILQ_NEXT(cl, link)) {
            struct dhcp6_listval *lv;

            /* IA_NA address */
            if ((lv = malloc(sizeof(*lv))) == NULL) {
                dhcpv6_dprintf(LOG_ERR, "%s"
                               "failed to allocate memory for an ipv6 addr",
                               FNAME);

                if (sp->state == RENEW) {
                    duidfree(&ev->serverid);
                }

                free(ev->timer);
                free(ev);
                return NULL;
            }

            memcpy(&lv->val_dhcp6addr, &cl->lease_addr,
                   sizeof(lv->val_dhcp6addr));
            lv->val_dhcp6addr.status_code = DH6OPT_STCODE_UNDEFINE;
            TAILQ_INSERT_TAIL(&request_list, lv, link);
        }

        dhcp6_set_timer(&timeo, sp->timer);
    } else {
        dhcp6_remove_iaidaddr(&client6_iaidaddr);
        /* remove event data for that event */
        sp->timer = NULL;
    }

    ev->timeouts = 0;
    dhcp6_set_timeoparam(ev);
    dhcp6_reset_timer(ev);
    client6_send(ev);

    return sp->timer;
}

struct dhcp6_timer *dhcp6_lease_timo(void *arg) {
    struct dhcp6_lease *sp = (struct dhcp6_lease *) arg;
    struct timeval timeo;
    double d;

    dhcpv6_dprintf(LOG_DEBUG, "%s" "lease timeout for %s, state=%d", FNAME,
                   in6addr2str(&sp->lease_addr.addr, 0), sp->state);

    /* cancel the current event for this lease */
    if (sp->state == INVALID) {
        dhcpv6_dprintf(LOG_INFO, "%s" "failed to remove an addr %s",
                       FNAME, in6addr2str(&sp->lease_addr.addr, 0));
        dhcp6_remove_lease(sp);
        return NULL;
    }

    switch (sp->state) {
        case ACTIVE:
            sp->state = EXPIRED;
            d = sp->lease_addr.validlifetime - sp->lease_addr.preferlifetime;
            timeo.tv_sec = (long) d;
            timeo.tv_usec = 0;
            dhcp6_set_timer(&timeo, sp->timer);
            break;
        case EXPIRED:
            sp->state = INVALID;
            dhcp6_remove_lease(sp);
        default:
            return NULL;
    }

    return sp->timer;
}

int client6_ifaddrconf(ifaddrconf_cmd_t cmd, struct dhcp6_addr *ifaddr) {
    struct in6_ifreq req;
    struct dhcp6_if *ifp = client6_iaidaddr.ifp;
    unsigned long ioctl_cmd;
    char *cmdstr;
    int s, errno;

    switch (cmd) {
        case IFADDRCONF_ADD:
            cmdstr = "add";
            ioctl_cmd = SIOCSIFADDR;
            break;
        case IFADDRCONF_REMOVE:
            cmdstr = "remove";
            ioctl_cmd = SIOCDIFADDR;
            break;
        default:
            return -1;
    }

    if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "can't open a temporary socket: %s",
                       FNAME, strerror(errno));
        return -1;
    }

    memset(&req, 0, sizeof(req));

#if defined(__linux__)
    req.ifr6_ifindex = if_nametoindex(ifp->ifname);
    memcpy(&req.ifr6_addr, &ifaddr->addr, sizeof(req.ifr6_addr));
    req.ifr6_prefixlen = ifaddr->plen;
#endif

#if defined(__MACOSX__)
    memcpy(&req.ifr_name, ifp->ifname, strlen(ifp->ifname));
    memcpy(&req.ifr_ifru.ifru_addr, &ifaddr->addr,
           sizeof(req.ifr_ifru.ifru_addr));
#endif

    if (ioctl(s, ioctl_cmd, &req) && errno != EEXIST) {
        dhcpv6_dprintf(LOG_NOTICE, "%s" "failed to %s an address on %s: %s",
                       FNAME, cmdstr, ifp->ifname, strerror(errno));
        close(s);
        return (-1);
    }

    dhcpv6_dprintf(LOG_DEBUG, "%s" "%s an address %s on %s", FNAME, cmdstr,
                   in6addr2str(&ifaddr->addr, 0), ifp->ifname);
    close(s);

    return 0;
}

int get_iaid(const char *ifname, const struct iaid_table *iaidtab,
             int num_device) {
    struct hardware hdaddr;
    struct iaid_table *temp = (struct iaid_table *) iaidtab;
    int i;

    hdaddr.len = gethwid(hdaddr.data, 6, ifname, &hdaddr.type);

    for (i = 0; i < num_device; i++, temp++) {
        if (!memcmp(temp->hwaddr.data, hdaddr.data, temp->hwaddr.len)
            && hdaddr.len == temp->hwaddr.len
            && hdaddr.type == temp->hwaddr.type) {
            dhcpv6_dprintf(LOG_DEBUG, "%s" " found interface %s iaid %u",
                           FNAME, ifname, temp->iaid);
            return temp->iaid;
        } else {
            continue;
        }
    }

    return 0;
}

int create_iaid(struct iaid_table *iaidtab, int num_device) {
    struct iaid_table *temp = iaidtab;
    struct ifaddrs *ifa = NULL, *ifap = NULL;
    int i;

    if (getifaddrs(&ifap) != 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "getifaddrs", FNAME);
        return -1;
    }

    for (i = 0, ifa = ifap;
         (ifa != NULL) && (i < MAX_DEVICE); i++, ifa = ifa->ifa_next) {
        if (!strcmp(ifa->ifa_name, "lo")) {
            continue;
        }

        temp->hwaddr.len = gethwid(temp->hwaddr.data,
                                   sizeof(temp->hwaddr.data),
                                   ifa->ifa_name,
                                   &temp->hwaddr.type);

        switch (temp->hwaddr.type) {
            case ARPHRD_ETHER:
            case ARPHRD_IEEE802:
                memcpy(&temp->iaid, (temp->hwaddr.data) + 2,
                       sizeof(temp->iaid));
                break;
#if defined(__linux__)
            case ARPHRD_PPP:
                temp->iaid = do_hash(ifa->ifa_name, sizeof(ifa->ifa_name))
                    + if_nametoindex(ifa->ifa_name);
                break;
#endif
            default:
                dhcpv6_dprintf(LOG_INFO,
                               "doesn't support %s address family %d",
                               ifa->ifa_name, temp->hwaddr.type);
                continue;
        }

        dhcpv6_dprintf(LOG_DEBUG, "%s" " create iaid %u for interface %s",
                       FNAME, temp->iaid, ifa->ifa_name);
        num_device++;
        temp++;
    }

    freeifaddrs(ifap);
    return num_device;
}

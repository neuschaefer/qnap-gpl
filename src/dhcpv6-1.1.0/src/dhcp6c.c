/* ported from KAME: dhcp6c.c,v 1.97 2002/09/24 14:20:49 itojun Exp */

/*
 * Copyright (C) 1998 and 1999 WIDE Project.
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

#include "config.h"

#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <libgen.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/param.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# include <time.h>
#endif

#ifdef HAVE_SYS_TIMEB_H
# include <sys/timeb.h>
#endif

#ifdef HAVE_LINUX_SOCKIOS_H
# include <linux/sockios.h>
#endif

#ifdef HAVE_NET_IF_VAR_H
# include <net/if_var.h>
#endif

#ifdef HAVE_NETINET6_IN6_VAR_H
# include <netinet6/in6_var.h>
#endif

#include <linux/netlink.h>
#include <netlink/socket.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>

#include "dhcp6.h"
#include "cfg.h"
#include "common.h"
#include "timer.h"
#include "lease.h"

#ifdef LIBDHCP
#include "libdhcp_control.h"
#endif

static int debug = 0;
static u_long sig_flags = 0;

#define SIGF_TERM 0x1
#define SIGF_HUP 0x2
#define SIGF_CLEAN 0x4

#define DHCP6S_VALID_REPLY(a) \
	(a == DHCP6S_REQUEST || a == DHCP6S_RENEW || \
	 a == DHCP6S_REBIND || a == DHCP6S_DECLINE || \
	 a == DHCP6S_RELEASE || a == DHCP6S_CONFIRM || \
	 a == DHCP6S_INFOREQ)


const dhcp6_mode_t dhcp6_mode = DHCP6_MODE_CLIENT;

static char *device = NULL;
static int num_device = 0;
static struct iaid_table iaidtab[MAX_DEVICE];
static u_int8_t client6_request_flag = 0;
static char leasename[100];

#define CLIENT6_RELEASE_ADDR	0x1
#define CLIENT6_CONFIRM_ADDR	0x2
#define CLIENT6_REQUEST_ADDR	0x4
#define CLIENT6_DECLINE_ADDR	0x8

#define CLIENT6_INFO_REQ	0x10

int iosock = -1;                /* inbound/outbound udp port */
int nlsock = -1;

extern char *raproc_file;
extern char *ifproc_file;
extern FILE *client6_lease_file;
extern struct dhcp6_iaidaddr client6_iaidaddr;
FILE *dhcp6_resolv_file;
static const struct sockaddr_in6 *sa6_allagent;
static socklen_t sa6_alen;
static struct duid client_duid;

static void usage __P((char *name));
static int client6_init __P((char *));
static int client6_ifinit __P((char *));
static iatype_t iatype_of_if __P((struct dhcp6_if *));
void free_servers __P((struct dhcp6_if *));
static void free_resources __P((struct dhcp6_if *));
static int create_request_list __P((int));
static void client6_mainloop __P((void));
static void process_signals __P((void));
static struct dhcp6_serverinfo *find_server __P((struct dhcp6_if *,
                                                 struct duid *));
static struct dhcp6_serverinfo *allocate_newserver
__P((struct dhcp6_if *, struct dhcp6_optinfo *));
static struct dhcp6_serverinfo *select_server __P((struct dhcp6_if *));
void client6_send __P((struct dhcp6_event *));
int client6_send_newstate __P((struct dhcp6_if *, int));
static void client6_recv __P((void));
static int client6_recvadvert __P((struct dhcp6_if *, struct dhcp6 *,
                                   ssize_t, struct dhcp6_optinfo *));
static int client6_recvreply __P((struct dhcp6_if *, struct dhcp6 *,
                                  ssize_t, struct dhcp6_optinfo *));
static int set_info_refresh_timer __P((struct dhcp6_if *, u_int32_t));
static struct dhcp6_timer *info_refresh_timo __P((void *));
#ifndef LIBDHCP
static void client6_signal __P((int));
#endif
static struct dhcp6_event *find_event_withid __P((struct dhcp6_if *,
                                                  u_int32_t));
static struct dhcp6_timer *check_lease_file_timo __P((void *));
static struct dhcp6_timer *check_link_timo __P((void *));
static struct dhcp6_timer *check_dad_timo __P((void *));
static void setup_check_timer __P((struct dhcp6_if *));
static void setup_interface __P((char *));
struct dhcp6_timer *client6_timo __P((void *));
extern int client6_ifaddrconf __P((ifaddrconf_cmd_t, struct dhcp6_addr *));
extern struct dhcp6_timer *syncfile_timo __P((void *));
extern int dad_parse __P((const char *, struct dhcp6_list *));

#define DHCP6C_CONF "/etc/dhcp6c.conf"
#define DHCP6C_PIDFILE "/var/run/dhcpv6/dhcp6c.pid"
#define DUID_FILE "/var/lib/dhcpv6/dhcp6c_duid"

static int pid;
static char pidfile[MAXPATHLEN];

#ifdef LIBDHCP
struct sockaddr_in6 sa6_allagent_storage;
#endif
char client6_lease_temp[256];
struct dhcp6_list request_list;

#ifndef LIBDHCP
int main(int argc, char **argv, char **envp)
#else
#define exit return
LIBDHCP_Control *libdhcp_control;
int dhcpv6_client(LIBDHCP_Control *libdhcp_ctl,
                  int argc, char **argv, char **envp)
#endif
{
    int ch;
    char *progname, *conffile = DHCP6C_CONF;
    FILE *pidfp;
    char *addr;

#ifdef LIBDHCP
    libdhcp_control = libdhcp_ctl;
#endif

    pid = getpid();
    srandom(time(NULL) & pid);

    strcpy(pidfile, DHCP6C_PIDFILE);

    if ((progname = strrchr(*argv, '/')) == NULL)
        progname = *argv;
    else
        progname++;

    TAILQ_INIT(&request_list);
    while ((ch = getopt(argc, argv, "c:r:R:P:vfIp:")) != -1) {
        switch (ch) {
            case 'p':
                if (strlen(optarg) >= MAXPATHLEN) {
                    dhcpv6_dprintf(LOG_ERR, "pid file name is too long");
                    exit(1);
                }

                strcpy(pidfile, optarg);
                break;
            case 'c':
                conffile = optarg;
                break;
            case 'P':
                client6_request_flag |= CLIENT6_REQUEST_ADDR;

                for (addr = strtok(optarg, " "); addr;
                     addr = strtok(NULL, " ")) {
                    struct dhcp6_listval *lv;

                    if ((lv = (struct dhcp6_listval *) malloc(sizeof(*lv)))
                        == NULL) {
                        dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
                        exit(1);
                    }

                    memset(lv, 0, sizeof(*lv));

                    if (inet_pton(AF_INET6, strtok(addr, "/"),
                                  &lv->val_dhcp6addr.addr) < 1) {
                        dhcpv6_dprintf(LOG_ERR,
                                       "invalid ipv6address for release");
                        usage(argv[0]);
                        exit(1);
                    }

                    lv->val_dhcp6addr.type = IAPD;
                    lv->val_dhcp6addr.plen = atoi(strtok(NULL, "/"));
                    lv->val_dhcp6addr.status_code = DH6OPT_STCODE_UNDEFINE;
                    TAILQ_INSERT_TAIL(&request_list, lv, link);
                }

                break;
            case 'R':
                client6_request_flag |= CLIENT6_REQUEST_ADDR;

                for (addr = strtok(optarg, " "); addr;
                     addr = strtok(NULL, " ")) {
                    struct dhcp6_listval *lv;

                    if ((lv = (struct dhcp6_listval *) malloc(sizeof(*lv)))
                        == NULL) {
                        dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
                        exit(1);
                    }

                    memset(lv, 0, sizeof(*lv));

                    if (inet_pton(AF_INET6, addr, &lv->val_dhcp6addr.addr) <
                        1) {
                        dhcpv6_dprintf(LOG_ERR,
                                       "invalid ipv6address for release");
                        usage(argv[0]);
                        exit(1);
                    }

                    lv->val_dhcp6addr.type = IANA;
                    lv->val_dhcp6addr.status_code = DH6OPT_STCODE_UNDEFINE;
                    TAILQ_INSERT_TAIL(&request_list, lv, link);
                }

                break;
            case 'r':
                client6_request_flag |= CLIENT6_RELEASE_ADDR;

                if (strcmp(optarg, "all")) {
                    for (addr = strtok(optarg, " "); addr;
                         addr = strtok(NULL, " ")) {
                        struct dhcp6_listval *lv;

                        if ((lv =
                             (struct dhcp6_listval *) malloc(sizeof(*lv)))
                            == NULL) {
                            dhcpv6_dprintf(LOG_ERR,
                                           "failed to allocate memory");
                            exit(1);
                        }

                        memset(lv, 0, sizeof(*lv));

                        if (inet_pton(AF_INET6, addr,
                                      &lv->val_dhcp6addr.addr) < 1) {
                            dhcpv6_dprintf(LOG_ERR,
                                           "invalid ipv6address for release");
                            usage(argv[0]);
                            exit(1);
                        }

                        lv->val_dhcp6addr.type = IANA;
                        TAILQ_INSERT_TAIL(&request_list, lv, link);
                    }
                }

                break;
            case 'I':
                client6_request_flag |= CLIENT6_INFO_REQ;
                break;
            case 'v':
                debug = 2;
                break;
            case 'f':
                foreground++;
                break;
            default:
                usage(argv[0]);
                exit(0);
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage(argv[0]);
        exit(0);
    }
    device = argv[0];

#ifndef __DARWIN_DEPRECATED_ATTRIBUTE
    if (foreground == 0) {
        if (daemon(0, 0) < 0) {
            err(1, "daemon");
        }

        openlog(progname, LOG_NDELAY | LOG_PID, LOG_DAEMON);

        /* Get out pid again now that we've daemonized so we log the proper
         * value in the pid file
         */
        pid = getpid();
    }
#endif

    setloglevel(debug);

#ifdef LIBDHCP
    if (libdhcp_control && (libdhcp_control->capability & DHCP_USE_PID_FILE))
#endif
        /* dump current PID */
        if ((pidfp = fopen(pidfile, "w")) != NULL) {
            fprintf(pidfp, "%d\n", pid);
            fclose(pidfp);
        }
#ifdef LIBDHCP
    sa6_allagent = (const struct sockaddr_in6 *) &sa6_allagent_storage;
#endif
    ifinit(device);
    setup_interface(device);

    if ((cfparse(conffile)) != 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to parse configuration file",
                       FNAME);
        exit(1);
    }

    if (client6_init(device)) {
        return -1;
    }

    if (client6_ifinit(device)) {
        return -1;
    }

    client6_mainloop();

#ifdef LIBDHCP
    /* close all file descriptors */
    close(nlsock);
    nlsock = -1;
    close(iosock);
    iosock = -1;
    closelog();

    /* release all memory */
    sleep(1);                   /* keep valgrind happy :-) */
    dhc6_free_all_pointers();

    /* initialize globals */
    optarg = 0L;
    optind = 0;
    opterr = 0;
    optopt = 0;
    memset(&client6_iaidaddr, '\0', sizeof(client6_iaidaddr));
    dhcp6_if = NULL;
    extern LIST_HEAD(, dhcp6_timer) timer_head;

    memset(&timer_head, '\0', sizeof(timer_head));
    memset(&request_list, '\0', sizeof(request_list));
    memset(&sa6_allagent_storage, '\0', sizeof(sa6_allagent_storage));
    sa6_allagent = (const struct sockaddr_in6 *) &sa6_allagent_storage;
    memset(&client_duid, '\0', sizeof(client_duid));
    memset(&iaidtab, '\0', sizeof(iaidtab));
    client6_request_flag = 0;
    memset(&leasename, '\0', sizeof(leasename));
    debug = 0;
    device = NULL;
    num_device = 0;
    sig_flags = 0;
    extern struct host_conf *host_conflist;

    host_conflist = 0;
    client6_lease_file = server6_lease_file = sync_file = NULL;
    cf_dns_list = NULL;
    extern int cfdebug;

    cfdebug = 0;
    hash_anchors = 0;
    configfilename = NULL;
    debug_thresh = 0;
    memset(&dnslist, '\0', sizeof(dnslist));
    memset(&resolv_dhcpv6_file, '\0', sizeof(resolv_dhcpv6_file));
    memset(&client6_lease_temp, '\0', sizeof(client6_lease_temp));
    foreground = 0;
#endif
    return (0);
}

static void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] interface\n", basename(name));
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -c PATH        Configuration file (e.g., /etc/dhcp6c.conf)\n");
    fprintf(stderr, "    -p PATH        PID file name (default: %s)\n", DHCP6C_PIDFILE);
    fprintf(stderr, "    -r ADDR...     Release the specified addresses (either \"all\" or\n                    named addresses)\n");
    fprintf(stderr, "    -R ADDR...     Request the specified IANA address(es)\n");
    fprintf(stderr, "    -P ADDR...     Request the specified IAPD address(es)\n");
    fprintf(stderr, "    -I             Request only information from the server\n");
    fprintf(stderr, "    -v             Verbose debugging output\n");
    fprintf(stderr, "    -f             Run client as a foreground process\n");
    fprintf(stderr, "IANA is identiy association named address.\n");
    fprintf(stderr, "IAPD is identiy association prefix delegation.\n");
    fflush(stderr);
    return;
}

int client6_init(char *device) {
    struct addrinfo hints, *res;

#ifndef LIBDHCP
    static struct sockaddr_in6 sa6_allagent_storage;
#endif
    int error, on = 1;
    struct dhcp6_if *ifp;
    int ifidx;
    char linklocal[64];
    struct in6_addr lladdr;
    time_t retry, now;
    int bound;

#ifdef QNAP
		char qnap_duid_file[64];
#endif

    ifidx = if_nametoindex(device);
    if (ifidx == 0) {
        dhcpv6_dprintf(LOG_ERR, "if_nametoindex(%s)", device);
        return -1;
    }

    /* get our DUID */
#ifdef QNAP
		snprintf(qnap_duid_file,sizeof(qnap_duid_file),"%s%s",DUID_FILE,device);
		if (get_duid(qnap_duid_file, device, &client_duid)) {
#else
    if (get_duid(DUID_FILE, device, &client_duid)) {
#endif
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to get a DUID", FNAME);
        return -1;
    }

    if (get_linklocal(device, &lladdr) < 0) {
        return -1;
    }

    if (inet_ntop(AF_INET6, &lladdr, linklocal, sizeof(linklocal)) < 0) {
        return -1;
    }

    dhcpv6_dprintf(LOG_DEBUG, "link local addr is %s", linklocal);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = 0;
    error = getaddrinfo(linklocal, DH6PORT_DOWNSTREAM, &hints, &res);

    if (error) {
        dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                       FNAME, strerror(error));
        return -1;
    }

    iosock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (iosock < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "socket", FNAME);
        return -1;
    }

#ifdef IPV6_RECVPKTINFO
    if (setsockopt(iosock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on,
                   sizeof(on)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s"
                       "setsockopt(inbound, IPV6_RECVPKTINFO): %s",
                       FNAME, strerror(errno));
        return -1;
    }
#else
    if (setsockopt(iosock, IPPROTO_IPV6, IPV6_PKTINFO, &on, sizeof(on)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s"
                       "setsockopt(inbound, IPV6_PKTINFO): %s",
                       FNAME, strerror(errno));
        return -1;
    }
#endif

    ((struct sockaddr_in6 *) (res->ai_addr))->sin6_scope_id = ifidx;
    dhcpv6_dprintf(LOG_DEBUG, "res addr is %s/%d",
                   addr2str(res->ai_addr, res->ai_addrlen), res->ai_addrlen);

    /* 
     * If the interface has JUST been brought up, the kernel may not have
     * enough time to allow the bind to the linklocal address - it will
     * then return EADDRNOTAVAIL. The bind will succeed if we try again.
     */
    retry = now = time(0);
    bound = 0;
    do {
        if (bind(iosock, res->ai_addr, res->ai_addrlen) < 0) {
            bound = -errno;
            retry = time(0);
            if ((bound != -EADDRNOTAVAIL) || ((retry - now) > 5))
                break;
            struct timespec tv = { 0, 200000000 };
            nanosleep(&tv, 0);
        } else {
            bound = 1;
            break;
        }
    } while ((retry - now) < 5);

    if (bound < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "bind: %s", FNAME, strerror(-bound));
        return -1;
    }

    freeaddrinfo(res);

    /* initiallize socket address structure for outbound packets */
    hints.ai_flags = 0;
    error = getaddrinfo(linklocal, DH6PORT_UPSTREAM, &hints, &res);
    if (error) {
        dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                       FNAME, gai_strerror(error));
        return -1;
    }

    if (setsockopt(iosock, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                   &ifidx, sizeof(ifidx)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s"
                       "setsockopt(iosock, IPV6_MULTICAST_IF): %s",
                       FNAME, strerror(errno));
        return -1;
    }

    ((struct sockaddr_in6 *) (res->ai_addr))->sin6_scope_id = ifidx;
    freeaddrinfo(res);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    error = getaddrinfo(DH6ADDR_ALLAGENT, DH6PORT_UPSTREAM, &hints, &res);

    if (error) {
        dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                       FNAME, gai_strerror(error));
        return -1;
    }

    memcpy(&sa6_allagent_storage, res->ai_addr, res->ai_addrlen);
    sa6_allagent = (const struct sockaddr_in6 *) &sa6_allagent_storage;
    sa6_alen = res->ai_addrlen;
    freeaddrinfo(res);

    /* client interface configuration */
    if ((ifp = find_ifconfbyname(device)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "interface %s not configured",
                       FNAME, device);
        return -1;
    }

    ifp->outsock = iosock;

#ifndef LIBDHCP
    if (signal(SIGHUP, client6_signal) == SIG_ERR) {
        dhcpv6_dprintf(LOG_WARNING, "%s" "failed to set signal: %s",
                       FNAME, strerror(errno));
        return -1;
    }

    if (signal(SIGTERM | SIGKILL, client6_signal) == SIG_ERR) {
        dhcpv6_dprintf(LOG_WARNING, "%s" "failed to set signal: %s",
                       FNAME, strerror(errno));
        return -1;
    }

    if (signal(SIGINT, client6_signal) == SIG_ERR) {
        dhcpv6_dprintf(LOG_WARNING, "%s" "failed to set signal: %s",
                       FNAME, strerror(errno));
        return -1;
    }
#endif

    return 0;
}

/*
 * Call libnl and collect information about the current state of the interface.
 */
int get_if_rainfo(struct dhcp6_if *ifp) {
    struct nl_handle *handle = NULL;
    struct nl_cache *cache = NULL;
    struct nl_object *obj = NULL;
    struct rtnl_addr *raddr = NULL;
    struct rtnl_link *link = NULL;
    struct nl_addr *addr = NULL;
    char buf[INET6_ADDRSTRLEN+1];
    struct in6_addr *tmpaddr = NULL;
    struct ra_info *rainfo = NULL, *ra = NULL, *ra_prev = NULL;

    memset(&buf, '\0', sizeof(buf));

    if ((handle = nl_handle_alloc()) == NULL) {
        return 1;
    }

    if (nl_connect(handle, NETLINK_ROUTE)) {
        nl_handle_destroy(handle);
        return 2;
    }

    if ((cache = rtnl_addr_alloc_cache(handle)) == NULL) {
        nl_close(handle);
        nl_handle_destroy(handle);
        return 3;
    }

    if ((obj = nl_cache_get_first(cache)) == NULL) {
        nl_close(handle);
        nl_handle_destroy(handle);
        return 4;
    }

    do {
        raddr = (struct rtnl_addr *) obj;

        /*
         * Copy IPv6 prefix addresses and associated values in to our
         * ifp->ralist array.
         */
        if ((rtnl_addr_get_ifindex(raddr) == ifp->ifid) &&
            (rtnl_addr_get_family(raddr) == AF_INET6) &&
            (rtnl_addr_get_scope(raddr) & RT_SCOPE_SITE)) {
            /* found a prefix address, add it to the list */
            addr = rtnl_addr_get_local(raddr);
            tmpaddr = (struct in6_addr *) nl_addr_get_binary_addr(addr);

            /* create a new rainfo struct and add it to the list of addresses */
            rainfo = (struct ra_info *) malloc(sizeof(*rainfo));
            if (rainfo == NULL) {
                nl_addr_destroy(addr);
                rtnl_addr_put(raddr);
                nl_close(handle);
                nl_handle_destroy(handle);
                return 5;
            }

            memset(rainfo, 0, sizeof(rainfo));
            memcpy((&rainfo->prefix), tmpaddr, sizeof(struct in6_addr));
            rainfo->plen = rtnl_addr_get_prefixlen(raddr);

            if (inet_ntop(AF_INET6, &(rainfo->prefix), buf, INET6_ADDRSTRLEN) == NULL) {
                nl_addr_destroy(addr);
                rtnl_addr_put(raddr);
                nl_close(handle);
                nl_handle_destroy(handle);
                return 6;
            }

            dhcpv6_dprintf(LOG_DEBUG, "get prefix address %s", buf);
            dhcpv6_dprintf(LOG_DEBUG, "get prefix plen %d", rainfo->plen);

            if (ifp->ralist == NULL) {
                ifp->ralist = rainfo;
                rainfo->next = NULL;
            } else {
                ra_prev = ifp->ralist;

                for (ra = ifp->ralist; ra; ra = ra->next) {
                    if (rainfo->plen >= ra->plen) {
                        if (ra_prev == ra) {
                            ifp->ralist = rainfo;
                            rainfo->next = ra;
                        } else {
                            ra_prev->next = rainfo;
                            rainfo->next = ra;
                        }

                        break;
                    } else {
                        if (ra->next == NULL) {
                            ra->next = rainfo;
                            rainfo->next = NULL;
                            break;
                        } else {
                            ra_prev = ra;
                            continue;
                        }
                    }
                }
            }

            nl_addr_destroy(addr);

            /* gather flags */
            if ((cache = rtnl_addr_alloc_cache(handle)) == NULL) {
                rtnl_addr_put(raddr);
                nl_close(handle);
                nl_handle_destroy(handle);
                return 7;
            }

            link = rtnl_link_get(cache, rtnl_addr_get_ifindex(raddr));

            if (link) {
                ifp->ra_flag = rtnl_link_get_flags(link);
                rtnl_link_put(link);
                rtnl_addr_put(raddr);
            }
        }
    } while ((obj = nl_cache_get_next(obj)) != NULL);

    nl_close(handle);
    nl_handle_destroy(handle);
    return 0;
}

static int client6_ifinit(char *device) {
    int err = 0;
    struct dhcp6_if *ifp = dhcp6_if;
    struct dhcp6_event *ev;
    char iaidstr[20];
#ifdef QNAP
		char qnap_duid_file[64];
#endif
    dhcp6_init_iaidaddr();
    /* get iaid for each interface */
    if (num_device == 0) {
        if ((num_device = create_iaid(&iaidtab[0], num_device)) < 0) {
            return -1;
        }

        if (ifp->iaidinfo.iaid == 0) {
            ifp->iaidinfo.iaid = get_iaid(ifp->ifname, &iaidtab[0],
                                          num_device);
        }

        if (ifp->iaidinfo.iaid == 0) {
            dhcpv6_dprintf(LOG_DEBUG, "%s"
                           "interface %s iaid failed to be created",
                           FNAME, ifp->ifname);
            return -1;
        }

        dhcpv6_dprintf(LOG_DEBUG, "%s" "interface %s iaid is %u",
                       FNAME, ifp->ifname, ifp->iaidinfo.iaid);
    }

    client6_iaidaddr.ifp = ifp;
    memcpy(&client6_iaidaddr.client6_info.iaidinfo, &ifp->iaidinfo,
           sizeof(client6_iaidaddr.client6_info.iaidinfo));
    duidcpy(&client6_iaidaddr.client6_info.clientid, &client_duid);
#ifdef QNAP
		snprintf(qnap_duid_file,sizeof(qnap_duid_file),"%s%s",DUID_FILE,device);
		save_duid(qnap_duid_file, device, &client_duid);
#else
    save_duid(DUID_FILE, device, &client_duid);
#endif
    if (!(ifp->send_flags & DHCIFF_INFO_ONLY) &&
            !(client6_request_flag & CLIENT6_INFO_REQ) &&
            ((ifp->ra_flag & IF_RA_MANAGED) ||
            !(ifp->ra_flag & IF_RA_OTHERCONF))) {
#ifdef LIBDHCP
        if (libdhcp_control
            && (libdhcp_control->capability & DHCP_USE_LEASE_DATABASE)) {
#endif
            /* parse the lease file */
            strcpy(leasename, PATH_CLIENT6_LEASE);
            sprintf(iaidstr, "%u", ifp->iaidinfo.iaid);
            strcat(leasename, iaidstr);

            if ((client6_lease_file = init_leases(leasename)) == NULL) {
                dhcpv6_dprintf(LOG_ERR, "%s" "failed to parse lease file",
                               FNAME);
                return -1;
            }

            strcpy(client6_lease_temp, leasename);
            strcat(client6_lease_temp, "XXXXXX");
            client6_lease_file = sync_leases(client6_lease_file,
                                             leasename, client6_lease_temp);

            if (client6_lease_file == NULL) {
                return -1;
            }
#ifdef LIBDHCP
        }
#endif

        if (!TAILQ_EMPTY(&client6_iaidaddr.lease_list)) {
            struct dhcp6_listval *lv;

            if (!(client6_request_flag & CLIENT6_REQUEST_ADDR) &&
                !(client6_request_flag & CLIENT6_RELEASE_ADDR)) {
                client6_request_flag |= CLIENT6_CONFIRM_ADDR;
            }

            if (TAILQ_EMPTY(&request_list)) {
                if (create_request_list(1) < 0) {
                    return -1;
                }
            } else if (client6_request_flag & CLIENT6_RELEASE_ADDR) {
                for (lv = TAILQ_FIRST(&request_list); lv;
                     lv = TAILQ_NEXT(lv, link)) {
                    if (dhcp6_find_lease(&client6_iaidaddr,
                                         &lv->val_dhcp6addr) == NULL) {
                        dhcpv6_dprintf(LOG_INFO, "this address %s is not"
                                       " leased by this client",
                                       in6addr2str(&lv->val_dhcp6addr.addr, 0));
                        return -1;
                    }
                }
            }
        } else if (client6_request_flag & CLIENT6_RELEASE_ADDR) {
            dhcpv6_dprintf(LOG_INFO, "no ipv6 addresses are leased by client");
            return -1;
        }
    }

    ifp->link_flag |= IFF_RUNNING;

    /* get addrconf prefix from kernel */
    err = get_if_rainfo(ifp);
    if (err) {
        dhcpv6_dprintf(LOG_ERR, "failed to get interface info via libnl: %d",
                       err);
        return -1;
    }

    /* set up check link timer and sync file timer */
    if ((ifp->link_timer = dhcp6_add_timer(check_link_timo, ifp)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to create a timer", FNAME);
        return -1;
    }

    if ((ifp->sync_timer = dhcp6_add_timer(check_lease_file_timo, ifp)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to create a timer", FNAME);
        return -1;
    }

    /* DAD timer set up after getting the address */
    ifp->dad_timer = NULL;

    /* create an event for the initial delay */
    if ((ev = dhcp6_create_event(ifp, DHCP6S_INIT)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to create an event", FNAME);
        return -1;
    }

    ifp->servers = NULL;
    ev->ifp->current_server = NULL;
    TAILQ_INSERT_TAIL(&ifp->event_list, ev, link);

    if ((ev->timer = dhcp6_add_timer(client6_timo, ev)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a timer for %s",
                       FNAME, ifp->ifname);
        return -1;
    }

    dhcp6_reset_timer(ev);
    return 0;
}

static iatype_t iatype_of_if(struct dhcp6_if *ifp) {
    if (ifp->send_flags & DHCIFF_PREFIX_DELEGATION) {
        return IAPD;
    } else if (ifp->send_flags & DHCIFF_TEMP_ADDRS) {
        return IATA;
    } else {
        return IANA;
    }
}

static void free_resources(struct dhcp6_if *ifp) {
    struct dhcp6_event *ev, *ev_next;
    struct dhcp6_lease *sp, *sp_next;
    struct stat buf;

    for (sp = TAILQ_FIRST(&client6_iaidaddr.lease_list); sp; sp = sp_next) {
        sp_next = TAILQ_NEXT(sp, link);
#ifdef LIBDHCP
        if (libdhcp_control
            && (libdhcp_control->capability & DHCP_CONFIGURE_ADDRESSES))
#endif
            if (client6_ifaddrconf(IFADDRCONF_REMOVE, &sp->lease_addr) != 0)
                dhcpv6_dprintf(LOG_INFO, "%s" "deconfiging address %s failed",
                               FNAME, in6addr2str(&sp->lease_addr.addr, 0));
    }

    dhcpv6_dprintf(LOG_DEBUG, "%s" " remove all events on interface", FNAME);

    /* cancel all outstanding events for each interface */
    for (ev = TAILQ_FIRST(&ifp->event_list); ev; ev = ev_next) {
        ev_next = TAILQ_NEXT(ev, link);
        dhcp6_remove_event(ev);
    }

    /* restore /etc/resolv.conf.dhcpv6.bak back to /etc/resolv.conf */
    if (!lstat(RESOLV_CONF_BAK_FILE, &buf)) {
        if (rename(RESOLV_CONF_BAK_FILE, RESOLV_CONF_FILE))
            dhcpv6_dprintf(LOG_ERR, "%s" " failed to backup resolv.conf",
                           FNAME);
    }

    free_servers(ifp);
}

static void process_signals() {
    if ((sig_flags & SIGF_TERM)) {
        dhcpv6_dprintf(LOG_INFO, FNAME "exiting");
        free_resources(dhcp6_if);
        unlink(pidfile);
#ifdef LIBDHCP
        return;
#else
        exit(0);
#endif
    }

    if ((sig_flags & SIGF_HUP)) {
        dhcpv6_dprintf(LOG_INFO, FNAME "restarting");
        free_resources(dhcp6_if);
        client6_ifinit(device);
    }

    if ((sig_flags & SIGF_CLEAN)) {
        free_resources(dhcp6_if);
        unlink(pidfile);
#ifdef LIBDHCP
        return;
#else
        exit(0);
#endif
    }

    sig_flags = 0;
}

static void client6_mainloop() {
    struct timeval *w;
    int ret;
    fd_set r;

#ifdef LIBDHCP
    struct timeval fb;          /* fallback timeout */

    if (libdhcp_control) {
        if (libdhcp_control->timeout)
            libdhcp_control->now = time(0);
        else
            libdhcp_control->now = 0;
    }
#endif

    while (1) {
        if (sig_flags)
            process_signals();
        w = dhcp6_check_timer();

#ifdef LIBDHCP
        if (libdhcp_control && libdhcp_control->timeout) {
            time_t now = time(0);
            double a = (double) w->tv_sec + now;
            double b = (double) w->tv_usec / 1000000.0;
            double c = (double) libdhcp_control->now;
            double d = (double) libdhcp_control->timeout;

            if ((w == NULL) || ((a + b) >= (c + d))) {
                w = &fb;
                fb.tv_sec = 0;
                fb.tv_usec = 0;
                if (now < (libdhcp_control->now + libdhcp_control->timeout))
                    fb.tv_sec =
                        (libdhcp_control->now + libdhcp_control->timeout) -
                        now;
            }
        }
#endif

        FD_ZERO(&r);
        FD_SET(iosock, &r);

        ret = select(iosock + 1, &r, NULL, NULL, w);
        switch (ret) {
            case -1:
                if (errno != EINTR) {
                    dhcpv6_dprintf(LOG_ERR, "%s" "select: %s",
                                   FNAME, strerror(errno));
                    return;
                }
                break;
            case 0:            /* timeout */
                break;          /* dhcp6_check_timer() will treat the case */
            default:           /* received a packet */
                client6_recv();
        }

#ifdef LIBDHCP
        if (libdhcp_control) {
            if (libdhcp_control->finished)
                return;

            if (libdhcp_control->timeout
                && (time(NULL) >=
                    (libdhcp_control->timeout + libdhcp_control->now))) {
                if (libdhcp_control->callback)
                    (*(libdhcp_control->callback)) (libdhcp_control,
                                                    DHC_TIMEDOUT,
                                                    &client6_iaidaddr);
                return;
            }
        }
#endif
    }
}

struct dhcp6_timer *client6_timo(arg)
     void *arg;
{
    struct dhcp6_event *ev = (struct dhcp6_event *) arg;
    struct dhcp6_if *ifp;
    struct timeval now;

    ifp = ev->ifp;
    ev->timeouts++;
    gettimeofday(&now, NULL);
    if ((ev->max_retrans_cnt && ev->timeouts >= ev->max_retrans_cnt) ||
        (ev->max_retrans_dur && (now.tv_sec - ev->start_time.tv_sec)
         >= ev->max_retrans_dur)) {
        /* XXX: check up the duration time for renew & rebind */
        dhcpv6_dprintf(LOG_INFO, "%s" "no responses were received", FNAME);
        dhcp6_remove_event(ev); /* XXX: should free event data? */
        return (NULL);
    }

    switch (ev->state) {
        case DHCP6S_INIT:
            /* From INIT state client could go to CONFIRM state if the client 
               reboots; go to RELEASE state if the client issues a release;
               go to INFOREQ state if the client requests info-only; go to
               SOLICIT state if the client requests addresses; */
            ev->timeouts = 0;   /* indicate to generate a new XID. */
            /* 
             * three cases client send information request:
             * 1. configuration file includes information-only
             * 2. command line includes -I
             * 3. check interface flags if managed bit isn't set and
             *    if otherconf bit set by RA
             *    and information-only, conmand line -I are not set.
             */
            if ((ifp->send_flags & DHCIFF_INFO_ONLY) ||
                (client6_request_flag & CLIENT6_INFO_REQ) ||
                (!(ifp->ra_flag & IF_RA_MANAGED) &&
                 (ifp->ra_flag & IF_RA_OTHERCONF)))
                ev->state = DHCP6S_INFOREQ;
            else if (client6_request_flag & CLIENT6_RELEASE_ADDR)
                /* do release */
                ev->state = DHCP6S_RELEASE;
            else if (client6_request_flag & CLIENT6_CONFIRM_ADDR) {
                struct dhcp6_listval *lv;

                /* do confirm for reboot for IANA, IATA */
                if (client6_iaidaddr.client6_info.type == IAPD)
                    ev->state = DHCP6S_REBIND;
                else
                    ev->state = DHCP6S_CONFIRM;
                for (lv = TAILQ_FIRST(&request_list); lv;
                     lv = TAILQ_NEXT(lv, link)) {
                    lv->val_dhcp6addr.preferlifetime = 0;
                    lv->val_dhcp6addr.validlifetime = 0;
                }
            } else
                ev->state = DHCP6S_SOLICIT;
            dhcp6_set_timeoparam(ev);
        case DHCP6S_SOLICIT:
            if (ifp->servers) {
                ifp->current_server = select_server(ifp);
                if (ifp->current_server == NULL) {
                    /* this should not happen! */
                    dhcpv6_dprintf(LOG_ERR, "%s" "can't find a server",
                                   FNAME);
                    return (NULL);
                }
                /* if get the address assginment break */
                if (!TAILQ_EMPTY(&client6_iaidaddr.lease_list)) {
                    dhcp6_remove_event(ev);
                    return (NULL);
                }
                ev->timeouts = 0;
                ev->state = DHCP6S_REQUEST;
                dhcp6_set_timeoparam(ev);
            }
        case DHCP6S_INFOREQ:
        case DHCP6S_REQUEST:
            client6_send(ev);
            break;
        case DHCP6S_RELEASE:
        case DHCP6S_DECLINE:
        case DHCP6S_CONFIRM:
        case DHCP6S_RENEW:
        case DHCP6S_REBIND:
            if (!TAILQ_EMPTY(&request_list))
                client6_send(ev);
            else {
                dhcpv6_dprintf(LOG_INFO, "%s"
                               "all information to be updated were canceled",
                               FNAME);
                dhcp6_remove_event(ev);
                return (NULL);
            }
            break;
        default:
            break;
    }
    dhcp6_reset_timer(ev);
    return (ev->timer);
}

static struct dhcp6_serverinfo *select_server(ifp)
     struct dhcp6_if *ifp;
{
    struct dhcp6_serverinfo *s;

    /* 
     * pick the best server according to dhcpv6-26 Section 17.1.3
     * XXX: we currently just choose the one that is active and has the
     * highest preference.
     */
    for (s = ifp->servers; s; s = s->next) {
        if (s->active) {
            dhcpv6_dprintf(LOG_DEBUG, "%s" "picked a server (ID: %s)",
                           FNAME, duidstr(&s->optinfo.serverID));
            return (s);
        }
    }

    return (NULL);
}

#ifndef LIBDHCP
static void client6_signal(sig)
     int sig;
{

    dhcpv6_dprintf(LOG_INFO, FNAME "received a signal (%d)", sig);

    switch (sig) {
        case SIGTERM:
            sig_flags |= SIGF_TERM;
            break;
        case SIGHUP:
            sig_flags |= SIGF_HUP;
            break;
        case SIGINT:
        case SIGKILL:
            sig_flags |= SIGF_CLEAN;
            break;
        default:
            break;
    }
}
#endif

void client6_send(struct dhcp6_event *ev) {
    struct dhcp6_if *ifp;
    char buf[BUFSIZ];
    struct sockaddr_in6 dst;
    struct dhcp6 *dh6;
    struct dhcp6_optinfo optinfo;
    struct ia_listval *ia;
    ssize_t optlen, len;
    struct timeval duration, now;
    socklen_t salen;
#ifdef QNAP
		char qnap_duid_file[64];
#endif
    ifp = ev->ifp;

    dh6 = (struct dhcp6 *) buf;
    memset(dh6, 0, sizeof(*dh6));

    switch (ev->state) {
        case DHCP6S_SOLICIT:
            dh6->dh6_msgtype = DH6_SOLICIT;
            break;
        case DHCP6S_REQUEST:
            if (ifp->current_server == NULL) {
                dhcpv6_dprintf(LOG_ERR, "%s" "assumption failure", FNAME);
                return;
            }
            dh6->dh6_msgtype = DH6_REQUEST;
            break;
        case DHCP6S_RENEW:
            if (ifp->current_server == NULL) {
                dhcpv6_dprintf(LOG_ERR, "%s" "assumption failure", FNAME);
                return;
            }
            dh6->dh6_msgtype = DH6_RENEW;
            break;
        case DHCP6S_DECLINE:
            if (ifp->current_server == NULL) {
                dhcpv6_dprintf(LOG_ERR, "%s" "assumption failure", FNAME);
                return;
            }
            dh6->dh6_msgtype = DH6_DECLINE;
            break;
        case DHCP6S_INFOREQ:
            dh6->dh6_msgtype = DH6_INFORM_REQ;
            break;
        case DHCP6S_REBIND:
            dh6->dh6_msgtype = DH6_REBIND;
            break;
        case DHCP6S_CONFIRM:
            dh6->dh6_msgtype = DH6_CONFIRM;
            break;
        case DHCP6S_RELEASE:
            dh6->dh6_msgtype = DH6_RELEASE;
            break;
        default:
            dhcpv6_dprintf(LOG_ERR, "%s" "unexpected state %d", FNAME,
                           ev->state);
            return;
    }
    /* 
     * construct options
     */
    dhcp6_init_options(&optinfo);

    if ((ia = ia_create_listval()) == NULL) {
        goto end;
    }

    TAILQ_INSERT_TAIL(&optinfo.ia_list, ia, link);

    if (ev->timeouts == 0) {
        gettimeofday(&ev->start_time, NULL);
        optinfo.elapsed_time = 0;
        /* 
         * A client SHOULD generate a random number that cannot easily
         * be guessed or predicted to use as the transaction ID for
         * each new message it sends.
         *
         * A client MUST leave the transaction-ID unchanged in
         * retransmissions of a message. [dhcpv6-26 15.1]
         */
        ev->xid = random() & DH6_XIDMASK;
        dhcpv6_dprintf(LOG_DEBUG,
                       "%s" "ifp %p event %p a new XID (%x) is generated",
                       FNAME, ifp, ev, ev->xid);
    } else {
        unsigned int etime;

        gettimeofday(&now, NULL);
        timeval_sub(&now, &(ev->start_time), &duration);
        etime = (duration.tv_sec) * 100 + (duration.tv_usec) / 10000;
        if (etime > DHCP6_ELAPSEDTIME_MAX)
            etime = DHCP6_ELAPSEDTIME_MAX;
        optinfo.elapsed_time = htons((uint16_t) etime);
    }
    dh6->dh6_xid &= ~ntohl(DH6_XIDMASK);
    dh6->dh6_xid |= htonl(ev->xid);
    len = sizeof(*dh6);


    /* server ID */
    switch (ev->state) {
        case DHCP6S_REQUEST:
        case DHCP6S_RENEW:
        case DHCP6S_DECLINE:
            if (&ifp->current_server->optinfo == NULL)
                return;
            dhcpv6_dprintf(LOG_DEBUG, "current server ID %s",
                           duidstr(&ifp->current_server->optinfo.serverID));
            if (duidcpy(&optinfo.serverID,
                        &ifp->current_server->optinfo.serverID)) {
                dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy server ID",
                               FNAME);
                goto end;
            }
            break;
        case DHCP6S_RELEASE:
            if (duidcpy
                (&optinfo.serverID,
                 &client6_iaidaddr.client6_info.serverid)) {
                dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy server ID",
                               FNAME);
                goto end;
            }
            break;
    }
    /* client ID */
    if (duidcpy(&optinfo.clientID, &client_duid)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy client ID", FNAME);
        goto end;
    }

    /* save DUID now for persistent DUID (e.g., if client reboots) */
#ifdef QNAP
		snprintf(qnap_duid_file,sizeof(qnap_duid_file),"%s%s",DUID_FILE,device);
		if (save_duid(qnap_duid_file, device, &client_duid)) {
#else
    if (save_duid(DUID_FILE, device, &client_duid)) {
#endif
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to save client ID", FNAME);
        goto end;
    }

    /* option request options */
    if (dhcp6_copy_list(&optinfo.reqopt_list, &ifp->reqopt_list)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy requested options",
                       FNAME);
        goto end;
    }
    if (ifp->send_flags & DHCIFF_INFO_ONLY) {       /* RFC4242 */
        int opttype = DH6OPT_INFO_REFRESH_TIME;
        if (dhcp6_add_listval(&optinfo.reqopt_list, &opttype,
                              DHCP6_LISTVAL_NUM) == NULL) {
            dhcpv6_dprintf(LOG_ERR, "%s"
                           "failed to copy infomation refresh time option",
                           FNAME);
            goto end;
        }
    }

    switch (ev->state) {
        case DHCP6S_SOLICIT:
            /* rapid commit */
            if (ifp->send_flags & DHCIFF_RAPID_COMMIT) {
                optinfo.flags |= DHCIFF_RAPID_COMMIT;
            }

            if (!(ifp->send_flags & DHCIFF_INFO_ONLY) ||
                (client6_request_flag & CLIENT6_REQUEST_ADDR)) {
                memcpy(&ia->iaidinfo, &client6_iaidaddr.client6_info.iaidinfo,
                       sizeof(ia->iaidinfo));
                ia->type = iatype_of_if(ifp);
            }

            /* support for client preferred ipv6 address */
            if (client6_request_flag & CLIENT6_REQUEST_ADDR) {
                if (dhcp6_copy_list(&ia->addr_list, &request_list)) {
                    goto end;
                }
            }

            break;
        case DHCP6S_REQUEST:
            if (!(ifp->send_flags & DHCIFF_INFO_ONLY)) {
                memcpy(&ia->iaidinfo, &client6_iaidaddr.client6_info.iaidinfo,
                       sizeof(ia->iaidinfo));
                dhcpv6_dprintf(LOG_DEBUG, "%s IAID is %u", FNAME,
                               ia->iaidinfo.iaid);
                ia->type = iatype_of_if(ifp);
            }

            /*
             * Windows 2008 interoperability fix
             * If IA address is included in the DHCPv6 ADVERTISE (which is
             * what Windows 2008 does), put the IA address into the DHCPv6
             * REQUEST.  Windows 2008 will check for the IA address it has
             * given in the ADVERTISE, if it doesn't see it, the REQUEST
             * will be ignored).
             */
            if (!TAILQ_EMPTY(&request_list)) {
                dhcp6_copy_list(&ia->addr_list, &request_list);
            }

            break;
        case DHCP6S_RENEW:
        case DHCP6S_REBIND:
        case DHCP6S_RELEASE:
        case DHCP6S_CONFIRM:
        case DHCP6S_DECLINE:
            memcpy(&ia->iaidinfo, &client6_iaidaddr.client6_info.iaidinfo,
                   sizeof(ia->iaidinfo));
            ia->type = client6_iaidaddr.client6_info.type;

            if (ev->state == DHCP6S_CONFIRM) {
                ia->iaidinfo.renewtime = 0;
                ia->iaidinfo.rebindtime = 0;
            }

            if (!TAILQ_EMPTY(&request_list)) {
                /* XXX: ToDo: seperate to prefix list and address list */
                if (dhcp6_copy_list(&ia->addr_list, &request_list)) {
                    goto end;
                }
            } else {
                if (ev->state == DHCP6S_RELEASE) {
                    dhcpv6_dprintf(LOG_INFO, "release empty address list");
                    return;
                }
                /* XXX: allow the other emtpy list ?? */
            }

            if (client6_request_flag & CLIENT6_RELEASE_ADDR) {
#ifdef LIBDHCP
                if (libdhcp_control
                    && (libdhcp_control->
                        capability & DHCP_CONFIGURE_ADDRESSES))
#endif
                    if (dhcp6_update_iaidaddr(&optinfo, ia, ADDR_REMOVE)) {
                        dhcpv6_dprintf(LOG_INFO, "client release failed");
                        return;
                    }
#ifdef LIBDHCP
                if (libdhcp_control && libdhcp_control->callback)
                    (*(libdhcp_control->callback)) (libdhcp_control,
                                                    DHC6_RELEASE,
                                                    &client6_iaidaddr);
#endif
            }

            break;
        default:
            break;
    }

    /* set options in the message */
    if ((optlen = dhcp6_set_options((struct dhcp6opt *) (dh6 + 1),
                                    (struct dhcp6opt *) (buf + sizeof(buf)),
                                    &optinfo)) < 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "failed to construct options", FNAME);
        goto end;
    }

    len += optlen;

    /* 
     * Unless otherwise specified, a client sends DHCP messages to the
     * All_DHCP_Relay_Agents_and_Servers or the DHCP_Anycast address.
     * [dhcpv6-26 Section 13.]
     * Our current implementation always follows the case.
     */
    switch (ev->state) {
        case DHCP6S_REQUEST:
        case DHCP6S_RENEW:
        case DHCP6S_DECLINE:
        case DHCP6S_RELEASE:
            if (ifp->current_server &&
                !IN6_IS_ADDR_UNSPECIFIED(&ifp->current_server->server_addr)) {
                struct addrinfo hints, *res;
                int error;

                memset(&hints, 0, sizeof(hints));
                hints.ai_family = PF_INET6;
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_protocol = IPPROTO_UDP;
                error =
                    getaddrinfo(in6addr2str
                                (&ifp->current_server->server_addr, 0),
                                DH6PORT_UPSTREAM, &hints, &res);
                if (error) {
                    dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                                   FNAME, gai_strerror(error));
                    return;
                }
                memcpy(&dst, res->ai_addr, res->ai_addrlen);
                salen = res->ai_addrlen;
                break;
            }
        default:
            if (sa6_allagent != NULL) {
                dst = *sa6_allagent;
            }

            salen = sa6_alen;
            break;
    }

    dst.sin6_scope_id = ifp->linkid;
    dhcpv6_dprintf(LOG_DEBUG, "send dst if %s addr is %s scope id is %d",
                   ifp->ifname, addr2str((struct sockaddr *) &dst, salen),
                   ifp->linkid);
    if (sendto
        (ifp->outsock, buf, len, MSG_DONTROUTE, (struct sockaddr *) &dst,
         sizeof(dst)) == -1) {
        dhcpv6_dprintf(LOG_ERR, FNAME "transmit failed: %s", strerror(errno));
        goto end;
    }

    dhcpv6_dprintf(LOG_DEBUG, "%s" "send %s to %s", FNAME,
                   dhcp6msgstr(dh6->dh6_msgtype),
                   addr2str((struct sockaddr *) &dst, salen));

  end:
    dhcp6_clear_options(&optinfo);
    return;
}

static void client6_recv() {
    char rbuf[BUFSIZ], cmsgbuf[BUFSIZ];
    struct msghdr mhdr;
    struct iovec iov;
    struct sockaddr_storage from;
    struct dhcp6_if *ifp;
    struct dhcp6opt *p, *ep;
    struct dhcp6_optinfo optinfo;
    ssize_t len;
    struct dhcp6 *dh6;
    struct cmsghdr *cm;
    struct in6_pktinfo *pi = NULL;

    memset(&iov, 0, sizeof(iov));
    memset(&mhdr, 0, sizeof(mhdr));

    iov.iov_base = (caddr_t) rbuf;
    iov.iov_len = sizeof(rbuf);
    mhdr.msg_name = (caddr_t) & from;
    mhdr.msg_namelen = sizeof(from);
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = (caddr_t) cmsgbuf;
    mhdr.msg_controllen = sizeof(cmsgbuf);
    if ((len = recvmsg(iosock, &mhdr, 0)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "recvmsg: %s", FNAME, strerror(errno));
        return;
    }

    /* detect receiving interface */
    for (cm = (struct cmsghdr *) CMSG_FIRSTHDR(&mhdr); cm;
         cm = (struct cmsghdr *) CMSG_NXTHDR(&mhdr, cm)) {
        if (cm->cmsg_level == IPPROTO_IPV6 &&
            cm->cmsg_type == IPV6_PKTINFO &&
            cm->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo))) {
            pi = (struct in6_pktinfo *) (CMSG_DATA(cm));
        }
    }
    if (pi == NULL) {
        dhcpv6_dprintf(LOG_NOTICE, "%s" "failed to get packet info", FNAME);
        return;
    }
    if ((ifp = find_ifconfbyid(pi->ipi6_ifindex)) == NULL) {
        dhcpv6_dprintf(LOG_INFO, "%s" "unexpected interface (%d)", FNAME,
                       (unsigned int) pi->ipi6_ifindex);
        return;
    }
    dhcpv6_dprintf(LOG_DEBUG,
                   "receive packet info ifname %s, addr is %s scope id is %d",
                   ifp->ifname, in6addr2str(&pi->ipi6_addr, 0),
                   pi->ipi6_ifindex);
    dh6 = (struct dhcp6 *) rbuf;

    dhcpv6_dprintf(LOG_DEBUG, "%s" "receive %s from %s scope id %d %s", FNAME,
                   dhcp6msgstr(dh6->dh6_msgtype),
                   addr2str((struct sockaddr *) &from, sizeof(from)),
                   ((struct sockaddr_in6 *) &from)->sin6_scope_id,
                   ifp->ifname);

    /* get options */
    dhcp6_init_options(&optinfo);
    p = (struct dhcp6opt *) (dh6 + 1);
    ep = (struct dhcp6opt *) ((char *) dh6 + len);

    if (dhcp6_get_options(p, ep, &optinfo) < 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "failed to parse options", FNAME);
#ifdef TEST
        return;
#endif
    }

    switch (dh6->dh6_msgtype) {
        case DH6_ADVERTISE:
            (void) client6_recvadvert(ifp, dh6, len, &optinfo);
            break;
        case DH6_REPLY:
            (void) client6_recvreply(ifp, dh6, len, &optinfo);
            break;
        default:
            dhcpv6_dprintf(LOG_INFO,
                           "%s" "received an unexpected message (%s) "
                           "from %s", FNAME, dhcp6msgstr(dh6->dh6_msgtype),
                           addr2str((struct sockaddr *) &from, sizeof(from)));
            break;
    }

    dhcp6_clear_options(&optinfo);
    return;
}

static int client6_recvadvert(struct dhcp6_if *ifp, struct dhcp6 *dh6,
                              ssize_t len, struct dhcp6_optinfo *optinfo0) {
    struct ia_listval *ia;
    struct dhcp6_serverinfo *newserver;
    struct dhcp6_event *ev;
    struct dhcp6_listval *lv;

    /* find the corresponding event based on the received xid */
    ev = find_event_withid(ifp, ntohl(dh6->dh6_xid) & DH6_XIDMASK);
    if (ev == NULL) {
        dhcpv6_dprintf(LOG_INFO, "%s" "XID mismatch", FNAME);
        return -1;
    }

    /* if server policy doesn't allow rapid commit if (ev->state !=
       DHCP6S_SOLICIT || (ifp->send_flags & DHCIFF_RAPID_COMMIT)) { */
    if (ev->state != DHCP6S_SOLICIT) {
        dhcpv6_dprintf(LOG_INFO, "%s" "unexpected advertise", FNAME);
        return -1;
    }

    /* packet validation based on Section 15.3 of dhcpv6-26. */
    if (optinfo0->serverID.duid_len == 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "no server ID option", FNAME);
        return -1;
    } else {
        dhcpv6_dprintf(LOG_DEBUG, "%s" "server ID: %s, pref=%2x", FNAME,
                       duidstr(&optinfo0->serverID), optinfo0->pref);
    }

    if (optinfo0->clientID.duid_len == 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "no client ID option", FNAME);
        return -1;
    }

    if (duidcmp(&optinfo0->clientID, &client_duid)) {
        dhcpv6_dprintf(LOG_INFO, "%s" "client DUID mismatch", FNAME);
        return -1;
    }

    /* 
     * The client MUST ignore any Advertise message that includes a Status
     * Code option containing any error.
     */
    dhcpv6_dprintf(LOG_INFO, "%s" "status code: %s",
                   FNAME, dhcp6_stcodestr(optinfo0->status_code));
    if (optinfo0->status_code != DH6OPT_STCODE_SUCCESS &&
        optinfo0->status_code != DH6OPT_STCODE_UNDEFINE) {
        return -1;
    }

    /* ignore the server if it is known */
    if (find_server(ifp, &optinfo0->serverID)) {
        dhcpv6_dprintf(LOG_INFO, "%s" "duplicated server (ID: %s)",
                       FNAME, duidstr(&optinfo0->serverID));
        return -1;
    }

    newserver = allocate_newserver(ifp, optinfo0);

    if (newserver == NULL) {
        return -1;
    }

    /* if the server has an extremely high preference, just use it. */
    if (newserver->pref == DH6OPT_PREF_MAX) {
        ev->timeouts = 0;
        ev->state = DHCP6S_REQUEST;
        ifp->current_server = newserver;
        dhcp6_set_timeoparam(ev);
        dhcp6_reset_timer(ev);
        client6_send(ev);
    } else if (ifp->servers->next == NULL) {
        struct timeval *rest, elapsed, tv_rt, tv_irt, timo;

        /* 
         * If this is the first advertise, adjust the timer so that
         * the client can collect other servers until IRT elapses.
         * XXX: we did not want to do such "low level" timer
         *      calculation here.
         */
        rest = dhcp6_timer_rest(ev->timer);
        tv_rt.tv_sec = (ev->retrans * 1000) / 1000000;
        tv_rt.tv_usec = (ev->retrans * 1000) % 1000000;
        tv_irt.tv_sec = (ev->init_retrans * 1000) / 1000000;
        tv_irt.tv_usec = (ev->init_retrans * 1000) % 1000000;
        timeval_sub(&tv_rt, rest, &elapsed);
        if (TIMEVAL_LEQ(elapsed, tv_irt))
            timeval_sub(&tv_irt, &elapsed, &timo);
        else
            timo.tv_sec = timo.tv_usec = 0;

        dhcpv6_dprintf(LOG_DEBUG, "%s" "reset timer for %s to %d.%06d",
                       FNAME, ifp->ifname,
                       (int) timo.tv_sec, (int) timo.tv_usec);

        dhcp6_set_timer(&timo, ev->timer);
    }

    /* if the client send preferred addresses reqeust in SOLICIT */
    /* XXX: client might have some local policy to select the addresses */
    if ((ia = ia_find_listval(&optinfo0->ia_list,
        iatype_of_if(ifp), ifp->iaidinfo.iaid)) != NULL) {
#ifdef LIBDHCP
        if (!TAILQ_EMPTY(&(client6_iaidaddr.lease_list)))
            /* looks like we did a successful REBIND ? */
            if (libdhcp_control && libdhcp_control->callback) {
                (*(libdhcp_control->callback)) (libdhcp_control, DHC6_REBIND,
                                                optinfo0);
            }
#endif
        dhcp6_copy_list(&request_list, &ia->addr_list);
    }

    return 0;
}

static struct dhcp6_serverinfo *find_server(ifp, duid)
     struct dhcp6_if *ifp;
     struct duid *duid;
{
    struct dhcp6_serverinfo *s;

    for (s = ifp->servers; s; s = s->next) {
        if (duidcmp(&s->optinfo.serverID, duid) == 0)
            return (s);
    }

    return (NULL);
}

static struct dhcp6_serverinfo *allocate_newserver(ifp, optinfo)
     struct dhcp6_if *ifp;
     struct dhcp6_optinfo *optinfo;
{
    struct dhcp6_serverinfo *newserver, **sp;

    /* keep the server */
    if ((newserver = malloc(sizeof(*newserver))) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "memory allocation failed for server",
                       FNAME);
        return (NULL);
    }
    memset(newserver, 0, sizeof(*newserver));
    dhcp6_init_options(&newserver->optinfo);
    if (dhcp6_copy_options(&newserver->optinfo, optinfo)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy options", FNAME);
        free(newserver);
        return (NULL);
    }
    dhcpv6_dprintf(LOG_DEBUG, "%s" "new server DUID %s, len %d ",
                   FNAME, duidstr(&newserver->optinfo.serverID),
                   newserver->optinfo.serverID.duid_len);
    if (optinfo->pref != DH6OPT_PREF_UNDEF)
        newserver->pref = optinfo->pref;
    if (optinfo->flags & DHCIFF_UNICAST)
        memcpy(&newserver->server_addr, &optinfo->server_addr,
               sizeof(newserver->server_addr));
    newserver->active = 1;
    for (sp = &ifp->servers; *sp; sp = &(*sp)->next) {
        if ((*sp)->pref != DH6OPT_PREF_MAX && (*sp)->pref < newserver->pref) {
            break;
        }
    }
    newserver->next = *sp;
    *sp = newserver;
    return newserver;
}

void free_servers(ifp)
     struct dhcp6_if *ifp;
{
    struct dhcp6_serverinfo *sp, *sp_next;

    /* free all servers we've seen so far */
    for (sp = ifp->servers; sp; sp = sp_next) {
        sp_next = sp->next;
        dhcpv6_dprintf(LOG_DEBUG, "%s" "removing server (ID: %s)",
                       FNAME, duidstr(&sp->optinfo.serverID));
        dhcp6_clear_options(&sp->optinfo);
        free(sp);
    }
    ifp->servers = NULL;
    ifp->current_server = NULL;
}

static int client6_recvreply(struct dhcp6_if *ifp, struct dhcp6 *dh6, ssize_t len,
                             struct dhcp6_optinfo *optinfo) {
    struct ia_listval *ia;
    struct dhcp6_event *ev;
    struct dhcp6_serverinfo *newserver;
    int newstate = 0;
    int err = 0;

    /* find the corresponding event based on the received xid */
    dhcpv6_dprintf(LOG_DEBUG, "%s" "reply message XID is (%x)",
                   FNAME, ntohl(dh6->dh6_xid) & DH6_XIDMASK);
    ev = find_event_withid(ifp, ntohl(dh6->dh6_xid) & DH6_XIDMASK);

    if (ev == NULL) {
        dhcpv6_dprintf(LOG_INFO, "%s" "XID mismatch", FNAME);
        return -1;
    }

    if (!(DHCP6S_VALID_REPLY(ev->state)) &&
        (ev->state != DHCP6S_SOLICIT ||
         !(optinfo->flags & DHCIFF_RAPID_COMMIT))) {
        dhcpv6_dprintf(LOG_INFO, "%s" "unexpected reply", FNAME);
        return -1;
    }

    /* A Reply message must contain a Server ID option */
    if (optinfo->serverID.duid_len == 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "no server ID option", FNAME);
        return -1;
    }

    dhcpv6_dprintf(LOG_DEBUG, "%s" "serverID is %s len is %d", FNAME,
                   duidstr(&optinfo->serverID), optinfo->serverID.duid_len);

    /* get current server */
    switch (ev->state) {
        case DHCP6S_SOLICIT:
        case DHCP6S_CONFIRM:
        case DHCP6S_REBIND:
            newserver = allocate_newserver(ifp, optinfo);
            if (newserver == NULL)
                return (-1);
            ifp->current_server = newserver;
            duidcpy(&client6_iaidaddr.client6_info.serverid,
                    &ifp->current_server->optinfo.serverID);
            break;
        default:
            break;
    }

    /* 
     * DUID in the Client ID option (which must be contained for our
     * client implementation) must match ours.
     */
    if (optinfo->clientID.duid_len == 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "no client ID option", FNAME);
        return -1;
    }

    if (duidcmp(&optinfo->clientID, &client_duid)) {
        dhcpv6_dprintf(LOG_INFO, "%s" "client DUID mismatch", FNAME);
        return -1;
    }

    if (!TAILQ_EMPTY(&optinfo->dns_list.addrlist) ||
        optinfo->dns_list.domainlist != NULL) {
#ifdef LIBDHCP
        if (libdhcp_control
            && (libdhcp_control->capability & DHCP_CONFIGURE_RESOLVER))
#endif
            resolv_parse(&optinfo->dns_list);
    }

    /* 
     * The client MAY choose to report any status code or message from the
     * status code option in the Reply message.
     * [dhcpv6-26 Section 18.1.8]
     */
    if (optinfo->status_code != DH6OPT_STCODE_UNDEFINE) {
        dhcpv6_dprintf(LOG_INFO, "%s" "status code of message: %s",
                       FNAME, dhcp6_stcodestr(optinfo->status_code));
    }

    ia = ia_find_listval(&optinfo->ia_list,
                         iatype_of_if(ifp), ifp->iaidinfo.iaid);

    if (ia == NULL) {
        dhcpv6_dprintf(LOG_INFO, "%s" "no IA option", FNAME);
    } else if (ia->status_code != DH6OPT_STCODE_UNDEFINE) {
        dhcpv6_dprintf(LOG_INFO, "%s" "status code of IA: %s",
                       FNAME, dhcp6_stcodestr(ia->status_code));
    }

    switch (optinfo->status_code) {
        case DH6OPT_STCODE_UNSPECFAIL:
        case DH6OPT_STCODE_USEMULTICAST:
            /* retransmit the message with multicast address */
            /* how many time allow the retransmission with error status code? 
             */
            return -1;
    }

    /* 
     * Update configuration information to be renewed or rebound
     * declined, confirmed, released.
     * Note that the returned list may be empty, in which case
     * the waiting information should be removed.
     */
    switch (ev->state) {
        case DHCP6S_SOLICIT:
            if (ia == NULL) {
                break;
            } else if (!optinfo->flags & DHCIFF_RAPID_COMMIT) {
                newstate = DHCP6S_REQUEST;
                break;
            }
        case DHCP6S_REQUEST:
            if (ia == NULL) {
                break;
            }

            /* NotOnLink: 1. SOLICIT NoAddrAvail: Information Request */
            switch (ia->status_code) {
                case DH6OPT_STCODE_NOTONLINK:
                    dhcpv6_dprintf(LOG_DEBUG, "%s"
                                   "got a NotOnLink reply for request/rapid commit,"
                                   " sending solicit.", FNAME);
                    newstate = DHCP6S_SOLICIT;
                    break;
                case DH6OPT_STCODE_NOADDRAVAIL:
                case DH6OPT_STCODE_NOPREFIXAVAIL:
                    dhcpv6_dprintf(LOG_DEBUG, "%s"
                                   "got a NoAddrAvail reply for request/rapid commit,"
                                   " sending inforeq.", FNAME);
                    ia = NULL;
                    newstate = DHCP6S_INFOREQ;
                    break;
                case DH6OPT_STCODE_SUCCESS:
                case DH6OPT_STCODE_UNDEFINE:
                default:
                    if (!TAILQ_EMPTY(&ia->addr_list)) {
                        err = get_if_rainfo(ifp);
                        if (err) {
                            dhcpv6_dprintf(LOG_ERR,
                                "failed to get interface info via libnl: %d",
                                err);
                            return -1;
                        }

                        dhcp6_add_iaidaddr(optinfo, ia);

                        if (ifp->dad_timer == NULL
                            && (ifp->dad_timer =
                                dhcp6_add_timer(check_dad_timo, ifp)) < 0) {
                            dhcpv6_dprintf(LOG_INFO,
                                           "%s"
                                           "failed to create a timer for DAD",
                                           FNAME);
                        }

                        setup_check_timer(ifp);
#ifdef LIBDHCP
                        if (libdhcp_control && libdhcp_control->callback)
                            (*(libdhcp_control->callback)) (libdhcp_control,
                                                            DHC6_BOUND,
                                                            optinfo);
#endif
                    }

                    break;
            }

            break;
        case DHCP6S_RENEW:
        case DHCP6S_REBIND:
            if (ia == NULL) {
                newstate = ev->state;
                break;
            }

            if (client6_request_flag & CLIENT6_CONFIRM_ADDR) {
                goto rebind_confirm;
            }

            /* NoBinding for RENEW, REBIND, send REQUEST */
            switch (ia->status_code) {
                case DH6OPT_STCODE_NOBINDING:
                    newstate = DHCP6S_REQUEST;
                    dhcpv6_dprintf(LOG_DEBUG, "%s"
                                   "got a NoBinding reply, sending request.",
                                   FNAME);
                    dhcp6_remove_iaidaddr(&client6_iaidaddr);
#ifdef LIBDHCP
                    if (libdhcp_control && libdhcp_control->callback)
                        (*(libdhcp_control->callback)) (libdhcp_control,
                                                        DHC6_RELEASE,
                                                        &client6_iaidaddr);
#endif
                    break;
                case DH6OPT_STCODE_NOADDRAVAIL:
                case DH6OPT_STCODE_NOPREFIXAVAIL:
                case DH6OPT_STCODE_UNSPECFAIL:
                    break;
                case DH6OPT_STCODE_SUCCESS:
                case DH6OPT_STCODE_UNDEFINE:
                default:
                    dhcp6_update_iaidaddr(optinfo, ia, ADDR_UPDATE);
#ifdef LIBDHCP
                    if (libdhcp_control && libdhcp_control->callback)
                        (*(libdhcp_control->callback)) (libdhcp_control,
                                                        DHC6_REBIND, optinfo);
#endif
                    break;
            }

            break;
        case DHCP6S_CONFIRM:
            /* NOtOnLink for a Confirm, send SOLICIT message */
          rebind_confirm:client6_request_flag &=
                ~CLIENT6_CONFIRM_ADDR;
            switch (optinfo->status_code) {
                    struct timeb now;
                    struct timeval timo;
                    time_t offset;

                case DH6OPT_STCODE_NOTONLINK:
                case DH6OPT_STCODE_NOBINDING:
                case DH6OPT_STCODE_NOADDRAVAIL:
                    dhcpv6_dprintf(LOG_DEBUG, "%s"
                                   "got a NotOnLink reply for confirm, sending solicit.",
                                   FNAME);
                    /* remove event data list */
                    free_servers(ifp);
                    /* remove the address which is judged NotOnLink */
                    dhcp6_remove_iaidaddr(&client6_iaidaddr);
#ifdef LIBDHCP
                    if (libdhcp_control && libdhcp_control->callback)
                        (*(libdhcp_control->callback)) (libdhcp_control, DHC6_RELEASE, &client6_iaidaddr);
#endif
                    newstate = DHCP6S_SOLICIT;
                    break;
                case DH6OPT_STCODE_SUCCESS:
                case DH6OPT_STCODE_UNDEFINE:
                    /* XXX: set up renew/rebind timer */
                    dhcpv6_dprintf(LOG_DEBUG,
                                   "%s" "got an expected reply for confirm",
                                   FNAME);
                    ftime(&now);
                    client6_iaidaddr.state = ACTIVE;

                    if ((client6_iaidaddr.timer =
                         dhcp6_add_timer(dhcp6_iaidaddr_timo,
                                         &client6_iaidaddr)) == NULL) {
                        dhcpv6_dprintf(LOG_ERR,
                                       "%s"
                                       "failed to add a timer for iaid %u",
                                       FNAME,
                                       client6_iaidaddr.client6_info.iaidinfo.
                                       iaid);
                        return (-1);
                    }

                    if (client6_iaidaddr.client6_info.iaidinfo.renewtime == 0) {
                        client6_iaidaddr.client6_info.iaidinfo.renewtime
                            = get_min_preferlifetime(&client6_iaidaddr) / 2;
                    }

                    if (client6_iaidaddr.client6_info.iaidinfo.rebindtime ==
                        0) {
                        client6_iaidaddr.client6_info.iaidinfo.rebindtime =
                            (get_min_preferlifetime(&client6_iaidaddr) * 4) /
                            5;
                    }

                    offset = now.time - client6_iaidaddr.start_date;

                    if (offset >
                        client6_iaidaddr.client6_info.iaidinfo.renewtime)
                        timo.tv_sec = 0;
                    else
                        timo.tv_sec =
                            client6_iaidaddr.client6_info.iaidinfo.renewtime -
                            offset;

                    timo.tv_usec = 0;
                    dhcp6_set_timer(&timo, client6_iaidaddr.timer);

                    /* check DAD */
                    if (client6_iaidaddr.client6_info.type != IAPD &&
			ifp->dad_timer == NULL &&
                        (ifp->dad_timer =
                         dhcp6_add_timer(check_dad_timo, ifp)) < 0) {
                        dhcpv6_dprintf(LOG_INFO,
                                       "%s" "failed to create a timer for "
                                       " DAD", FNAME);
                    }

                    setup_check_timer(ifp);

                    break;
                default:
                    break;
            }

            break;
        case DHCP6S_DECLINE:
            /* send REQUEST message to server with none decline address */
            dhcpv6_dprintf(LOG_DEBUG, "%s"
                           "got an expected reply for decline, sending request.",
                           FNAME);
            create_request_list(0);

            /* remove event data list */
            newstate = DHCP6S_REQUEST;
            break;
        case DHCP6S_RELEASE:
            dhcpv6_dprintf(LOG_INFO, "%s" "got an expected release, exit.",
                           FNAME);
            dhcp6_remove_event(ev);
            exit(0);
        case DHCP6S_INFOREQ:
            set_info_refresh_timer(ifp, optinfo->irt);
            break;
        default:
            break;
    }

    dhcp6_remove_event(ev);

    if (newstate) {
        client6_send_newstate(ifp, newstate);
    } else {
        dhcpv6_dprintf(LOG_DEBUG, "%s" "got an expected reply, sleeping.",
                       FNAME);
    }

    dhcp6_clear_list(&request_list);
    TAILQ_INIT(&request_list);
    return 0;
}

static int set_info_refresh_timer(struct dhcp6_if *ifp, u_int32_t offered_irt) {
    int irt;
    struct timeval timo;
    double rval;

    if (offered_irt == 0) {
        irt = ifp->default_irt;
    } else if (offered_irt < IRT_MINIMUM) {
        irt = IRT_MINIMUM;
    } else if (offered_irt > ifp->maximum_irt) {
        irt = ifp->maximum_irt;
    } else {
        irt = offered_irt;
    }

    if (irt == DHCP6_DURATITION_INFINITE) {
        dhcpv6_dprintf(LOG_DEBUG, "%s"
                       "information would not be refreshed any more", FNAME);
        return 0;
    }

    if ((ifp->info_refresh_timer =
                dhcp6_add_timer(info_refresh_timo, ifp)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a timer for %s",
                       FNAME, ifp->ifname);
        return -1;
    }

    /*
     * the client MUST delay sending the first Information-Request by
     * a random amount of time between 0 and INF_MAX_DELAY
     * [RFC4242 3.2.]
     */
    rval = (double)random() / RAND_MAX * INF_MAX_DELAY * 1000;
    timo.tv_sec = irt + (long)(rval / 1000000);
    timo.tv_usec = (long)rval % 1000000;
    dhcp6_set_timer(&timo, ifp->info_refresh_timer);
    dhcpv6_dprintf(LOG_DEBUG, "%s"
                   "information will be refreshed in %ld.%06ld [sec]",
                   FNAME, timo.tv_sec, timo.tv_usec);

    return 0;
}

static struct dhcp6_timer *info_refresh_timo(void *arg) {
    struct dhcp6_if *ifp = (struct dhcp6_if *)arg;

    dhcpv6_dprintf(LOG_DEBUG, "%s" "information is refreshing...", FNAME);
    dhcp6_remove_timer(ifp->info_refresh_timer);
    ifp->info_refresh_timer = NULL;
    client6_send_newstate(ifp, DHCP6S_INFOREQ);
    return NULL;
}

int client6_send_newstate(struct dhcp6_if *ifp, int state) {
    struct dhcp6_event *ev;

    if ((ev = dhcp6_create_event(ifp, state)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to create an event", FNAME);
        return (-1);
    }

    if ((ev->timer = dhcp6_add_timer(client6_timo, ev)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to add a timer for %s",
                       FNAME, ifp->ifname);
        free(ev);
        return (-1);
    }

    TAILQ_INSERT_TAIL(&ifp->event_list, ev, link);
    ev->timeouts = 0;
    dhcp6_set_timeoparam(ev);
    dhcp6_reset_timer(ev);
    client6_send(ev);

    return 0;
}

static struct dhcp6_event *find_event_withid(ifp, xid)
     struct dhcp6_if *ifp;
     u_int32_t xid;
{
    struct dhcp6_event *ev;

    for (ev = TAILQ_FIRST(&ifp->event_list); ev; ev = TAILQ_NEXT(ev, link)) {
        dhcpv6_dprintf(LOG_DEBUG, "%s" "ifp %p event %p id is %x",
                       FNAME, ifp, ev, ev->xid);
        if (ev->xid == xid)
            return (ev);
    }

    return (NULL);
}

static int create_request_list(int reboot) {
    struct dhcp6_lease *cl;
    struct dhcp6_listval *lv;

    /* create an address list for release all/confirm */
    for (cl = TAILQ_FIRST(&client6_iaidaddr.lease_list); cl;
         cl = TAILQ_NEXT(cl, link)) {
        /* IANA, IAPD */
        if ((lv = malloc(sizeof(*lv))) == NULL) {
            dhcpv6_dprintf(LOG_ERR, "%s"
                           "failed to allocate memory for an ipv6 addr",
                           FNAME);
            exit(1);
        }
        memcpy(&lv->val_dhcp6addr, &cl->lease_addr,
               sizeof(lv->val_dhcp6addr));
        lv->val_dhcp6addr.status_code = DH6OPT_STCODE_UNDEFINE;
        TAILQ_INSERT_TAIL(&request_list, lv, link);
        /* config the interface for reboot */
        if (reboot && client6_iaidaddr.client6_info.type != IAPD &&
            (client6_request_flag & CLIENT6_CONFIRM_ADDR)) {
#ifdef LIBDHCP
            if (libdhcp_control
                && (libdhcp_control->capability & DHCP_CONFIGURE_ADDRESSES))
#endif
                if (client6_ifaddrconf(IFADDRCONF_ADD, &cl->lease_addr) != 0) {
                    dhcpv6_dprintf(LOG_INFO, "config address failed: %s",
                                   in6addr2str(&cl->lease_addr.addr, 0));
                    return (-1);
                }
        }
    }

    return (0);
}

static void setup_check_timer(struct dhcp6_if *ifp) {
    double d;
    struct timeval timo;

    d = DHCP6_CHECKLINK_TIME_UPCASE;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcpv6_dprintf(LOG_DEBUG, "set timer for checking link ...");
    dhcp6_set_timer(&timo, ifp->link_timer);
    if (ifp->dad_timer != NULL) {
        d = DHCP6_CHECKDAD_TIME;
        timo.tv_sec = (long) d;
        timo.tv_usec = 0;
        dhcpv6_dprintf(LOG_DEBUG, "set timer for checking DAD ...");
        dhcp6_set_timer(&timo, ifp->dad_timer);
    }
    d = DHCP6_SYNCFILE_TIME;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcpv6_dprintf(LOG_DEBUG, "set timer for syncing file ...");
    dhcp6_set_timer(&timo, ifp->sync_timer);
    return;
}

static struct dhcp6_timer
 *check_lease_file_timo(void *arg) {
    struct dhcp6_if *ifp = (struct dhcp6_if *) arg;
    double d;
    struct timeval timo;
    struct stat buf;
    FILE *file;

    stat(leasename, &buf);
    strcpy(client6_lease_temp, leasename);
    strcat(client6_lease_temp, "XXXXXX");
    if (buf.st_size > MAX_FILE_SIZE) {
        file = sync_leases(client6_lease_file, leasename, client6_lease_temp);
        if (file != NULL)
            client6_lease_file = file;
    }
    d = DHCP6_SYNCFILE_TIME;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcp6_set_timer(&timo, ifp->sync_timer);
    return ifp->sync_timer;
}

static struct dhcp6_timer
 *check_dad_timo(void *arg) {
    struct dhcp6_if *ifp = (struct dhcp6_if *) arg;
    int newstate;
    struct dhcp6_list dad_list;
    struct dhcp6_lease *cl;
    struct dhcp6_listval *lv;

    if (client6_iaidaddr.client6_info.type == IAPD)
        goto end;
    dhcpv6_dprintf(LOG_DEBUG, "enter checking dad ...");

    TAILQ_INIT(&dad_list);
    if (dad_parse("/proc/net/if_inet6", &dad_list) < 0) {
        dhcpv6_dprintf(LOG_ERR, "parse /proc/net/if_inet6 failed");
        goto end;
    }
    for (lv = TAILQ_FIRST(&dad_list); lv; lv = TAILQ_NEXT(lv, link)) {
        for (cl = TAILQ_FIRST(&client6_iaidaddr.lease_list);
             cl; cl = TAILQ_NEXT(cl, link)) {
            if (cl->lease_addr.type != IAPD &&
                IN6_ARE_ADDR_EQUAL(&cl->lease_addr.addr,
                                   &lv->val_dhcp6addr.addr)) {
                /* deconfigure the interface's the address assgined by dhcpv6 */
                if (dhcp6_remove_lease(cl) != 0) {
                    dprintf(LOG_ERR,
                            "remove duplicated address failed: %s",
                            in6addr2str(&cl->lease_addr.addr, 0));
                } else {
                    TAILQ_REMOVE(&dad_list, lv, link);
                    TAILQ_INSERT_TAIL(&request_list, lv, link);
                }
                break;
            }
        }
    }
    dhcp6_clear_list(&dad_list);

    if (TAILQ_EMPTY(&request_list))
        goto end;
    /* remove RENEW timer for client6_iaidaddr */
    if (client6_iaidaddr.timer != NULL)
        dhcp6_remove_timer(client6_iaidaddr.timer);
    newstate = DHCP6S_DECLINE;
    client6_send_newstate(ifp, newstate);
  end:
    /* one time check for DAD */
    dhcp6_remove_timer(ifp->dad_timer);
    ifp->dad_timer = NULL;
    return NULL;
}

static struct dhcp6_timer
 *check_link_timo(void *arg) {
    struct dhcp6_if *ifp = (struct dhcp6_if *) arg;
    struct ifreq ifr;
    struct timeval timo;
    static long d = DHCP6_CHECKLINK_TIME_UPCASE;
    int newstate;
    struct dhcp6_list dad_list;
    struct dhcp6_listval *lv;

    dhcpv6_dprintf(LOG_DEBUG, "enter checking link ...");
    strncpy(ifr.ifr_name, dhcp6_if->ifname, IFNAMSIZ);
    if (ioctl(nlsock, SIOCGIFFLAGS, &ifr) < 0) {
        dhcpv6_dprintf(LOG_DEBUG, "ioctl SIOCGIFFLAGS failed");
        goto settimer;
    }
    if (ifr.ifr_flags & IFF_RUNNING) {
        /* check previous flag set current flag UP */
        if (ifp->link_flag & IFF_RUNNING) {
            goto settimer;
        }
        switch (client6_iaidaddr.client6_info.type) {
            case IAPD:
                newstate = DHCP6S_REBIND;
                break;
            default:
                /* check DAD status of the link-local address */
                TAILQ_INIT(&dad_list);
                if (dad_parse("/proc/net/if_inet6", &dad_list) < 0) {
                    dhcpv6_dprintf(LOG_ERR, "parse /proc/net/if_inet6 failed");
                    goto settimer;
                }
                for (lv = TAILQ_FIRST(&dad_list);
                     lv; lv = TAILQ_NEXT(lv, link)) {
                    if (IN6_ARE_ADDR_EQUAL(&lv->val_dhcp6addr.addr,
                                           &ifp->linklocal)) {
                        dprintf(LOG_DEBUG, "wait for the DAD completion");
                        dhcp6_clear_list(&dad_list);
                        goto settimer;
                    }
                }
                dhcp6_clear_list(&dad_list);
                newstate = DHCP6S_CONFIRM;
                break;
        }

        /* check current state ACTIVE */
        if (client6_iaidaddr.state == ACTIVE) {
            /* remove timer for renew/rebind send confirm for ipv6address or
               rebind for prefix delegation */
            dhcp6_remove_timer(client6_iaidaddr.timer);
            client6_request_flag |= CLIENT6_CONFIRM_ADDR;
            create_request_list(1);
            client6_send_newstate(ifp, newstate);
        }
        dhcpv6_dprintf(LOG_INFO, "interface is from down to up");
        ifp->link_flag |= IFF_RUNNING;
        d = DHCP6_CHECKLINK_TIME_UPCASE;
    } else {
        dhcpv6_dprintf(LOG_INFO, "interface is down");
        /* set flag_prev flag DOWN */
        ifp->link_flag &= ~IFF_RUNNING;
        d = DHCP6_CHECKLINK_TIME_DOWNCASE;
    }
  settimer:
    timo.tv_sec = d;
    timo.tv_usec = 0;
    dhcp6_set_timer(&timo, ifp->link_timer);
    return ifp->link_timer;
}

static void setup_interface(char *ifname) {
    struct ifreq ifr;
    int retries = 0;

    /* check the interface */

    /* open a socket to watch the off-on link for confirm messages */
    if ((nlsock == -1) && ((nlsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "open a socket: %s", FNAME,
                       strerror(errno));
        return;
    }

    memset(&ifr, '\0', sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    if (ioctl(nlsock, SIOCGIFFLAGS, &ifr) < 0) {
        dhcpv6_dprintf(LOG_ERR, "ioctl SIOCGIFFLAGS failed");
        return;
    }

    while ((ifr.ifr_flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING)) {
        if (retries++ > 1) {
            dhcpv6_dprintf(LOG_INFO,
                           "NIC is not connected to the network, please connect it.");
            return;
        }

        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
        if (ioctl(nlsock, SIOCSIFFLAGS, &ifr) < 0) {
            dhcpv6_dprintf(LOG_ERR, "ioctl SIOCSIFFLAGS failed");
            return;
        }

        /* 
         * give kernel time to assign link local address and to find/respond
         * to IPv6 routers...
         */
        sleep(2);

        memset(&ifr, '\0', sizeof(struct ifreq));
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
        if (ioctl(nlsock, SIOCGIFFLAGS, &ifr) < 0) {
            dhcpv6_dprintf(LOG_ERR, "ioctl SIOCGIFFLAGS failed");
            return;
        }
    }

    return;
}

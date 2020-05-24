/* ported from KAME: dhcp6s.c,v 1.91 2002/09/24 14:20:50 itojun Exp */

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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/param.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# include <time.h>
#endif

#ifdef HAVE_LINUX_SOCKIOS_H
# include <linux/sockios.h>
#endif

#include "dhcp6.h"
#include "cfg.h"
#include "common.h"
#include "lease.h"
#include "server6_conf.h"
#include "timer.h"

typedef enum { DHCP6_CONFINFO_PREFIX, DHCP6_CONFINFO_ADDRS } dhcp6_conftype_t;

struct dhcp6_binding {
    TAILQ_ENTRY(dhcp6_binding) link;

    dhcp6_conftype_t type;
    struct duid clientid;
    void *val;

    u_int32_t duration;
    struct dhcp6_timer *timer;
};

static char pidfile[MAXPATHLEN];
static char *device[MAX_DEVICE];
static int num_device = 0;
static int debug = 0;
const dhcp6_mode_t dhcp6_mode = DHCP6_MODE_SERVER;
int iosock = -1;                /* inbound/outbound udp port */
extern FILE *server6_lease_file;
char server6_lease_temp[100];

static const struct sockaddr_in6 *sa6_any_downstream;
static u_int16_t upstream_port;
static struct msghdr rmh;
static char rdatabuf[BUFSIZ];
static int rmsgctllen;
static char *rmsgctlbuf;
static struct duid server_duid;
static struct dns_list arg_dnslist;
static struct dhcp6_timer *sync_lease_timer;

struct link_decl *subnet = NULL;
struct host_decl *host = NULL;
struct rootgroup *globalgroup = NULL;

#define DUID_FILE "/var/lib/dhcpv6/dhcp6s_duid"
#define DHCP6S_CONF "/etc/dhcp6s.conf"
#define DHCP6S_PIDFILE "/var/run/dhcpv6/dhcp6s.pid"

#define DH6_VALID_MESSAGE(a) \
	(a == DH6_SOLICIT || a == DH6_REQUEST || a == DH6_RENEW || \
	 a == DH6_REBIND || a == DH6_CONFIRM || a == DH6_RELEASE || \
	 a == DH6_DECLINE || a == DH6_INFORM_REQ)

static void usage __P((char *name));
static void server6_init __P((void));
static void server6_mainloop __P((void));
static int server6_recv __P((int));
static int handle_addr_request __P((struct dhcp6_optinfo *,
                                    struct ia_list *, struct ia_list *,
                                    int, int *));
static int update_binding_ia __P((struct duid *clientID,
                                  struct ia_list *ria_list,
                                  struct ia_list *ia_list,
                                  u_int8_t msgtype, int addr_flag,
                                  int *status_code));
static int server6_react_message __P((struct dhcp6_if *,
                                      struct in6_pktinfo *, struct dhcp6 *,
                                      struct dhcp6_optinfo *,
                                      struct sockaddr *, int));
static int server6_send __P((int, struct dhcp6_if *, struct dhcp6 *,
                             struct dhcp6_optinfo *,
                             struct sockaddr *, int, struct dhcp6_optinfo *));
static struct dhcp6_timer *check_lease_file_timo __P((void *arg));
static struct dhcp6 *dhcp6_parse_relay __P((struct dhcp6_relay *,
                                            struct dhcp6_relay *,
                                            struct dhcp6_optinfo *,
                                            struct in6_addr *));
static int dhcp6_set_relay __P((struct dhcp6_relay *,
                                struct dhcp6_relay *,
                                struct dhcp6_optinfo *));
static void dhcp6_set_relay_option_len __P((struct dhcp6_optinfo *, int len));
extern struct link_decl *dhcp6_allocate_link
__P((struct dhcp6_if *, struct rootgroup *, struct in6_addr *));
extern struct host_decl *dhcp6_allocate_host
__P((struct dhcp6_if *, struct rootgroup *, struct dhcp6_optinfo *));

extern int dhcp6_get_hostconf
__P((struct ia_listval *, struct ia_listval *, struct dhcp6_iaidaddr *,
     struct host_decl *));

static void random_init(void) {
    int f, n;
    unsigned int seed = time(NULL) & getpid();
    char rand_state[256];

    f = open("/dev/urandom", O_RDONLY);
    if (f > 0) {
        n = read(f, rand_state, sizeof(rand_state));
        close(f);
        if (n > 32) {
            initstate(seed, rand_state, n);
            return;
        }
    }
    srandom(seed);
}

static void server6_sighandler(int sig) {
    dhcpv6_dprintf(LOG_INFO, FNAME "received a signal (%d)", sig);

    switch (sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGINT:
            dhcpv6_dprintf(LOG_INFO, FNAME "exiting");
            unlink(pidfile);
            exit(0);
            break;
        default:
            break;
    }
}

int main(int argc, char **argv) {
    int ch;
    char *progname, *conffile = DHCP6S_CONF;
    FILE *pidfp = NULL;
    struct interface *ifnetwork;

    if ((progname = strrchr(*argv, '/')) == NULL) {
        progname = *argv;
    } else {
        progname++;
    }

    strcpy(pidfile, DHCP6S_PIDFILE);

    TAILQ_INIT(&arg_dnslist.addrlist);

    random_init();
    while ((ch = getopt(argc, argv, "c:vfn:p:")) != -1) {
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

    while (optind < argc) {
        device[num_device] = argv[optind++];
        num_device += 1;
    }

    if (foreground == 0) {
        if (daemon(0, 0) < 0) {
            err(1, "daemon");
        }

        openlog(progname, LOG_NDELAY | LOG_PID, LOG_DAEMON);
    }

    setloglevel(debug);
    server6_init();

    if ((server6_lease_file = init_leases(PATH_SERVER6_LEASE)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to parse lease file", FNAME);
        exit(1);
    }

    strcpy(server6_lease_temp, PATH_SERVER6_LEASE);
    strcat(server6_lease_temp, "XXXXXX");
    server6_lease_file =
        sync_leases(server6_lease_file, PATH_SERVER6_LEASE,
                    server6_lease_temp);

    if (server6_lease_file == NULL) {
        exit(1);
    }

    globalgroup = (struct rootgroup *) malloc(sizeof(struct rootgroup));
    if (globalgroup == NULL) {
        dhcpv6_dprintf(LOG_ERR, "failed to allocate memory %s",
                       strerror(errno));
        exit(1);
    }

    memset(globalgroup, 0, sizeof(*globalgroup));
    TAILQ_INIT(&globalgroup->scope.dnslist.addrlist);

    if ((sfparse(conffile)) != 0) {
        dhcpv6_dprintf(LOG_ERR,
                       "%s" "failed to parse addr configuration file", FNAME);
        exit(1);
    }

    ifnetwork = globalgroup->iflist;
    for (ifnetwork = globalgroup->iflist; ifnetwork;
         ifnetwork = ifnetwork->next) {
        if(ifnetwork->linklist == NULL) {
            /* If there was no link defined in the conf file, make an empty one. */
            ifnetwork->linklist = (struct link_decl *)malloc(sizeof(*subnet));
            if (ifnetwork->linklist == NULL) {
                dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
                exit(1);
            }
            memset(ifnetwork->linklist, 0, sizeof(*ifnetwork->linklist));
            TAILQ_INIT(&ifnetwork->linklist->linkscope.dnslist.addrlist);
        }
    }

    if (signal(SIGHUP, server6_sighandler) == SIG_ERR) {
        dhcpv6_dprintf(LOG_WARNING, "%s" "failed to set signal: %s",
                       FNAME, strerror(errno));
        return -1;
    }

    if (signal(SIGTERM, server6_sighandler) == SIG_ERR) {
        dhcpv6_dprintf(LOG_WARNING, "%s" "failed to set signal: %s",
                       FNAME, strerror(errno));
        return -1;
    }

    if (signal(SIGINT, server6_sighandler) == SIG_ERR) {
        dhcpv6_dprintf(LOG_WARNING, "%s" "failed to set signal: %s",
                       FNAME, strerror(errno));
        return -1;
    }

    /* dump current PID */
    if ((pidfp = fopen(pidfile, "w")) != NULL) {
        fprintf(pidfp, "%d\n", getpid());
        fclose(pidfp);
    }

    server6_mainloop();
    exit(0);
}

static void usage(char *name) {
    fprintf(stderr, "Usage: %s [options] [interface]\n", basename(name));
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -c PATH        Configuration file (e.g., /etc/dhcp6s.conf\n");
    fprintf(stderr, "    -p PATH        PID file name (default: %s)\n", DHCP6S_PIDFILE);
    fprintf(stderr, "    -v             Verbose debugging output\n");
    fprintf(stderr, "    -f             Run server as a foreground process\n");
    fflush(stderr);
    return;
}

void server6_init() {
    struct addrinfo hints;
    struct addrinfo *res, *res2;
    int error, skfd, i;
    int on = 1;
    int ifidx[MAX_DEVICE];
    struct ipv6_mreq mreq6;
    static struct iovec iov;
    static struct sockaddr_in6 sa6_any_downstream_storage;
    char buff[1024];
    struct ifconf ifc;
    struct ifreq *ifr;
    double d;
    struct timeval timo;

    /* initialize socket for inbound packets */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;
    error = getaddrinfo(NULL, DH6PORT_UPSTREAM, &hints, &res);
    if (error) {
        dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                       FNAME, gai_strerror(error));
        exit(1);
    }
    iosock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (iosock < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "socket: %s", FNAME, strerror(errno));
        exit(1);
    }
#ifdef IPV6_RECVPKTINFO
    if (setsockopt(iosock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on,
                   sizeof(on)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s"
                       "setsockopt(inbound, IPV6_RECVPKTINFO): %s",
                       FNAME, strerror(errno));
        exit(1);
    }
#else
    if (setsockopt(iosock, IPPROTO_IPV6, IPV6_PKTINFO, &on, sizeof(on)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s"
                       "setsockopt(inbound, IPV6_PKTINFO): %s",
                       FNAME, strerror(errno));
        exit(1);
    }
#endif
    if (bind(iosock, res->ai_addr, res->ai_addrlen) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "bind: %s", FNAME, strerror(errno));
        exit(1);
    }
    upstream_port = ((struct sockaddr_in6 *) res->ai_addr)->sin6_port;
    freeaddrinfo(res);

    /* initiallize socket address structure for outbound packets */
    hints.ai_flags = AI_PASSIVE;
    error = getaddrinfo(NULL, DH6PORT_DOWNSTREAM, &hints, &res);
    if (error) {
        dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                       FNAME, gai_strerror(error));
        exit(1);
    }
    memcpy(&sa6_any_downstream_storage, res->ai_addr, res->ai_addrlen);
    sa6_any_downstream =
        (const struct sockaddr_in6 *) &sa6_any_downstream_storage;
    freeaddrinfo(res);

    /* initialize send/receive buffer */
    iov.iov_base = (caddr_t) rdatabuf;
    iov.iov_len = sizeof(rdatabuf);
    rmh.msg_iov = &iov;
    rmh.msg_iovlen = 1;
    rmsgctllen = CMSG_SPACE(sizeof(struct in6_pktinfo));
    if ((rmsgctlbuf = (char *) malloc(rmsgctllen)) == NULL) {
        dhcpv6_dprintf(LOG_ERR, "%s" "memory allocation failed", FNAME);
        exit(1);
    }
    if (num_device != 0) {
        for (i = 0; i < num_device; i++) {
            ifidx[i] = if_nametoindex(device[i]);
            if (ifidx[i] == 0) {
                dhcpv6_dprintf(LOG_ERR, "%s"
                               "invalid interface %s", FNAME, device[0]);
                exit(1);
            }
            ifinit(device[i]);
        }
        if (get_duid(DUID_FILE, device[0], &server_duid)) {
            dhcpv6_dprintf(LOG_ERR, "%s" "failed to get a DUID", FNAME);
            exit(1);
        }
        if (save_duid(DUID_FILE, device[0], &server_duid)) {
            dhcpv6_dprintf(LOG_ERR, "%s" "failed to save server ID", FNAME);
        }
    } else {
        /* all the interfaces join multicast group */
        ifc.ifc_len = sizeof(buff);
        ifc.ifc_buf = buff;
        if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            dhcpv6_dprintf(LOG_ERR, "new socket failed");
            exit(1);
        }
        if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
            dhcpv6_dprintf(LOG_ERR, "SIOCGIFCONF: %s\n", strerror(errno));
            exit(1);
        }
        ifr = ifc.ifc_req;
        for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; ifr++) {
            dhcpv6_dprintf(LOG_DEBUG, "found device %s", ifr->ifr_name);
            ifidx[num_device] = if_nametoindex(ifr->ifr_name);
            if (ifidx[num_device] < 0) {
                dhcpv6_dprintf(LOG_ERR, "%s: unknown interface.\n",
                               ifr->ifr_name);
                continue;
            }
            dhcpv6_dprintf(LOG_DEBUG, "if %s index is %d", ifr->ifr_name,
                           ifidx[num_device]);
#ifdef mshirley
            if (((ifr->ifr_flags & IFF_UP) == 0))
                continue;
#endif
            if (strcmp(ifr->ifr_name, "lo")) {
                /* get our DUID */
                if (get_duid(DUID_FILE, ifr->ifr_name, &server_duid)) {
                    dhcpv6_dprintf(LOG_ERR, "%s" "failed to get a DUID",
                                   FNAME);
                    exit(1);
                }
                if (save_duid(DUID_FILE, ifr->ifr_name, &server_duid)) {
                    dhcpv6_dprintf(LOG_ERR, "%s" "failed to save server ID",
                                   FNAME);
                }
            }
            ifinit(ifr->ifr_name);
            num_device += 1;
        }
    }
    for (i = 0; i < num_device; i++) {
        hints.ai_flags = 0;
        error =
            getaddrinfo(DH6ADDR_ALLAGENT, DH6PORT_UPSTREAM, &hints, &res2);
        if (error) {
            dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                           FNAME, gai_strerror(error));
            exit(1);
        }
        memset(&mreq6, 0, sizeof(mreq6));
        mreq6.ipv6mr_interface = ifidx[i];
        memcpy(&mreq6.ipv6mr_multiaddr,
               &((struct sockaddr_in6 *) res2->ai_addr)->sin6_addr,
               sizeof(mreq6.ipv6mr_multiaddr));
        if (setsockopt(iosock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                       &mreq6, sizeof(mreq6))) {
            dhcpv6_dprintf(LOG_ERR,
                           "%s" "setsockopt(iosock, IPV6_JOIN_GROUP) %s",
                           FNAME, strerror(errno));
            exit(1);
        }
        freeaddrinfo(res2);

        hints.ai_flags = 0;
        error = getaddrinfo(DH6ADDR_ALLSERVER, DH6PORT_UPSTREAM,
                            &hints, &res2);
        if (error) {
            dhcpv6_dprintf(LOG_ERR, "%s" "getaddrinfo: %s",
                           FNAME, gai_strerror(error));
            exit(1);
        }
        memset(&mreq6, 0, sizeof(mreq6));
        mreq6.ipv6mr_interface = ifidx[i];
        memcpy(&mreq6.ipv6mr_multiaddr,
               &((struct sockaddr_in6 *) res2->ai_addr)->sin6_addr,
               sizeof(mreq6.ipv6mr_multiaddr));
        if (setsockopt(iosock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                       &mreq6, sizeof(mreq6))) {
            dhcpv6_dprintf(LOG_ERR,
                           "%s" "setsockopt(iosock, IPV6_JOIN_GROUP): %s",
                           FNAME, strerror(errno));
            exit(1);
        }
        freeaddrinfo(res2);

        /* set outgoing interface of multicast packets for DHCP reconfig */
        if (setsockopt(iosock, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                       &ifidx[i], sizeof(ifidx[i])) < 0) {
            dhcpv6_dprintf(LOG_ERR,
                           "%s" "setsockopt(iosock, IPV6_MULTICAST_IF): %s",
                           FNAME, strerror(errno));
            exit(1);
        }
    }
    /* set up sync lease file timer */
    sync_lease_timer = dhcp6_add_timer(check_lease_file_timo, NULL);
    d = DHCP6_SYNCFILE_TIME;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcpv6_dprintf(LOG_DEBUG, "set timer for syncing file ...");
    dhcp6_set_timer(&timo, sync_lease_timer);
    return;
}


static void server6_mainloop() {
    struct timeval *w;
    int ret;
    fd_set r;

    while (1) {
        w = dhcp6_check_timer();

        FD_ZERO(&r);
        FD_SET(iosock, &r);
        ret = select(iosock + 1, &r, NULL, NULL, w);
        switch (ret) {
            case -1:
                dhcpv6_dprintf(LOG_ERR, "%s" "select: %s",
                               FNAME, strerror(errno));
                exit(1);
                /* NOTREACHED */
            case 0:            /* timeout */
                break;
            default:
                break;
        }
        if (FD_ISSET(iosock, &r))
            server6_recv(iosock);
    }
}

static int server6_recv(s)
     int s;
{
    ssize_t len;
    struct sockaddr_storage from;
    int fromlen;
    struct msghdr mhdr;
    struct iovec iov;
    char cmsgbuf[BUFSIZ];
    struct cmsghdr *cm;
    struct in6_pktinfo *pi = NULL;
    struct dhcp6_if *ifp;
    struct dhcp6 *dh6;
    struct dhcp6_optinfo optinfo;
    struct in6_addr relay;      /* the address of the first relay, if any */

    memset(&iov, 0, sizeof(iov));
    memset(&mhdr, 0, sizeof(mhdr));

    iov.iov_base = rdatabuf;
    iov.iov_len = sizeof(rdatabuf);
    mhdr.msg_name = &from;
    mhdr.msg_namelen = sizeof(from);
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = (caddr_t) cmsgbuf;
    mhdr.msg_controllen = sizeof(cmsgbuf);

    if ((len = recvmsg(iosock, &mhdr, 0)) < 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "recvmsg: %s", FNAME, strerror(errno));
        return -1;
    }
    fromlen = mhdr.msg_namelen;

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
        return -1;
    }
    dhcpv6_dprintf(LOG_DEBUG,
                   "received message packet info addr is %s, scope id (%d)",
                   in6addr2str(&pi->ipi6_addr, 0),
                   (unsigned int) pi->ipi6_ifindex);
    if ((ifp = find_ifconfbyid((unsigned int) pi->ipi6_ifindex)) == NULL) {
        dhcpv6_dprintf(LOG_INFO, "%s" "unexpected interface (%d)", FNAME,
                       (unsigned int) pi->ipi6_ifindex);
        return -1;
    }
    if (len < sizeof(*dh6)) {
        dhcpv6_dprintf(LOG_INFO, "%s" "short packet", FNAME);
        return -1;
    }

    dh6 = (struct dhcp6 *) rdatabuf;

    dhcpv6_dprintf(LOG_DEBUG, "%s" "received %s from %s", FNAME,
                   dhcp6msgstr(dh6->dh6_msgtype),
                   addr2str((struct sockaddr *) &from, sizeof(from)));

    dhcp6_init_options(&optinfo);

    /* 
     * If this is a relayed message, parse all of the relay data, storing
     * the link addresses, peer addresses, and interface identifiers for
     * later use. Get a pointer to the original client message.
     */
    if (dh6->dh6_msgtype == DH6_RELAY_FORW) {
        dh6 = dhcp6_parse_relay((struct dhcp6_relay *) dh6,
                                (struct dhcp6_relay *) (rdatabuf + len),
                                &optinfo, &relay);

        /* 
         * NULL means there was an error in the relay format or no
         * client message was found.
         */
        if (dh6 == NULL) {
            dhcpv6_dprintf(LOG_INFO, "%s" "failed to parse relay fields "
                           "or could not find client message", FNAME);
            return -1;
        }
    }

    /* 
     * parse and validate options in the request
     */
    if (dhcp6_get_options((struct dhcp6opt *) (dh6 + 1),
                          (struct dhcp6opt *) (rdatabuf + len),
                          &optinfo) < 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "failed to parse options", FNAME);
        return -1;
    }
    /* check host decl first */
    host = dhcp6_allocate_host(ifp, globalgroup, &optinfo);
    /* ToDo: allocate subnet after relay agent done now assume client is on
       the same link as server if the subnet couldn't be found return status
       code NotOnLink to client */
    /* 
     * If the relay list is empty, then this is a message received directly
     * from the client, so client is on the same link as the server. 
     * Otherwise, allocate the client an address based on the first relay
     * that forwarded the message.
     */
    if (TAILQ_EMPTY(&optinfo.relay_list))
        subnet = dhcp6_allocate_link(ifp, globalgroup, NULL);
    else
        subnet = dhcp6_allocate_link(ifp, globalgroup, &relay);

    if (!(DH6_VALID_MESSAGE(dh6->dh6_msgtype)))
        dhcpv6_dprintf(LOG_INFO, "%s" "unknown or unsupported msgtype %s",
                       FNAME, dhcp6msgstr(dh6->dh6_msgtype));
    else
        server6_react_message(ifp, pi, dh6, &optinfo,
                              (struct sockaddr *) &from, fromlen);
    dhcp6_clear_options(&optinfo);
    return 0;
}

static int
handle_addr_request(struct dhcp6_optinfo *roptinfo,
                    struct ia_list *ria_list,
                    struct ia_list *ia_list,
                    int resptype,
                    int *status_code)
{
    struct ia_listval *ria, *ia;
    struct dhcp6_iaidaddr *iaidaddr;
    int addr_flag = 0;
    int found_binding = 0;

    for (ia = TAILQ_FIRST(ia_list); ia; ia = TAILQ_NEXT(ia, link)) {
        /* find bindings */
        if ((iaidaddr = dhcp6_find_iaidaddr(&roptinfo->clientID,
                        ia->iaidinfo.iaid, ia->type)) != NULL) {
            found_binding = 1;
            addr_flag = ADDR_UPDATE;
        }

        if ((ria = ia_create_listval()) == NULL)
            goto fail;
        ria->type = ia->type;
        ria->iaidinfo.iaid = ia->iaidinfo.iaid;

        if (host)
            dhcp6_get_hostconf(ria, ia, iaidaddr, host);

        /* valid and create addresses list */
        if (ia->type == IAPD) {
            if (dhcp6_create_prefixlist(ria, ia, iaidaddr,
                        subnet, &ria->status_code))
                goto fail;
        } else {
            if (dhcp6_create_addrlist(ria, ia, iaidaddr,
                        subnet, &ria->status_code))
                goto fail;
        }

        if (TAILQ_EMPTY(&ria->addr_list)) {
            if (resptype == DH6_ADVERTISE) {
                /* Omit IA option */
                free(ria);
                continue;
            } else if (resptype == DH6_REPLY) {
                /* Set status code in IA */
                ria->status_code = DH6OPT_STCODE_NOADDRAVAIL;
            }
        } else if (resptype == DH6_REPLY &&
                ria->status_code == DH6OPT_STCODE_UNDEFINE) {
            /* valid client request address list */
            if (found_binding) {
                if (dhcp6_update_iaidaddr(roptinfo, ria, addr_flag) != 0) {
                    dhcpv6_dprintf(LOG_ERR, "assigned ipv6address for client "
                            "iaid %u failed", ria->iaidinfo.iaid);
                    ria->status_code = DH6OPT_STCODE_UNSPECFAIL;
                } else {
                    ria->status_code = DH6OPT_STCODE_SUCCESS;
                }
            } else {
                if (dhcp6_add_iaidaddr(roptinfo, ria) != 0) {
                    dhcpv6_dprintf(LOG_ERR, "assigned ipv6address for client "
                            "iaid %u failed", ria->iaidinfo.iaid);
                    ria->status_code = DH6OPT_STCODE_UNSPECFAIL;
                } else {
                    ria->status_code = DH6OPT_STCODE_SUCCESS;
                }
            }
        }
        TAILQ_INSERT_TAIL(ria_list, ria, link);
    }

    if (resptype == DH6_ADVERTISE && TAILQ_EMPTY(ria_list))
        *status_code = DH6OPT_STCODE_NOADDRAVAIL;
    return 0;

fail:
    if (ria != NULL)
        free(ria);
    ia_clear_list(ia_list);
    return -1;
}

static int
update_binding_ia(struct duid *clientID,
                  struct ia_list *ria_list,
                  struct ia_list *ia_list,
                  u_int8_t msgtype,
                  int addr_flag,
                  int *status_code)
{
    struct ia_listval *ria, *ia;
    struct dhcp6_iaidaddr *iaidaddr;
    size_t num_ia = 0;
    size_t num_noaddr_ia = 0;
    size_t num_nobinding_ia = 0;
    size_t num_invalid_ia = 0;

    for (ia = TAILQ_FIRST(ia_list); ia; ia = TAILQ_NEXT(ia, link)) {
        ++num_ia;
        ria = NULL;
        if (!TAILQ_EMPTY(&ia->addr_list)) {
            if (addr_flag != ADDR_VALIDATE) {
                if ((ria = ia_create_listval()) == NULL)
                    goto fail;
                ria->type = ia->type;
                ria->iaidinfo = ia->iaidinfo;
            }

            if ((iaidaddr = dhcp6_find_iaidaddr(clientID,
                            ia->iaidinfo.iaid, ia->type)) == NULL) {
                /* Not found binding IA Addr */
                ++num_nobinding_ia;
                dhcpv6_dprintf(LOG_INFO, "%s" "Nobinding for client %s iaid %u",
                        FNAME, duidstr(clientID), ia->iaidinfo.iaid);
                if (addr_flag == ADDR_VALIDATE) {
                    goto out;
                } else if (msgtype == DH6_REBIND) {
                    free(ria);
                    ria = NULL;
                } else {
                    ria->status_code = DH6OPT_STCODE_NOBINDING;
                }
            } else {
                /* Found a binding IA Addr */
                switch (addr_flag) {
                    case ADDR_VALIDATE:
                        if (dhcp6_validate_bindings(&ia->addr_list, iaidaddr, 0)) {
                            ++num_invalid_ia;
                            goto out;
                        }
                        break;

                    case ADDR_UPDATE:
                        /* get static host configuration */
                        if (dhcp6_validate_bindings(&ia->addr_list, iaidaddr, 1)) {
                            ++num_invalid_ia;
                            dhcp6_copy_list(&ria->addr_list, &ia->addr_list);
                            break;
                        }
                        if (host)
                            dhcp6_get_hostconf(ria, ia, iaidaddr, host);

                        /* allow dynamic address assginment for the host too */
                        if (ria->type == IAPD) {
                            if (dhcp6_create_prefixlist(ria, ia,
                                        iaidaddr, subnet, &ria->status_code))
                                goto fail;
                        } else {
                            if (dhcp6_create_addrlist(ria, ia,
                                        iaidaddr, subnet, &ria->status_code))
                                goto fail;
                        }
                        break;

                    default:
                        dhcp6_copy_list(&ria->addr_list, &ia->addr_list);
                        break;
                }
            }
            if (ria != NULL)
                TAILQ_INSERT_TAIL(ria_list, ria, link);
        } else {
            /* IA doesn't include any IA Addr */
            ++num_noaddr_ia;
        }
    }

out:
    switch (msgtype) {
        case DH6_CONFIRM:
            if (num_noaddr_ia == num_ia) {
                dhcpv6_dprintf(LOG_DEBUG, "No addresses in confirm message");
                goto fail;
            } else if (num_nobinding_ia || num_invalid_ia) {
                *status_code = DH6OPT_STCODE_NOTONLINK;
            } else {
                *status_code = DH6OPT_STCODE_SUCCESS;
            }
            break;
        case DH6_RENEW:
            break;
        case DH6_REBIND:
            if (num_noaddr_ia + num_nobinding_ia == num_ia)
                goto fail;
        case DH6_RELEASE:
        case DH6_DECLINE:
            *status_code = DH6OPT_STCODE_SUCCESS;
            break;
    }
    return 0;

fail:
    if (ria != NULL)
        free(ria);
    ia_clear_list(ria_list);
    return -1;
}

static int server6_react_message(struct dhcp6_if *ifp,
                                 struct in6_pktinfo *pi,
                                 struct dhcp6 *dh6,
                                 struct dhcp6_optinfo *optinfo,
                                 struct sockaddr *from, int fromlen) {
    struct dhcp6_optinfo roptinfo;
    int addr_flag = 0;
    int resptype = DH6_REPLY;
    int num = DH6OPT_STCODE_UNDEFINE;

    /* message validation according to Section 18.2 of dhcpv6-28 */

    /* the message must include a Client Identifier option */
    if (optinfo->clientID.duid_len == 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "no client ID option", FNAME);
        return -1;
    } else {
        dhcpv6_dprintf(LOG_DEBUG, "%s" "client ID %s", FNAME,
                       duidstr(&optinfo->clientID));
    }

    /* Make sure DUID LLT time field matches the client */
    if (duid_match_llt(&optinfo->clientID, &server_duid)) {
        dprintf(LOG_INFO, "failed to match DUID LLT time field between server and client");
        return -1;
    }

    /* what kind of message did we receive? */
    switch (dh6->dh6_msgtype) {
            /* these messages must include a Server Identifier option */
        case DH6_REQUEST:
        case DH6_RENEW:
        case DH6_DECLINE:
        case DH6_RELEASE:
            if (optinfo->serverID.duid_len == 0) {
                dhcpv6_dprintf(LOG_INFO, "%s" "no server ID option", FNAME);
                return -1;
            }

            /* the contents of the Server Identifier option must match ours */
            if (duidcmp(&optinfo->serverID, &server_duid)) {
                dhcpv6_dprintf(LOG_INFO, "server ID %s mismatch %s",
                               duidstr(&optinfo->serverID),
                               duidstr(&server_duid));
                return -1;
            }

            break;

            /* these messages must not include a Server Identifier option */
        case DH6_SOLICIT:
        case DH6_CONFIRM:
        case DH6_REBIND:
            if (optinfo->serverID.duid_len != 0) {
                dhcpv6_dprintf(LOG_INFO,
                               "%s" "found server ID option in message "
                               "Solicit/Confirm/Rebind", FNAME);
                return -1;
            }

            break;

        default:
            break;
    }

    /* configure necessary options based on the options in request. */
    dhcp6_init_options(&roptinfo);

    /* server information option */
    if (duidcpy(&roptinfo.serverID, &server_duid)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy server ID", FNAME);
        goto fail;
    }

    /* copy client information back */
    if (duidcpy(&roptinfo.clientID, &optinfo->clientID)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "failed to copy client ID", FNAME);
        goto fail;
    }

    /* if the client is not on the link */
    if (host == NULL && subnet == NULL) {
        num = DH6OPT_STCODE_NOTONLINK;

        /* Draft-28 18.2.2, drop the message if NotOnLink */
        if (dh6->dh6_msgtype == DH6_CONFIRM || dh6->dh6_msgtype == DH6_REBIND) {
            goto fail;
        } else {
            goto send;
        }
    }

    if (subnet) {
        if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_PREFERENCE)) {
            roptinfo.pref = subnet->linkscope.server_pref;
        }

        roptinfo.flags = (optinfo->flags & subnet->linkscope.allow_flags) |
            subnet->linkscope.send_flags;

        if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_DNS_SERVERS) ||
            dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_DOMAIN_LIST)) {
            dnslist = subnet->linkscope.dnslist;
        }

        if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_INFO_REFRESH_TIME)) {
            roptinfo.irt = subnet->linkscope.irt;
        }
    }

    if (host) {
        if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_PREFERENCE)) {
            roptinfo.pref = host->hostscope.server_pref;
        }

        roptinfo.flags = (optinfo->flags & host->hostscope.allow_flags) |
            host->hostscope.send_flags;

        if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_DNS_SERVERS) ||
            dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_DOMAIN_LIST)) {
            dnslist = host->hostscope.dnslist;
        }

        if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_INFO_REFRESH_TIME)) {
            roptinfo.irt = host->hostscope.irt;
        }
    }

    /* prohibit a mixture of old and new style of DNS server config */
    if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_DNS_SERVERS)) {
        if (!TAILQ_EMPTY(&arg_dnslist.addrlist)) {
            if (!TAILQ_EMPTY(&dnslist.addrlist)) {
                dhcpv6_dprintf(LOG_INFO, "%s" "do not specify DNS servers "
                               "both by command line and by configuration file.",
                               FNAME);
                exit(1);
            }

            dnslist = arg_dnslist;
            TAILQ_INIT(&arg_dnslist.addrlist);
        }
    }

    if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_PREFERENCE)) {
        dhcpv6_dprintf(LOG_DEBUG, "server preference is %2x", roptinfo.pref);
    }

    if (roptinfo.flags & DHCIFF_UNICAST) {
        /* todo find the right server unicast address to client */
        /* get_linklocal(device, &roptinfo.server_addr) */
        memcpy(&roptinfo.server_addr, &ifp->linklocal,
               sizeof(roptinfo.server_addr));
        dhcpv6_dprintf(LOG_DEBUG, "%s" "server address is %s",
                       FNAME, in6addr2str(&roptinfo.server_addr, 0));
    }

    /* 
     * When the server receives a Request message via unicast from a
     * client to which the server has not sent a unicast option, the server
     * discards the Request message and responds with a Reply message
     * containing a Status Code option with value UseMulticast, a Server
     * Identifier option containing the server's DUID, the Client
     * Identifier option from the client message and no other options.
     * [dhcpv6-26 18.2.1]
     */
    switch (dh6->dh6_msgtype) {
        case DH6_REQUEST:
        case DH6_RENEW:
        case DH6_DECLINE:
            /* 
             * If the message was relayed, then do not check whether the
             * message came in via unicast or multicast, since the relay
             * may be configured to send messages via unicast.
             */
            if (TAILQ_EMPTY(&optinfo->relay_list) &&
                !IN6_IS_ADDR_MULTICAST(&pi->ipi6_addr)) {
                if (!(roptinfo.flags & DHCIFF_UNICAST)) {
                    num = DH6OPT_STCODE_USEMULTICAST;
                    goto send;
                } else {
                    break;
                }
            }

            break;
        case DH6_SOLICIT:
        case DH6_CONFIRM:
        case DH6_REBIND:
        case DH6_INFORM_REQ:
            /* A server MUST discard any Solicit, Confirm, Rebind or
               Information-request * messages it receives with a unicast
               destination address. [RFC3315 Section 15] */
            if (TAILQ_EMPTY(&optinfo->relay_list) &&
                !IN6_IS_ADDR_MULTICAST(&pi->ipi6_addr)) {
                dhcpv6_dprintf(LOG_DEBUG, "reply no message as %s received "
                               "with unicast destination address",
                               dhcp6msgstr(num));
                goto fail;
            }

            break;
        default:
            /* 
             * If the message was relayed, then do not check whether the
             * message came in via unicast or multicast, since the relay
             * may be configured to send messages via unicast.
             */
            if (TAILQ_EMPTY(&optinfo->relay_list) &&
                !IN6_IS_ADDR_MULTICAST(&pi->ipi6_addr)) {
                num = DH6OPT_STCODE_USEMULTICAST;
                goto send;
            }

            break;
    }

    switch (dh6->dh6_msgtype) {
        case DH6_SOLICIT:
            /* 
             * If the client has included a Rapid Commit option and the
             * server has been configured to respond with committed address
             * assignments and other resources, responds to the Solicit
             * with a Reply message.
             * [dhcpv6-28 Section 17.2.1]
             * [dhcpv6-28 Section 17.2.2]
             * If Solicit has IA option, responds to Solicit with a Advertise
             * message.
             */
            if (!TAILQ_EMPTY(&optinfo->ia_list) &&
                !(roptinfo.flags & DHCIFF_INFO_ONLY)) {
                resptype = (roptinfo.flags & DHCIFF_RAPID_COMMIT)
                           ? DH6_REPLY : DH6_ADVERTISE;
                if (handle_addr_request(&roptinfo, &roptinfo.ia_list,
                                        &optinfo->ia_list, resptype,
                                        &num)) {
                    goto fail;
                }
            }

            break;
        case DH6_INFORM_REQ:
            /* don't response to info-req if there is any IA or server ID
               option */
            if (!TAILQ_EMPTY(&optinfo->ia_list) || optinfo->serverID.duid_len) {
                goto fail;
            }

            break;
        case DH6_REQUEST:
            /* get iaid for that request client for that interface */
            if (!TAILQ_EMPTY(&optinfo->ia_list) &&
                !(roptinfo.flags & DHCIFF_INFO_ONLY)) {
                if (handle_addr_request(&roptinfo, &roptinfo.ia_list,
                                        &optinfo->ia_list, resptype,
                                        &num)) {
                    goto fail;
                }
            }

            break;
        case DH6_DECLINE:
        case DH6_RELEASE:
        case DH6_RENEW:
        case DH6_REBIND:
        case DH6_CONFIRM:
            /* 
             * Locates the client's binding and verifies that the information
             * from the client matches the information stored for that client.
             */
            switch (dh6->dh6_msgtype) {
                case DH6_CONFIRM:
                    addr_flag = ADDR_VALIDATE;
                    break;
                case DH6_RENEW:
                case DH6_REBIND:
                    /*
                     * XXX: how server know the diff between rebind_confirm and
                     * rebind for prefix delegation?
                     */
                    addr_flag = ADDR_UPDATE;
                    break;
                case DH6_RELEASE:
                    addr_flag = ADDR_REMOVE;
                    break;
                case DH6_DECLINE:
                    addr_flag = ADDR_ABANDON;
                    break;
            }

            if (!TAILQ_EMPTY(&optinfo->ia_list)) {
                if (update_binding_ia(&roptinfo.clientID, &roptinfo.ia_list,
                    &optinfo->ia_list, dh6->dh6_msgtype, addr_flag, &num)) {
                    goto fail;
                }
            } else {
                dhcpv6_dprintf(LOG_ERR, "invalid message type");
            }

            break;
        default:
            break;
    }

    /* Options regarding DNS */
    switch (dh6->dh6_msgtype) {
        case DH6_SOLICIT:
        case DH6_REQUEST:
        case DH6_RENEW:
        case DH6_REBIND:
        case DH6_INFORM_REQ:
            /* DNS Recursive Name Server option */
            if (dhcp6_has_option(&optinfo->reqopt_list,
                                 DH6OPT_DNS_SERVERS)) {
                if (dhcp6_copy_list(&roptinfo.dns_list.addrlist,
                                    &dnslist.addrlist)) {
                    dhcpv6_dprintf(LOG_ERR,
                                   "%s" "failed to copy DNS servers",
                                   FNAME);
                    goto fail;
                }
            }

            /* Domain Search List option */
            if (dhcp6_has_option(&optinfo->reqopt_list,
                                 DH6OPT_DOMAIN_LIST)) {
                roptinfo.dns_list.domainlist = dnslist.domainlist;
            }

            break;
    }

    /* Information refresh time option (RFC 4242) */
    if (dhcp6_has_option(&optinfo->reqopt_list, DH6OPT_INFO_REFRESH_TIME)) {
        switch (dh6->dh6_msgtype) {
            case DH6_INFORM_REQ:
                if (roptinfo.irt == 0) {
                    roptinfo.irt = IRT_DEFAULT;
                }
                dhcpv6_dprintf(LOG_DEBUG, "information refresh time is %u",
                               roptinfo.irt);
                break;
            default:
                dhcpv6_dprintf(LOG_INFO, "Ignore the requirement to reply "
                               "an information refresh time option as the "
                               "message is %s", dhcp6msgstr(dh6->dh6_msgtype));
                roptinfo.irt = 0;
                break;
        }
    } else {
        /* make sure that the infomation refresh time is set to 0 */
        roptinfo.irt = 0;
    }

    /* add address status code */
 send:
    dhcpv6_dprintf(LOG_DEBUG, " status code: %s", dhcp6_stcodestr(num));
    roptinfo.status_code = num;

    /* send a reply message. */
    (void) server6_send(resptype, ifp, dh6, optinfo, from, fromlen,
                        &roptinfo);

    dhcp6_clear_options(&roptinfo);
    return 0;

fail:
    dhcp6_clear_options(&roptinfo);
    return -1;
}

static int server6_send(type, ifp, origmsg, optinfo, from, fromlen, roptinfo)
     int type;
     struct dhcp6_if *ifp;
     struct dhcp6 *origmsg;
     struct dhcp6_optinfo *optinfo, *roptinfo;
     struct sockaddr *from;
     int fromlen;
{
    char replybuf[BUFSIZ];
    struct sockaddr_in6 dst;
    int len, optlen, relaylen = 0;
    struct dhcp6 *dh6;

    if (sizeof(struct dhcp6) > sizeof(replybuf)) {
        dhcpv6_dprintf(LOG_ERR, "%s" "buffer size assumption failed", FNAME);
        return (-1);
    }

    if (!TAILQ_EMPTY(&optinfo->relay_list) &&
        (relaylen = dhcp6_set_relay((struct dhcp6_relay *) replybuf,
                                    (struct dhcp6_relay *) (replybuf +
                                                            sizeof(replybuf)),
                                    optinfo)) < 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "failed to construct relay message",
                       FNAME);
        return (-1);
    }

    dh6 = (struct dhcp6 *) (replybuf + relaylen);
    len = sizeof(*dh6);
    memset(dh6, 0, sizeof(*dh6));
    dh6->dh6_msgtypexid = origmsg->dh6_msgtypexid;
    dh6->dh6_msgtype = (u_int8_t) type;

    /* set options in the reply message */
    if ((optlen = dhcp6_set_options((struct dhcp6opt *) (dh6 + 1),
                                    (struct dhcp6opt *) (replybuf +
                                                         sizeof(replybuf)),
                                    roptinfo)) < 0) {
        dhcpv6_dprintf(LOG_INFO, "%s" "failed to construct reply options",
                       FNAME);
        return (-1);
    }
    len += optlen;

    /* 
     * If there were any Relay Message options, fill in the option-len
     * field(s) with the appropriate value(s).
     */
    if (!TAILQ_EMPTY(&optinfo->relay_list))
        dhcp6_set_relay_option_len(optinfo, len);

    len += relaylen;

    /* specify the destination and send the reply */
    dst = *sa6_any_downstream;
    dst.sin6_addr = ((struct sockaddr_in6 *) from)->sin6_addr;

    /* RELAY-REPL messages need to be directed back to the port the relay
       agent is listening on, namely DH6PORT_UPSTREAM */
    if (relaylen > 0)
        dst.sin6_port = upstream_port;

    dst.sin6_scope_id = ((struct sockaddr_in6 *) from)->sin6_scope_id;
    dhcpv6_dprintf(LOG_DEBUG,
                   "send destination address is %s, scope id is %d",
                   addr2str((struct sockaddr *) &dst, sizeof(dst)),
                   dst.sin6_scope_id);
    if (transmit_sa(iosock, &dst, replybuf, len) != 0) {
        dhcpv6_dprintf(LOG_ERR, "%s" "transmit %s to %s failed", FNAME,
                       dhcp6msgstr(type), addr2str((struct sockaddr *) &dst,
                                                   sizeof(dst)));
        return (-1);
    }

    dhcpv6_dprintf(LOG_DEBUG, "%s" "transmit %s to %s", FNAME,
                   dhcp6msgstr(type), addr2str((struct sockaddr *) &dst,
                                               sizeof(dst)));

    return 0;
}

static struct dhcp6_timer
 *check_lease_file_timo(void *arg) {
    double d;
    struct timeval timo;
    struct stat buf;
    FILE *file;

    strcpy(server6_lease_temp, PATH_SERVER6_LEASE);
    strcat(server6_lease_temp, "XXXXXX");
    if (!stat(PATH_SERVER6_LEASE, &buf)) {
        if (buf.st_size > MAX_FILE_SIZE) {
            file =
                sync_leases(server6_lease_file, PATH_SERVER6_LEASE,
                            server6_lease_temp);
            if (file != NULL)
                server6_lease_file = file;
        }
    }
    d = DHCP6_SYNCFILE_TIME;
    timo.tv_sec = (long) d;
    timo.tv_usec = 0;
    dhcp6_set_timer(&timo, sync_lease_timer);
    return sync_lease_timer;
}

/*
 * Parse all of the RELAY-FORW messages and interface ID options. Each
 * RELAY-FORW messages will have its hop count, link address, peer-address,
 * and interface ID (if any) put into a relay_listval structure.
 * A pointer to the actual original client message will be returned.
 * If this client message cannot be found, NULL is returned to signal an error.
 */
static struct dhcp6 *dhcp6_parse_relay(relay_msg, endptr, optinfo, relay_addr)
     struct dhcp6_relay *relay_msg;
     struct dhcp6_relay *endptr;
     struct dhcp6_optinfo *optinfo;
     struct in6_addr *relay_addr;
{
    struct relay_listval *relay_val;
    struct dhcp6 *relayed_msg;  /* the original message that the relay
                                   received */
    struct dhcp6opt *option, *option_endptr = (struct dhcp6opt *) endptr;

    u_int16_t optlen;
    u_int16_t opt;

    while ((relay_msg + 1) < endptr) {
        relay_val = (struct relay_listval *)
            calloc(1, sizeof(struct relay_listval));

        if (relay_val == NULL) {
            dhcpv6_dprintf(LOG_ERR, "%s" "failed to allocate memory", FNAME);
            relayfree(&optinfo->relay_list);
            return NULL;
        }

        /* copy the msg-type, hop-count, link-address, and peer-address */
        memcpy(&relay_val->relay, relay_msg, sizeof(struct dhcp6_relay));

        /* set the msg type to relay reply now so that it doesn't need to be
           done when formatting the reply */
        relay_val->relay.dh6_msg_type = DH6_RELAY_REPL;

        TAILQ_INSERT_TAIL(&optinfo->relay_list, relay_val, link);

        /* 
         * need to record the first relay's link address field for later use.
         * The first relay is the last one we see, so keep overwriting the
         * relay value.
         */
        memcpy(relay_addr, &relay_val->relay.link_addr,
               sizeof(struct in6_addr));

        /* now handle the options in the RELAY-FORW message */
        /* 
         * The only options that should appear in a RELAY-FORW message are:
         * - Interface identifier
         * - Relay message
         *
         * All other options are ignored.
         */
        option = (struct dhcp6opt *) (relay_msg + 1);

        relayed_msg = NULL;     /* if this is NULL at the end of the loop, no
                                   relayed message was found */

        /* since the order of options is not specified, all of the options
           must be processed */
        while ((option + 1) < option_endptr) {
            memcpy(&opt, &option->dh6opt_type, sizeof(opt));
            opt = ntohs(opt);
            memcpy(&optlen, &option->dh6opt_len, sizeof(optlen));
            optlen = ntohs(optlen);

            if ((char *) (option + 1) + optlen > (char *) option_endptr) {
                dhcpv6_dprintf(LOG_ERR,
                               "%s" "invalid option length in %s option",
                               FNAME, dhcp6optstr(opt));
                relayfree(&optinfo->relay_list);
                return NULL;
            }

            if (opt == DH6OPT_INTERFACE_ID) {
                /* if this is not the first interface identifier option, then 
                   the message is incorrectly formed */
                if (relay_val->intf_id == NULL) {
                    if (optlen) {
                        relay_val->intf_id = (struct intf_id *)
                            malloc(sizeof(struct intf_id));
                        if (relay_val->intf_id == NULL) {
                            dhcpv6_dprintf(LOG_ERR,
                                           "%s" "failed to allocate memory",
                                           FNAME);
                            relayfree(&optinfo->relay_list);
                            return NULL;
                        } else {
                            relay_val->intf_id->intf_len = optlen;
                            relay_val->intf_id->intf_id = (char *)
                                malloc(optlen);

                            if (relay_val->intf_id->intf_id == NULL) {
                                dhcpv6_dprintf(LOG_ERR, "%s"
                                               "failed to allocate memory",
                                               FNAME);
                                relayfree(&optinfo->relay_list);
                                return NULL;
                            } else {    /* copy the interface identifier so
                                           it can be sent in the reply */
                                memcpy(relay_val->intf_id->intf_id,
                                       ((char *) (option + 1)), optlen);
                            }
                        }
                    } else {
                        dhcpv6_dprintf(LOG_ERR,
                                       "%s" "Invalid length for interface "
                                       "identifier option", FNAME);
                    }
                } else {
                    dhcpv6_dprintf(LOG_INFO, "%s"
                                   "Multiple interface identifier "
                                   "options in RELAY-FORW Message ", FNAME);
                    relayfree(&optinfo->relay_list);
                    return NULL;
                }
            } else if (opt == DH6OPT_RELAY_MSG) {
                if (relayed_msg == NULL)
                    relayed_msg = (struct dhcp6 *) (option + 1);
                else {
                    dhcpv6_dprintf(LOG_INFO,
                                   "%s" "Duplicated Relay Message option",
                                   FNAME);
                    relayfree(&optinfo->relay_list);
                    return NULL;
                }
            } else              /* No other options besides interface
                                   identifier and relay message make sense,
                                   so ignore them with a warning */
                dhcpv6_dprintf(LOG_INFO,
                               "%s" "Unsupported option %s found in "
                               "RELAY-FORW message", FNAME, dhcp6optstr(opt));

            /* advance the option pointer */
            option = (struct dhcp6opt *) (((char *) (option + 1)) + optlen);
        }

        /* 
         * If the relayed message is non-NULL and is a regular client
         * message, then the relay processing is done. If it is another
         * RELAY_FORW message, then continue. If the relayed message is
         * NULL, signal an error.
         */
        if (relayed_msg != NULL && (char *) (relayed_msg + 1) <=
            (char *) endptr) {
            /* done if have found the client message */
            if (relayed_msg->dh6_msgtype != DH6_RELAY_FORW)
                return relayed_msg;
            else
                relay_msg = (struct dhcp6_relay *) relayed_msg;
        } else {
            dhcpv6_dprintf(LOG_ERR, "%s" "invalid relayed message", FNAME);
            relayfree(&optinfo->relay_list);
            return NULL;
        }
    }
    return NULL;
}

/*
 * Format all of the RELAY-REPL messages and options to send back to the 
 * client. A RELAY-REPL message and Relay Message option are added for 
 * each of the relays that were in the RELAY-FORW packet that this is 
 * in response to.
 */
static int dhcp6_set_relay(msg, endptr, optinfo)
     struct dhcp6_relay *msg;
     struct dhcp6_relay *endptr;
     struct dhcp6_optinfo *optinfo;
{
    struct relay_listval *relay;
    struct dhcp6opt *option;
    int relaylen = 0;
    u_int16_t type, len;

    for (relay = TAILQ_FIRST(&optinfo->relay_list); relay;
         relay = TAILQ_NEXT(relay, link)) {
        /* bounds check */
        if (((char *) msg) + sizeof(struct dhcp6_relay) >= (char *) endptr) {
            dhcpv6_dprintf(LOG_ERR,
                           "%s" "insufficient buffer size for RELAY-REPL",
                           FNAME);
            return -1;
        }

        memcpy(msg, &relay->relay, sizeof(struct dhcp6_relay));

        relaylen += sizeof(struct dhcp6_relay);

        option = (struct dhcp6opt *) (msg + 1);

        /* include an Interface Identifier option if it was present in the
           original message */
        if (relay->intf_id != NULL) {
            /* bounds check */
            if ((((char *) option) + sizeof(struct dhcp6opt) +
                 relay->intf_id->intf_len) >= (char *) endptr) {
                dhcpv6_dprintf(LOG_ERR,
                               "%s" "insufficient buffer size for RELAY-REPL",
                               FNAME);
                return -1;
            }
            type = htons(DH6OPT_INTERFACE_ID);
            memcpy(&option->dh6opt_type, &type, sizeof(type));
            len = htons(relay->intf_id->intf_len);
            memcpy(&option->dh6opt_len, &len, sizeof(len));
            memcpy(option + 1, relay->intf_id->intf_id,
                   relay->intf_id->intf_len);

            option = (struct dhcp6opt *) (((char *) (option + 1)) +
                                          relay->intf_id->intf_len);
            relaylen += sizeof(struct dhcp6opt) + relay->intf_id->intf_len;
        }

        /* save a pointer to the relay message option so that it is easier
           to fill in the length later */
        relay->option = option;

        /* bounds check */
        if ((char *) (option + 1) >= (char *) endptr) {
            dhcpv6_dprintf(LOG_ERR,
                           "%s" "insufficient buffer size for RELAY-REPL",
                           FNAME);
            return -1;
        }

        /* lastly include the Relay Message option, which encapsulates the
           message being relayed */
        type = htons(DH6OPT_RELAY_MSG);
        memcpy(&option->dh6opt_type, &type, sizeof(type));
        relaylen += sizeof(struct dhcp6opt);
        /* dh6opt_len will be set by dhcp6_set_relay_option_len */

        msg = (struct dhcp6_relay *) (option + 1);
    }

    /* 
     * if there were no relays, this is an error since this function should
     * not have even been called in this case
     */
    if (relaylen == 0)
        return -1;
    else
        return relaylen;
}

/*
 * Fill in all of the opt-len fields for the Relay Message options now that
 * the length of the entire message is known.
 * 
 * len - the length of the DHCPv6 message to the client (not including any
 *       relay options)
 *
 * Precondition: dhcp6_set_relay has already been called and the relay->option
 *               fields of all of the elements in optinfo->relay_list are 
 *               non-NULL
 */
static void dhcp6_set_relay_option_len(optinfo, reply_msg_len)
     struct dhcp6_optinfo *optinfo;
     int reply_msg_len;
{
    struct relay_listval *relay, *last = NULL;
    u_int16_t len;

    for (relay = TAILQ_LAST(&optinfo->relay_list, relay_list);
         relay; relay = TAILQ_PREV(relay, relay_list, link)) {
        if (last == NULL) {
            len = htons(reply_msg_len);
            memcpy(&relay->option->dh6opt_len, &len, sizeof(len));
            last = relay;
        } else {
            len = reply_msg_len + (((void *) (last->option + 1)) -
                                   ((void *) (relay->option + 1)));
            len = htons(len);
            memcpy(&relay->option->dh6opt_len, &len, sizeof(len));
        }
    }
}

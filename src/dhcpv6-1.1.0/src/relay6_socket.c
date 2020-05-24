/*
 * Copyright (C) NEC Europe Ltd., 2003
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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "dhcp6r.h"
#include "relay6_socket.h"
#include "relay6_parser.h"
#include "relay6_database.h"

#ifndef IPV6_2292PKTINFO
#define IPV6_2292PKTINFO IPV6_PKTINFO
#endif

extern FILE *dump;

void init_socket() {
    relaysock = (struct relay_socket *) malloc(sizeof(struct relay_socket));

    if (relaysock == NULL) {
        TRACE(dump, "%s - %s", dhcp6r_clock(),
              "init_socket--> error no more memory available\n");
        exit(1);
    }

    memset(relaysock, 0, sizeof(struct relay_socket));

    relaysock->databuf = (char *) malloc(MAX_DHCP_MSG_LENGTH * sizeof(char));
    if (relaysock->databuf == NULL) {
        TRACE(dump, "%s - %s", dhcp6r_clock(),
              "init_socket--> error no more memory available\n");
        exit(1);
    }

    if ((relaysock->sock_desc = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        TRACE(dump, "failed to get new socket with socket()\n");
        exit(0);
    }
}

int get_recv_data() {
    struct cmsghdr *cm;
    struct in6_pktinfo *pi;
    struct sockaddr_in6 dst;

    memset(relaysock->src_addr, 0, sizeof(relaysock->src_addr));

    for (cm = (struct cmsghdr *) CMSG_FIRSTHDR(&relaysock->msg); cm;
         cm = (struct cmsghdr *) CMSG_NXTHDR(&relaysock->msg, cm)) {
        if ((cm->cmsg_level == IPPROTO_IPV6)
            && (cm->cmsg_type == IPV6_2292PKTINFO)
            && (cm->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo)))) {
            pi = (struct in6_pktinfo *) (CMSG_DATA(cm));
            dst.sin6_addr = pi->ipi6_addr;
            relaysock->pkt_interface = pi->ipi6_ifindex;        /* the
                                                                   interface
                                                                   index the 
                                                                   packet got 
                                                                   in */

            if (IN6_IS_ADDR_LOOPBACK(&relaysock->from.sin6_addr)) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "get_recv_data()--> source address is loopback!\n");
                return 0;
            }

            if (inet_ntop(AF_INET6, &relaysock->from.sin6_addr,
                          relaysock->src_addr, INET6_ADDRSTRLEN) <= 0) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "inet_ntop failed in get_recv_data()\n");
                return 0;
            }

            if (IN6_IS_ADDR_LOOPBACK(&dst.sin6_addr)) {
                relaysock->dst_addr_type = 1;
            } else if (IN6_IS_ADDR_MULTICAST(&dst.sin6_addr)) {
                relaysock->dst_addr_type = 2;
                if (multicast_off == 1) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "received multicast packet is dropped, only unicast "
                          "is allowed!\n");
                    return 0;
                }
            } else if (IN6_IS_ADDR_LINKLOCAL(&dst.sin6_addr)) {
                relaysock->dst_addr_type = 3;
            } else if (IN6_IS_ADDR_SITELOCAL(&dst.sin6_addr))
                relaysock->dst_addr_type = 4;

            return 1;
        }
    }                           /* for */

    return 0;
}

int check_select(void) {
    int i = 0;
    int flag = 0;
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&readfd);
    /* check the max of them if many desc used */
    fdmax = relaysock->sock_desc;
    FD_SET(fdmax, &readfd);

    if ((i = select(fdmax + 1, &readfd, NULL, NULL, &tv)) == -1) {
        TRACE(dump, "%s - %s", dhcp6r_clock(), "failure in select()\n");
        return 0;
    }

    if (FD_ISSET(fdmax, &readfd)) {
        flag = 1;
    } else {
        flag = 0;
    }

    return flag;
}

int set_sock_opt() {
    int on = 1;
    int hop_limit;
    struct interface *device;
    int flag;
    struct cifaces *iface;
    struct ipv6_mreq sock_opt;

    /* If the relay agent relays messages to the All_DHCP_Servers multicast
       address or other multicast addresses, it sets the Hop Limit field to
       32. [RFC3315 Section 20] */
    hop_limit = 32;
    if (setsockopt(relaysock->sock_desc, IPPROTO_IPV6,
                   IPV6_MULTICAST_HOPS, &hop_limit, sizeof(hop_limit)) < 0) {
        TRACE(dump, "%s - %s, %s\n", dhcp6r_clock(),
              "failed to set socket for IPV6_MULTICAST_HOPS",
              strerror(errno));
        return 0;
    }

    if (setsockopt(relaysock->sock_desc, IPPROTO_IPV6, IPV6_2292PKTINFO,
                   &on, sizeof(on)) < 0) {
        TRACE(dump, "%s - %s, %s\n", dhcp6r_clock(),
              "failed to set socket for IPV6_2292PKTINFO", strerror(errno));
        return 0;
    }

    for (device = interface_list.next; device != &interface_list;
         device = device->next) {
        if (cifaces_list.next != &cifaces_list) {
            flag = 0;
            for (iface = cifaces_list.next; iface != &cifaces_list;
                 iface = iface->next) {
                if (strcmp(device->ifname, iface->ciface) == 0) {
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
                continue;
        }

        sock_opt.ipv6mr_interface = device->devindex;

        if (inet_pton(AF_INET6, ALL_DHCP_RELAY_AND_SERVERS,
                      &sock_opt.ipv6mr_multiaddr) <= 0) {
            TRACE(dump, "%s - %s", dhcp6r_clock(),
                  "failed to set struct for multicast receive\n");
            return 0;
        }

        if (setsockopt(relaysock->sock_desc, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                       (char *) &sock_opt, sizeof(sock_opt)) < 0) {
            TRACE(dump, "%s - %s", dhcp6r_clock(),
                  "failed to set socket option for IPV6_JOIN_GROUP \n");
            return 0;
        }
    }

    TRACE(dump, "%s - %s", dhcp6r_clock(), "socket options are set\n");
    fflush(dump);
    return 1;
}


int fill_addr_struct() {
    memset((char *) &relaysock->from, 0, sizeof(struct sockaddr_in6));
    relaysock->from.sin6_family = AF_INET6;
    relaysock->from.sin6_addr = in6addr_any;
    relaysock->from.sin6_port = htons(SERVER_PORT);

    relaysock->iov[0].iov_base = relaysock->databuf;
    relaysock->iov[0].iov_len = MAX_DHCP_MSG_LENGTH;
    relaysock->msg.msg_name = (void *) &relaysock->from;
    relaysock->msg.msg_namelen = sizeof(relaysock->from);
    relaysock->msg.msg_iov = &relaysock->iov[0];
    relaysock->msg.msg_iovlen = 1;

    relaysock->recvmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo));
    relaysock->recvp = (char *) malloc(relaysock->recvmsglen * sizeof(char));
    relaysock->msg.msg_control = (void *) relaysock->recvp;
    relaysock->msg.msg_controllen = relaysock->recvmsglen;

    if (bind(relaysock->sock_desc, (struct sockaddr *) &relaysock->from,
             sizeof(relaysock->from)) < 0) {
        perror("bind");
        return 0;
    }

    return 1;
}

int recv_data() {
    int count = -1;

    memset(relaysock->databuf, 0, (MAX_DHCP_MSG_LENGTH * sizeof(char)));

    if ((count = recvmsg(relaysock->sock_desc, &relaysock->msg, 0)) < 0) {
        TRACE(dump, "%s - %s", dhcp6r_clock(),
              "failed to receive data with recvmsg()-->receive::recv_data()\n");
        return -1;
    }

    relaysock->buflength = count;

    return 1;
}

int get_interface_info() {
    FILE *f;
    char addr6[40], devname[20];
    struct sockaddr_in6 sap;
    int plen, scope, dad_status, if_idx;
    char addr6p[8][5];
    char src_addr[INET6_ADDRSTRLEN];
    struct interface *device = NULL, *next_device;
    int opaq = OPAQ;
    int sw = 0;
    struct IPv6_address *ipv6addr;

    if ((f = fopen(INTERFACEINFO, "r")) == NULL) {
        TRACE(dump, "fatal error--> could not open file: %s\n",
              INTERFACEINFO);
        return 0;
    }

    while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
                  addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
                  addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
                  &dad_status, devname) != EOF) {
        memset(src_addr, 0, INET6_ADDRSTRLEN);
        sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s", addr6p[0], addr6p[1],
                addr6p[2], addr6p[3], addr6p[4], addr6p[5], addr6p[6],
                addr6p[7]);
        sap.sin6_family = AF_INET6;
        sap.sin6_port = 0;

        if (inet_pton(AF_INET6, addr6, sap.sin6_addr.s6_addr) <= 0)
            return 0;

        if (inet_ntop(AF_INET6, &sap.sin6_addr, src_addr, sizeof(src_addr)) <=
            0)
            return 0;

        if (IN6_IS_ADDR_LOOPBACK(&sap.sin6_addr))
            continue;

        sw = 0;
        for (device = interface_list.next; device != &interface_list;
             device = device->next) {
            if (device->devindex == if_idx) {
                sw = 1;
                break;
            }
        }

        if (sw == 0) {
            opaq += 10;
            device = (struct interface *) malloc(sizeof(struct interface));
            if (device == NULL) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "get_interface_info()--> "
                      "error no more memory available\n");
                exit(1);
            }
            device->opaq = opaq;
            device->ifname = strdup(devname);
            device->devindex = if_idx;
            device->ipv6addr = NULL;
            device->prev = &interface_list;
            device->next = interface_list.next;
            device->prev->next = device;
            device->next->prev = device;
            nr_of_devices += 1;
        }

        if (IN6_IS_ADDR_LINKLOCAL(&sap.sin6_addr)) {
            device->link_local = strdup(src_addr);
            TRACE(dump, "%s %s %s %d %s %s\n",
                  "relay interface info--> devname:", devname, "index:",
                  if_idx, "link_local_addr:", src_addr);
        } else {
            ipv6addr = (struct IPv6_address *)
                malloc(sizeof(struct IPv6_address));
            if (ipv6addr == NULL) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "get_interface_info()--> "
                      "error no more memory available\n");
                exit(1);
            }
            ipv6addr->gaddr = strdup(src_addr);
            ipv6addr->next = NULL;
            if (device->ipv6addr != NULL)
                ipv6addr->next = device->ipv6addr;

            device->ipv6addr = ipv6addr;
        }
    }                           /* while */

    for (device = interface_list.next; device != &interface_list;) {
        next_device = device->next;
        if (device->ipv6addr == NULL) {
            TRACE(dump, "%s - remove interface %s as it does not "
                  "have any global address\n",
                  dhcp6r_clock(), device->ifname);
            --nr_of_devices;
            device->prev->next = device->next;
            device->next->prev = device->prev;
            free(device->ifname);
            if (device->link_local != NULL)
                free(device->link_local);
            free(device);
        }
        device = next_device;
    }

    fflush(dump);
    fclose(f);
    return 1;
}

int send_message() {
    struct sockaddr_in6 sin6;   /* my address information */
    struct msghdr msg;
    uint32_t count = 0;
    struct msg_parser *mesg;
    struct in6_pktinfo *in6_pkt;
    struct cmsghdr *cmsgp;
    char dest_addr[INET6_ADDRSTRLEN];
    struct IPv6_uniaddr *ipv6uni;
    struct interface *iface;
    int hit = 0;
    struct iovec iov[1];
    int recvmsglen;
    char *recvp;
    struct server *uservers;
    struct sifaces *si;

    if ((mesg = get_send_messages_out()) == NULL)
        return 0;

    if (mesg->sent == 1)
        return 0;

    bzero((char *) &sin6, sizeof(struct sockaddr_in6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_flowinfo = 0;
    sin6.sin6_scope_id = 0;

    if (mesg->msg_type == RELAY_REPL) {
        memset(dest_addr, 0, INET6_ADDRSTRLEN);
        memcpy(dest_addr, mesg->peer_addr, INET6_ADDRSTRLEN);

        recvmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo));
        recvp = (char *) malloc(recvmsglen * sizeof(char));
        if (recvp == NULL) {
            TRACE(dump, "error--> recvp no more memory available \n");
            exit(1);
        }
        memset(recvp, 0, recvmsglen);
        cmsgp = (struct cmsghdr *) recvp;
        cmsgp->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
        cmsgp->cmsg_level = IPPROTO_IPV6;
        cmsgp->cmsg_type = IPV6_2292PKTINFO;
        in6_pkt = (struct in6_pktinfo *) CMSG_DATA(cmsgp);
        msg.msg_control = (void *) recvp;
        msg.msg_controllen = recvmsglen;

        /* destination address */
        if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
            TRACE(dump, "%s - %s", dhcp6r_clock(),
                  "send_message()--> inet_pton() failed \n");
            exit(1);
        }
        sin6.sin6_scope_id = mesg->if_index;

        if (mesg->hop > 0)
            sin6.sin6_port = htons(SERVER_PORT);
        else
            sin6.sin6_port = htons(CLIENT_PORT);

        iface = get_interface(mesg->if_index);

        if (iface != NULL) {
            char *src_addr;

            if (IN6_IS_ADDR_LINKLOCAL(&sin6.sin6_addr))
                src_addr = iface->link_local;
            else
                src_addr = iface->ipv6addr->gaddr;

            if (inet_pton(AF_INET6, src_addr, &in6_pkt->ipi6_addr) <= 0) {      /* source 
                                                                                   address 
                                                                                 */
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "inet_pton failed in send_message()\n");
                exit(1);
            }
            TRACE(dump, "%s - source address: %s\n", dhcp6r_clock(),
                  src_addr);
        } else {
            /* the kernel will choose the source address */
            memset(&in6_pkt->ipi6_addr, 0, sizeof(in6_pkt->ipi6_addr));
        }

        /* OUTGOING DEVICE FOR RELAY_REPLY MSG */
        in6_pkt->ipi6_ifindex = mesg->if_index;
        TRACE(dump, "%s - outgoing device index: %d\n", dhcp6r_clock(),
              in6_pkt->ipi6_ifindex);
        TRACE(dump, "%s - destination port: %d\n", dhcp6r_clock(),
              ntohs(sin6.sin6_port));

        iov[0].iov_base = mesg->buffer;
        iov[0].iov_len = mesg->datalength;
        msg.msg_name = (void *) &sin6;
        msg.msg_namelen = sizeof(sin6);
        msg.msg_iov = &iov[0];
        msg.msg_iovlen = 1;

        if ((count = sendmsg(relaysock->sock_desc, &msg, 0)) < 0) {
            perror("sendmsg");
            return 0;
        }

        if (count > MAX_DHCP_MSG_LENGTH)
            perror("bytes in sendmsg");

        TRACE(dump, "%s - *> relay_repl, sent to: %s sent_bytes: %d\n",
              dhcp6r_clock(), dest_addr, count);

        free(recvp);

        mesg->sent = 1;
        return 1;
    }

    if (mesg->msg_type == RELAY_FORW) {
        for (ipv6uni = IPv6_uniaddr_list.next; ipv6uni != &IPv6_uniaddr_list;
             ipv6uni = ipv6uni->next) {
            bzero((char *) &sin6, sizeof(struct sockaddr_in6));
            sin6.sin6_family = AF_INET6;

            memset(dest_addr, 0, INET6_ADDRSTRLEN);
            memcpy(dest_addr, ipv6uni->uniaddr, INET6_ADDRSTRLEN);

            /* destination address */
            if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "inet_pton failed in send_message()\n");
                return 0;
            }

            recvmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo));
            recvp = (char *) malloc(recvmsglen * sizeof(char));
            if (recvp == NULL) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "error--> recvp no more memory available\n");
                exit(1);
            }
            memset(recvp, 0, recvmsglen);

            cmsgp = (struct cmsghdr *) recvp;
            cmsgp->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
            cmsgp->cmsg_level = IPPROTO_IPV6;
            cmsgp->cmsg_type = IPV6_2292PKTINFO;
            in6_pkt = (struct in6_pktinfo *) CMSG_DATA(cmsgp);
            msg.msg_control = (void *) recvp;
            msg.msg_controllen = recvmsglen;

            /* destination address */
            if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "inet_pton failed in send_message()\n");
                return 0;
            }
            sin6.sin6_scope_id = 0;
            sin6.sin6_port = htons(SERVER_PORT);

            /* the kernel will choose the source address */
            memset(&in6_pkt->ipi6_addr, 0, sizeof(in6_pkt->ipi6_addr));
            /* OUTGOING DEVICE FOR RELAY_REPLY MSG */
            in6_pkt->ipi6_ifindex = 0;

            iov[0].iov_base = mesg->buffer;
            iov[0].iov_len = mesg->datalength;
            msg.msg_name = (void *) &sin6;
            msg.msg_namelen = sizeof(sin6);
            msg.msg_iov = &iov[0];
            msg.msg_iovlen = 1;

            if ((count = sendmsg(relaysock->sock_desc, &msg, 0)) < 0) {
                perror("sendmsg");
                return 0;
            }

            if (count > MAX_DHCP_MSG_LENGTH)
                perror("bytes sendmsg");

            TRACE(dump,
                  "%s - => relay_forw, sent to: %s sent_bytes: %d\n",
                  dhcp6r_clock(), dest_addr, count);
            free(recvp);
            hit = 1;
        }                       /* for */

        for (iface = interface_list.next; iface != &interface_list;
             iface = iface->next) {
            uservers = iface->sname;
            while (uservers != NULL) {
                bzero((char *) &sin6, sizeof(struct sockaddr_in6));
                sin6.sin6_family = AF_INET6;

                memset(dest_addr, 0, INET6_ADDRSTRLEN);
                memcpy(dest_addr, uservers->serv, INET6_ADDRSTRLEN);

                /* destination address */
                if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "inet_pton failed in send_message()\n");
                    exit(1);
                }

                recvmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo));
                recvp = (char *) malloc(recvmsglen * sizeof(char));
                if (recvp == NULL) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "error--> recvp no more memory available\n");
                    exit(1);
                }
                memset(recvp, 0, recvmsglen);

                cmsgp = (struct cmsghdr *) recvp;
                cmsgp->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
                cmsgp->cmsg_level = IPPROTO_IPV6;
                cmsgp->cmsg_type = IPV6_2292PKTINFO;
                in6_pkt = (struct in6_pktinfo *) CMSG_DATA(cmsgp);
                msg.msg_control = (void *) recvp;
                msg.msg_controllen = recvmsglen;

                /* destination address */
                if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "inet_pton failed in send_message()\n");
                    return 0;
                }

                in6_pkt->ipi6_ifindex = iface->devindex;
                sin6.sin6_scope_id = in6_pkt->ipi6_ifindex;

                TRACE(dump, "%s - outgoing device index: %d\n",
                      dhcp6r_clock(), in6_pkt->ipi6_ifindex);
                if (inet_pton(AF_INET6, iface->ipv6addr->gaddr, &in6_pkt->ipi6_addr) <= 0) {    /* source 
                                                                                                   address 
                                                                                                 */
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "inet_pton failed in send_message()\n");
                    exit(1);
                }
                TRACE(dump, "%s - source address: %s\n", dhcp6r_clock(),
                      iface->ipv6addr->gaddr);

                sin6.sin6_port = htons(SERVER_PORT);

                iov[0].iov_base = mesg->buffer;
                iov[0].iov_len = mesg->datalength;
                msg.msg_name = (void *) &sin6;
                msg.msg_namelen = sizeof(sin6);
                msg.msg_iov = &iov[0];
                msg.msg_iovlen = 1;

                if ((count = sendmsg(relaysock->sock_desc, &msg, 0)) < 0) {
                    perror("sendmsg");
                    return 0;
                }

                if (count > MAX_DHCP_MSG_LENGTH)
                    perror("bytes sendmsg");

                TRACE(dump,
                      "%s - => relay_forw, sent to: %s sent_bytes: %d\n",
                      dhcp6r_clock(), dest_addr, count);
                free(recvp);
                uservers = uservers->next;
                hit = 1;
            }                   /* while */
        }                       /* Interfaces */

        for (si = sifaces_list.next; si != &sifaces_list; si = si->next) {
            *(mesg->hc_pointer) = MAXHOPCOUNT;
            bzero((char *) &sin6, sizeof(struct sockaddr_in6));
            sin6.sin6_family = AF_INET6;

            memset(dest_addr, 0, INET6_ADDRSTRLEN);
            strcpy(dest_addr, ALL_DHCP_SERVERS);

            /* destination address */
            if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "inet_pton failed in send_message()\n");
                return 0;
            }

            recvmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo));
            recvp = (char *) malloc(recvmsglen * sizeof(char));
            if (recvp == NULL) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "error--> recvp no more memory available\n");
                exit(1);
            }
            memset(recvp, 0, recvmsglen);

            cmsgp = (struct cmsghdr *) recvp;
            cmsgp->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
            cmsgp->cmsg_level = IPPROTO_IPV6;
            cmsgp->cmsg_type = IPV6_2292PKTINFO;
            in6_pkt = (struct in6_pktinfo *) CMSG_DATA(cmsgp);
            msg.msg_control = (void *) recvp;
            msg.msg_controllen = recvmsglen;

            /* destination address */
            if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "inet_pton failed in send_message()\n");
                return 0;
            }

            in6_pkt->ipi6_ifindex = if_nametoindex(si->siface);
            sin6.sin6_scope_id = in6_pkt->ipi6_ifindex;

            TRACE(dump, "%s - outgoing device index: %d\n", dhcp6r_clock(),
                  in6_pkt->ipi6_ifindex);
            iface = get_interface(in6_pkt->ipi6_ifindex);
            if (iface == NULL) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "error--> send_message(), no interface info found\n");
                exit(0);
            }
            if (inet_pton(AF_INET6, iface->ipv6addr->gaddr, &in6_pkt->ipi6_addr) <= 0) {        /* source 
                                                                                                   address 
                                                                                                 */
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "inet_pton failed in send_message()\n");
                exit(1);
            }
            TRACE(dump, "%s - source address: %s\n", dhcp6r_clock(),
                  iface->ipv6addr->gaddr);

            sin6.sin6_port = htons(SERVER_PORT);

            iov[0].iov_base = mesg->buffer;
            iov[0].iov_len = mesg->datalength;
            msg.msg_name = (void *) &sin6;
            msg.msg_namelen = sizeof(sin6);
            msg.msg_iov = &iov[0];
            msg.msg_iovlen = 1;

            if ((count = sendmsg(relaysock->sock_desc, &msg, 0)) < 0) {
                perror("sendmsg");
                return 0;
            }

            if (count > MAX_DHCP_MSG_LENGTH)
                perror("bytes sendmsg");

            TRACE(dump,
                  "%s - => relay_forw, sent to: %s snet_bytes: %d\n",
                  dhcp6r_clock(), dest_addr, count);

            free(recvp);
            hit = 1;
        }                       /* for */

        if (hit == 0) {
            for (iface = interface_list.next; iface != &interface_list;
                 iface = iface->next) {
                if (mesg->interface_in == iface->devindex)
                    continue;

                *(mesg->hc_pointer) = MAXHOPCOUNT;
                bzero((char *) &sin6, sizeof(struct sockaddr_in6));
                sin6.sin6_family = AF_INET6;

                memset(dest_addr, 0, INET6_ADDRSTRLEN);
                strcpy(dest_addr, ALL_DHCP_SERVERS);

                /* destination address */
                if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "inet_pton failed in send_message()\n");
                    return 0;
                }

                recvmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo));
                recvp = (char *) malloc(recvmsglen * sizeof(char));
                if (recvp == NULL) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "error--> recvp no more memory available\n");
                    exit(1);
                }
                memset(recvp, 0, recvmsglen);

                cmsgp = (struct cmsghdr *) recvp;
                cmsgp->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
                cmsgp->cmsg_level = IPPROTO_IPV6;
                cmsgp->cmsg_type = IPV6_2292PKTINFO;
                in6_pkt = (struct in6_pktinfo *) CMSG_DATA(cmsgp);
                msg.msg_control = (void *) recvp;
                msg.msg_controllen = recvmsglen;

                /* destination address */
                if (inet_pton(AF_INET6, dest_addr, &sin6.sin6_addr) <= 0) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "inet_pton failed in send_message()\n");
                    return 0;
                }
                sin6.sin6_port = htons(SERVER_PORT);

                in6_pkt->ipi6_ifindex = iface->devindex;
                sin6.sin6_scope_id = in6_pkt->ipi6_ifindex;

                TRACE(dump, "%s - outgoing device index: %d\n",
                      dhcp6r_clock(), in6_pkt->ipi6_ifindex);
                if (inet_pton(AF_INET6, iface->ipv6addr->gaddr, &in6_pkt->ipi6_addr) <= 0) {    /* source 
                                                                                                   address 
                                                                                                 */
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "inet_pton failed in send_message()\n");
                    exit(1);
                }

                TRACE(dump, "%s - source address: %s\n", dhcp6r_clock(),
                      iface->ipv6addr->gaddr);

                iov[0].iov_base = mesg->buffer;
                iov[0].iov_len = mesg->datalength;
                msg.msg_name = (void *) &sin6;
                msg.msg_namelen = sizeof(sin6);
                msg.msg_iov = &iov[0];
                msg.msg_iovlen = 1;

                if ((count = sendmsg(relaysock->sock_desc, &msg, 0)) < 0) {
                    perror("sendmsg");
                    return 0;
                }

                if (count > MAX_DHCP_MSG_LENGTH)
                    perror("sendmsg");

                TRACE(dump,
                      "%s - => relay_forw, sent to: %s sent_bytes: %d\n",
                      dhcp6r_clock(), dest_addr, count);
                free(recvp);
            }                   /* for */
        }
    }

    fflush(dump);
    mesg->sent = 1;
    return 1;

    TRACE(dump, "%s - %s", dhcp6r_clock(),
          "fatal error--> no message type to be sent!\n");
    exit(1);
}

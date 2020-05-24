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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/param.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# include <time.h>
#endif

#include "dhcp6r.h"
#include "relay6_parser.h"
#include "relay6_socket.h"
#include "relay6_database.h"

#define DHCP6R_PIDFILE "/var/run/dhcpv6/dhcp6r.pid"

FILE *dump;
static char pidfile[MAXPATHLEN];

int main(int argc, char **argv) {
    int err = 0, i;
    int sw = 0;
    int du = 0;
    struct interface *iface;
    struct sockaddr_in6 sin6;
    char *sf, *eth, *addr;
    struct cifaces *ci;
    struct sifaces *si;
    struct IPv6_uniaddr *unia;
    struct server *sa;
    struct msg_parser *mesg;
    FILE *pidfp = NULL;

    strcpy(pidfile, DHCP6R_PIDFILE);

    dump = stderr;
    signal(SIGINT, handler);
    signal(SIGTERM, handler);
    signal(SIGHUP, handler);
    init_relay();

    /* Specify a file stream for logging */
    if (argc > 1) {
        for (i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-d") == 0)
                du = 1;
        }
    }

    if (du == 0) {
        FILE *tmp_dump;

        tmp_dump = fopen(DUMPFILE, "w+");
        if (tmp_dump == NULL) {
            TRACE(dump, "could not write dump file: %s\n", DUMPFILE);
            exit(1);
        }
        dump = tmp_dump;
    }

    if (get_interface_info() == 0)
        goto ERROR;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0) {
            continue;
        } else if (strcmp(argv[i], "-p") == 0) {
            i++;

            if (i < argc) {
                if (strlen(argv[i]) >= MAXPATHLEN) {
                    TRACE(dump, "pid file name is too long\n");
                    exit(1);
                }

                strcpy(pidfile, argv[i]);
            } else {
                command_text();
            }
        } else if (strcmp(argv[i], "-cm") == 0) {
            i++;

            if (get_interface_s(argv[i]) == NULL) {
                err = 5;
                goto ERROR;
            }

            sw = 1;
            ci = (struct cifaces *) malloc(sizeof(struct cifaces));

            if (ci == NULL) {
                TRACE(dump, "%s - %s ", dhcp6r_clock(),
                      "main--> error no more memory available\n");
                exit(1);
            }

            ci->ciface = strdup(argv[i]);
            ci->next = cifaces_list.next;
            cifaces_list.next = ci;

            TRACE(dump, "%s - %s'%s'\n", dhcp6r_clock(),
                  "setting up client interface: ", argv[i]);
            continue;
        } else if (strcmp(argv[i], "-cu") == 0) {
            multicast_off = 1;
        } else if (strcmp(argv[i], "-sm") == 0) {
            i++;

            if (get_interface_s(argv[i]) == NULL) {
                err = 5;
                goto ERROR;
            }

            si = (struct sifaces *) malloc(sizeof(struct sifaces));

            if (si == NULL) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "main--> error no more memory available\n");
                exit(1);
            }

            si->siface = strdup(argv[i]);
            si->next = sifaces_list.next;
            sifaces_list.next = si;

            TRACE(dump, "%s - %s'%s'\n", dhcp6r_clock(),
                  "setting up server interface: ", argv[i]);

            continue;
        } else if (strcmp(argv[i], "-su") == 0) {
            i++;

            /* destination address */
            if (inet_pton(AF_INET6, argv[i], &sin6.sin6_addr) <= 0) {
                err = 3;
                goto ERROR;
            }

            if (IN6_IS_ADDR_UNSPECIFIED(&sin6.sin6_addr)) {
                err = 3;
                goto ERROR;
            }

            unia = (struct IPv6_uniaddr *)
                malloc(sizeof(struct IPv6_uniaddr));

            if (unia == NULL) {
                TRACE(dump, "%s - %s", dhcp6r_clock(),
                      "main--> error no more memory available\n");
                exit(1);
            }

            unia->uniaddr = strdup(argv[i]);
            unia->next = IPv6_uniaddr_list.next;
            IPv6_uniaddr_list.next = unia;

            TRACE(dump, "%s - %s'%s'\n", dhcp6r_clock(),
                  "setting up server address: ", argv[i]);
            nr_of_uni_addr += 1;

            continue;
        } else if (strcmp(argv[i], "-sf") == 0) {
            i++;
            sf = strdup(argv[i]);
            eth = strtok(sf, "+");

            if (eth == NULL) {
                err = 4;
                goto ERROR;
            }

            addr = strtok((sf + strlen(eth) + 1), "\0");

            if (addr == NULL) {
                err = 4;
                goto ERROR;
            }

            /* destination address */
            if (inet_pton(AF_INET6, addr, &sin6.sin6_addr) <= 0) {
                err = 3;
                goto ERROR;
            }

            if (IN6_IS_ADDR_UNSPECIFIED(&sin6.sin6_addr)) {
                err = 3;
                goto ERROR;
            }

            if ((iface = get_interface_s(eth)) != NULL) {
                sa = (struct server *) malloc(sizeof(struct server));

                if (sa == NULL) {
                    TRACE(dump, "%s - %s", dhcp6r_clock(),
                          "main--> no more memory available\n");
                    exit(1);
                }

                sa->serv = strdup(addr);
                sa->next = NULL;

                if (iface->sname != NULL) {
                    sa->next = iface->sname;
                }

                iface->sname = sa;
                TRACE(dump, "%s - %s%s for interface: %s\n",
                      dhcp6r_clock(),
                      "setting up server address: ", addr, eth);
                free(sf);
            } else {
                err = 5;
                goto ERROR;
            }

            continue;
        } else if ((strcmp(argv[i], "-h") == 0) ||
                   (strcmp(argv[i], "--help") == 0)) {
            command_text();
        } else {
            err = 4;
            goto ERROR;
        }
    }

    if (sw == 1)
        multicast_off = 0;

    init_socket();

    if (set_sock_opt() == 0)
        goto ERROR;

    if (fill_addr_struct() == 0)
        goto ERROR;

    if (du == 0) {
        switch (fork()) {
            case 0:
                break;
            case -1:
                perror("fork");
                exit(1);
            default:
                exit(0);
        }
    }

    if ((pidfp = fopen(pidfile, "w+")) == NULL) {
        TRACE(dump, "could not write pid file\n");
        exit(1);
    }

    fprintf(pidfp, "%d\n", getpid());
    fclose(pidfp);

    while (1) {
        if (check_select() == 1) {
            if (recv_data() == 1) {
                if (get_recv_data() == 1) {
                    mesg = create_parser_obj();
                    if (put_msg_in_store(mesg) == 0)
                        mesg->sent = 1; /* mark it for deletion */
                }
            }
        }

        send_message();
        delete_messages();
    }

  ERROR:

    if (err == 3) {
        TRACE(dump, "dhcp6r: malformed address '%s'\n", argv[i]);
        exit(1);
    }

    if (err == 4) {
        TRACE(dump, "dhcp6r: option '%s' not recognized\n", argv[i]);
        exit(1);
    }

    if (err == 5) {
        TRACE(dump, "dhcp6r: interface '%s' does not exist \n", argv[i]);
        exit(1);
    }

    exit(1);
}

void command_text() {
    printf("Usage:\n");
    printf("       dhcp6r [-p pidfile] [-d] [-cu] [-cm <interface>] [-sm <interface>] "
           "[-su <address>] [-sf <interface>+<address>] \n");
    exit(1);
}

char *dhcp6r_clock() {
    time_t tim;
    char *s, *p;

    time(&tim);
    s = ctime(&tim);

    p = s;
    do {
        p = strstr(p, " ");
        if (p != NULL) {
            if (*(p - 1) == '/')
                *p = '0';
            else
                *p = '/';
        }
    } while (p != NULL);

    p = strstr(s, "\n");
    if (p != NULL)
        *p = '\0';

    return s;
}

void handler(int signo) {
    close(relaysock->sock_desc);
    TRACE(dump, "%s - %s", dhcp6r_clock(),
          "RELAY AGENT IS STOPPING............\n");
    fflush(dump);
    unlink(pidfile);

    exit(0);
}

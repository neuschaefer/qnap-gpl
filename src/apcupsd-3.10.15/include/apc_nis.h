/*
 * Include file for apc_nis.c definitions
 *
 */


extern struct sockaddr_in tcp_serv_addr;     /* socket information */
extern int net_errno;                        /* error number -- not yet implemented */
extern char *net_errmsg;                     /* pointer to error message */
extern char net_errbuf[256];                 /* error message buffer for messages */



/* 
 * Receive a message from the other end. Each message consists of
 * two packets. The first is a header that contains the size
 * of the data that follows in the second packet.
 * Returns number of bytes read
 * Returns 0 on end of file
 * Returns -1 on error
 */
int net_recv(int sockfd, char *buff, int maxlen);

/*
 * Send a message over the network. The send consists of
 * two network packets. The first is sends a short containing
 * the length of the data packet which follows.
 * Returns number of bytes sent
 * Returns -1 on error
 */
int net_send(int sockfd, char *buff, int len);

/*     
 * Open a TCP connection to the UPS network server
 * Returns -1 on error
 * Returns socket file descriptor otherwise
 */
int net_open(char *host, char *service, int port);

/* Close the network connection */
void net_close(int sockfd);

/* Wait for and accept a new TCP connection */
int net_accept(int fd, struct sockaddr_in *cli_addr);


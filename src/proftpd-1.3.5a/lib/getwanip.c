#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "getwanip.h"

extern int h_errno;

#if 0
#include <stdarg.h>
static void hugolog(char *msg, ...)
{
    FILE *dout;
    char buf[2048];
    va_list args;

    va_start(args, msg);
    vsnprintf(buf, sizeof(buf) - 1, msg, args);
    va_end(args);

    dout = fopen("/tmp/ftp.debug", "a");
    if (dout) {
      fputs(buf, dout);
      fclose(dout);
    }
}
#endif

int is_domain_name( char *name )
{
    char s[1024];
    strcpy( s, name );
    char *p, *ptr;
    p=strtok_r( s, ".", &ptr );
    if(!p)return 0;
    if( !isalpha(*p) )return 0;
    while((p=strtok_r( NULL, ".", &ptr )))
    {
        if( !isalpha(*p) )return 0;
    }
    return 1;
}

static int is_number(char *str)
{
	int len = strlen(str) - 1;
	
	while (len >= 0) {
		if (!isdigit(str[len]))
			return 0;
		len--;
	}
	return 1;
}

int is_valid_ip(char *ip)
{
	int count = 1;
	int zero_count = 0;
	int num;
	char *p = strtok(ip, ".");

	if (p && strlen(p) <= 3 ) {
		if (!is_number(p))
			return 0;
		num = atoi(p);
		if (num < 0 || num > 255)
			return 0;
		else if (num == 0)
			return 0;
	}
	while ((p = strtok(NULL, "."))) {
		count++;
		if (!is_number(p))
			return 0;
		num = atoi(p);
		if (num < 0 || num > 255)
			return 0;
		else if (num == 0)
			zero_count++;
		
		if (count > 4)
			return 0;
	}
	if (count == 4)
		return 1;
	return 0;
}

static int sock=-1;
static struct sockaddr_in name;

int resolve_ip( char *ip, char *name )
{
	struct hostent *host=gethostbyname(name);
	unsigned char *a;
	if(!host ) {
		return 0;
	}
	a=(unsigned char *)host->h_addr;
	sprintf( ip, "%u.%u.%u.%u", a[0], a[1], a[2], a[3] );

	return 1;
}

static void set_address( char *host, int port )
{
	sock=socket( AF_INET,SOCK_STREAM, 0 );
	name.sin_family = AF_INET;
	name.sin_port = htons((short)port);
	char host_ip[32];

	if( !host ) {
		name.sin_addr.s_addr = INADDR_ANY;
		close( sock );
                sock = -1;
		return;
	}

	if( !resolve_ip( host_ip, host ) ) {
		close( sock );
		sock = -1;
		return;
	}

	if( host_ip ) {
		name.sin_addr.s_addr = inet_addr(host_ip);
	}
}

static int SendReq( char *data, int len )
{
	int slen;
	
	if( sock<0 )
		return 0;

	while( len>0 ) {
		slen= write( sock, data, len );
		if( slen<0 ) {
			close( sock );
			sock=-1;
			return 0;
		}
		data+=slen;
		len-=slen;
	}
	return 1;
}

static int ReadLine( unsigned char *data, int len )
{
	int timeout_val=5;
	struct timeval timeout;
	int count=0;
	int is_blocking=0;
	int slen;
	fd_set rd_fdset;
	timeout.tv_sec=timeout_val;
	timeout.tv_usec=0;

	while( len>0 ) {
		if( is_blocking ) {
			FD_ZERO(&rd_fdset);
			FD_SET( sock, &rd_fdset );
			if( (select(FD_SETSIZE,&rd_fdset,NULL,NULL, timeout_val==0?0:&timeout))<=0 )
				return -1;
		}
		slen= read( sock, data, 1 );
		if( slen<0 ) {
			if( is_blocking ) {
				close( sock );
				sock=-1;
				return -1;
			}
			else {
				if( errno==EAGAIN )
					return 0;
				else {
					close( sock );
					sock=-1;
					return -1;
				}
			}
        }
		else if( slen==0  ) {
			return count;
		}
		count++;
		if( *data==(unsigned char)'\n' )
			return count;
		data++;
		len--;
	}
	return count;
}

static unsigned int get_host_address( char *address )
{
	struct hostent *host=gethostbyname(address);
	if(!host )return 0;
	return *((unsigned int *) (host->h_addr));
}

static int parse_ip(char *line, char *ip)
{
	char *p = line;
	char *p_ip = ip;
	
	int find[7] = { 0, 0, 0, 0, 0, 0, 0 };
	int m[4] = { 0, 0, 0, 0 };
	int count = 0;
	
	while (*p != 0) {
		if ( *p >= '0' && *p <= '9' ) {
			m[0] = 1;
			if (find[5] == 1)
				m[3] = 1;
			else if (find[3] == 1)
				m[2] = 1;
			else if (find[1] == 1)
				m[1] = 1;
				
			if ( m[3] == 1 ) {
				count++;
				*ip = *p;
				ip++;
				if (count > 3) {
					printf("error: %s\n", line);
				}
			}
			else if ( m[2] == 1 ) {
				count++;
				if (count <= 3) {
					*ip = *p;
					ip++;
				}
			}
			else if ( m[1] == 1 ) {
				count++;
				if (count <= 3) {
					*ip = *p;
					ip++;
				}
			}
			else if ( m[0] == 1 ) {
				count++;
				if (count <= 3) {
					*ip = *p;
					ip++;
				}
			}
		}
		else if ( *p == '.' ) {
			count = 0;
			if (m[3] == 1)
				printf("error 2: %s\n", line);
			else if (m[2] == 1) {
				find[5] = 1;
				*ip = '.';
				ip++;
			}
			else if (m[1] == 1) {
				find[3] = 1;
				*ip = '.';
				ip++;
			}
			else if (m[0] == 1) {
				find[1] = 1;
				*ip = '.';
				ip++;
			}
		}
		else {
			if ( m[3] == 1 && count <= 3) {
				*ip = 0;
				return 1;
			}
			else if ( m[0] == 1 ) {
				m[0] = m[1] = m[2] = m[3] = 0;
				count = 0;
				find[0] = find[1] = find[2] = find[3] = find[4] = find[5] = find[6] = 0;
				ip = p_ip;
			}
		}
		p++;
	}
	return 0;
	
}

#define GIP_URL1        "checkip.dyndns.org"
#define GIP_URL2        "www.edpsciences.com"
#define GIP_URL3		"myipaddress.co.uk"

static char *all_hosts[]={ GIP_URL1,
			GIP_URL2,
			GIP_URL3};

static char *sub_dir[]={ "/",
			"/htbin/ipaddress",
			"/"};

static char *setting[]={ "",
			"",
			"Accept: */*\r\nAccept-Language: zh-tw\r\nAccept-Encoding: gzip, deflate\r\nHost: myipaddress.co.uk\r\nConnection: Keep-Alive\r\n"};

int get_external_ip( char * wan_ip)
{
#if 0 // Bug#84824
	int i;
	char req[128];
	char reply[1024+1];
	int len = 0;
	int count=10;
#endif
	int last_ch = 0;
	FILE *fp = NULL;
	
#if 0 // Bug#84824
//	hugolog("start get wan ip\n");
	
	for( i=0; i<sizeof(all_hosts)/sizeof(unsigned char *);i++) {
//		hugolog("i = %d, host = %s, sub_req = %s\n", i, all_hosts[i], sub_dir[i]);
		set_address( all_hosts[i], 80 );
//		hugolog("sock = %d\n", sock);
		if( sock<0 )
			continue;
			
		if(connect( sock, (struct sockaddr *)&name, sizeof(name))) {
//			hugolog("connect fail\n");
			close(sock);
			sock=-1;
			continue;
		}
		sprintf( req, "GET %s HTTP/1.1\r\n%s\r\n", sub_dir[i], setting[i]);
//		printf("req: %s", req);
//		hugolog("req: %s\n", req);
		if(!SendReq( req, strlen(req))) {
			close(sock);
			sock = -1;
//			hugolog("send req fail\n");
			continue;
		}
		
		while(count>0 && (len=ReadLine( (unsigned char *)reply, 1024 ))==0) {
			count--;
			usleep(500000);
//			hugolog("read zero... sleep, count=%d\n", count);
			continue;
		}
		if( len<=0 ) {
			close(sock);
			sock = -1;
//			hugolog("fail to read\n");
			continue;
		}
		reply[len] = '\0';
		
//		printf("reply:[%s]", reply);
//		hugolog(reply);
		if(!strncmp( &reply[9], "200", 3)) {
			while((len=ReadLine( (unsigned char *)reply, 1024 ))>2) {
				reply[len]='\0';
//				printf("2reply:[%s]\n", reply);
//				hugolog("2reply:[%s]\n", reply);
			}
		}
		else {
//			hugolog("http reply is not 200\n");
		}
		while((len=ReadLine( (unsigned char *)reply, 1024 ))>0) {
			reply[len]='\0';
//			printf("3reply:[%s]\n", reply);
//			hugolog(reply);
			if (parse_ip(reply, wan_ip)) {
//				hugolog("get wanip:[%s]\n", wan_ip);
				close( sock );
				sock = -1;
//hugolog("get_external_ip succ exit\n");
				return 1;
				break;
			}
		}
		close(sock);
		sock = -1;
    	}
#endif

	fp = popen("/etc/init.d/get_external_ip.sh", "r");
	if (fp) {
        char eip[17] = {0};
		fread(eip, 1, 16, fp);
		pclose(fp);
		last_ch = strlen(eip) - 1;
		while(last_ch > 6 && eip[last_ch] < '0')
			eip[last_ch--] = 0;
        strncpy(wan_ip, eip, 16);
        if(last_ch < 6 || !is_valid_ip(eip))
            return 0;
		return 1;
	}

//hugolog("get_external_ip fail exit\n");
    	return 0;
}

#if 0
int main( void )
{
	char ip[32];	
	get_external_ip(ip);
	return 0;
}
#endif




/**************************************************************************
*   Copyright (C) 2005 by Achal Dhir                                      *
*   achaldhir@gmail.com                                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

// tftpserver.cpp
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <map>
using namespace std;
#include "opentftpd.h"

//Global Variables
bool kRunning = true;
myMap tftpCache;
myMultiMap tftpAge;
bool verbatim = false;
char iniFile[1024]="";
char logFile[1024]="";
char fileSep = '/';
char notFileSep = '\\';
WORD blksize = 65464;
WORD timeout = 3;
data2 cfig;
char tempbuff[256];
char logBuff[512];
char sVersion[] = "TFTP Server SinglePort Version 1.62 Unix Built 1621";
packet* datain;

int file_size;


#ifdef	QNAP_SERVICE_BINDING

// Constants for service binding feature.
#define	SZP_GET_BIND_ADDR		"/sbin/getbindaddr"
#define	SZ_INADDR_ANY			"0.0.0.0"

#ifndef	TRUE
#define	TRUE		1
#endif
#ifndef	FALSE
#define	FALSE		0
#endif

// The structure for binding address information.
typedef	struct _T_BIND_ADDR_
{
	int		bEnabled;
	int		cnIpv4;
	int		cnIpv6;
	char	**ryszIpv4;
	char	**ryszIpv6;
} T_BIND_ADDR, *PT_BIND_ADDR;

// A flag indicating whether if receiving a HUP signal.
int  g_bHup = FALSE;

// The global buffer to cache service binding address. 
T_BIND_ADDR g_tBindAddr;
static void Release_Bind_Addr(PT_BIND_ADDR ptbaAddr)
{
	if(!ptbaAddr) return;

	if(ptbaAddr->ryszIpv4)
	{
		int idx;
		for(idx=0; idx<ptbaAddr->cnIpv4; idx++)
			free(ptbaAddr->ryszIpv4[idx]);
		free(ptbaAddr->ryszIpv4);
		ptbaAddr->ryszIpv4 = NULL;
	}
	if(ptbaAddr->ryszIpv6)
	{
		int idx;
		for(idx=0; idx<ptbaAddr->cnIpv6; idx++)
			free(ptbaAddr->ryszIpv6[idx]);
		free(ptbaAddr->ryszIpv6);
		ptbaAddr->ryszIpv6 = NULL;
	}
}

/**
 * \brief	Get the service binding address.
 * \param	ptBindAddr	The buffer to the service binding settings.
 * \return	0 if function success, or -XXX if error.
 */
static int Get_Binding_Address(PT_BIND_ADDR ptBindAddr)
{
	int  iRet=-ENOMEM, cbAddr;
	char  szLine[256];
	FILE  *pfAddr = NULL;
	struct stat  tStat;

	// Reset the inet address first.
	ptBindAddr->bEnabled = FALSE;
	ptBindAddr->cnIpv4 = ptBindAddr->cnIpv6 = 0;
	Release_Bind_Addr(ptBindAddr);
	// Just report no binding address if the getbindaddr is not existed.
	if (-1 == stat(SZP_GET_BIND_ADDR, &tStat))  return 0;
	if (NULL == (pfAddr = popen(SZP_GET_BIND_ADDR "  TFTP -noipv6 -list 2>/dev/null", "r")))  return 0;
	while (NULL != fgets(szLine, sizeof(szLine), pfAddr))
	{
		szLine[sizeof(szLine)-1] = '\0';
		if (0 >= (cbAddr = strlen(szLine)))  continue;
		if ('\n' == szLine[cbAddr-1])  szLine[cbAddr-1] = '\0';
		if (NULL != strchr(szLine, ':'))
		{
			if(NULL == (ptBindAddr->ryszIpv6 = (char**)realloc(ptBindAddr->ryszIpv6, (++ptBindAddr->cnIpv6)*sizeof(char*))))
			{
				ptBindAddr->cnIpv6 = 0;
				goto exit;
			}
			if(NULL == (ptBindAddr->ryszIpv6[ptBindAddr->cnIpv6-1] = (char*)malloc(INET6_ADDRSTRLEN)))
			{
				ptBindAddr->cnIpv6 = 0;
				goto exit;
			}
			strcpy(ptBindAddr->ryszIpv6[ptBindAddr->cnIpv6-1], szLine);
		}
		else if (NULL != strchr(szLine, '.'))
		{
			if (strstr(SZ_INADDR_ANY, szLine))
			{
				ptBindAddr->bEnabled = FALSE;
				ptBindAddr->cnIpv4 = ptBindAddr->cnIpv6 = 0;
				break;
			}
			if(NULL == (ptBindAddr->ryszIpv4 = (char**)realloc(ptBindAddr->ryszIpv4, (++ptBindAddr->cnIpv4)*sizeof(char*)*INET6_ADDRSTRLEN)))
			{
				ptBindAddr->cnIpv4 = 0;
				goto exit;
			}
			if(NULL == (ptBindAddr->ryszIpv4[ptBindAddr->cnIpv4-1] = (char*)malloc(INET_ADDRSTRLEN)))
			{
				ptBindAddr->cnIpv4 = 0;
				goto exit;
			}
			strcpy(ptBindAddr->ryszIpv4[ptBindAddr->cnIpv4-1], szLine);
		}
	}
	iRet = 0;
exit:
	if(iRet != 0)
		Release_Bind_Addr(ptBindAddr);
	pclose(pfAddr);
	ptBindAddr->bEnabled = ptBindAddr->cnIpv4 || ptBindAddr->cnIpv6;
	return 0;
}

void Signal_Hup_Handler(int sig_num)
{
	g_bHup = true;
}

#endif	//QNAP_SERVICE_BINDING


unsigned long  get_file_length ( FILE * fileName)
{
     unsigned long pos = ftell(fileName);
     unsigned long len = 0;
   
     fseek ( fileName, 0L, SEEK_END );
     len = ftell ( fileName );
     fseek ( fileName, pos, SEEK_SET );
     return len;
}

int check_log_file_size(FILE * fileName)
{
	if(get_file_length(fileName)>=1024*1023){
		rename(logFile, "opentftpd.old");
		cfig.logfile = fopen(logFile, "wt");
	}
	return 0;
}

int main(int argc, char **argv)
{
	signal(SIGINT, catch_int);
	signal(SIGABRT, catch_int);
	signal(SIGTERM, catch_int);
	signal(SIGQUIT, catch_int);
	signal(SIGTSTP, catch_int);

#ifndef	QNAP_SERVICE_BINDING
	signal(SIGHUP, catch_int);
#else	//QNAP_SERVICE_BINDING
	signal(SIGHUP, Signal_Hup_Handler);
	memset(&g_tBindAddr, 0, sizeof(T_BIND_ADDR));
#endif	//QNAP_SERVICE_BINDING

    logBuff[0] = 0;

    for (int i = 1; i < argc; i++)
    {
        if (!strcasecmp(argv[i], "-v"))
            verbatim = true;
        else if (!strcmp(argv[i], "-i") && argc > i + 1 && argv[i + 1][0] != '-' )
        {
            myTrim(iniFile, argv[i + 1]);
            i++;
        }
        else if (!strcmp(argv[i], "-l") && argc > i + 1 && argv[i + 1][0] != '-' )
        {
            myTrim(logFile, argv[i + 1]);
            i++;
        }
        else if (!strncasecmp(argv[i], "-i", 2))
            myTrim(iniFile, argv[i] + 2);
        else if (!strncasecmp(argv[i], "-l", 2))
            myTrim(logFile, argv[i] + 2);
        else
            sprintf(logBuff, "Error: Invalid Argument %s", argv[i]);
    }

	if (!iniFile[0])
		strcpy(iniFile,"/etc/opentftpd.ini");

	if (verbatim)
	{
		if (logBuff[0])
		{
			printf("%s\n", logBuff);
			exit(EXIT_FAILURE);
		}

		init();
		timeval tv;
		fd_set readfds;
		request req;
		datain = (packet*)calloc(1, blksize + 4);
		int fdsReady = 0;

		if (!datain)
		{
			sprintf(logBuff,"Memory Error");
			logMess(logBuff, 0);
			exit(1);
		}

		printf("\nAccepting requests..\n");

		do
		{
			FD_ZERO(&readfds);

			tv.tv_sec = 1;
			tv.tv_usec = 0;

			for (int i = 0; i < MAX_SERVERS && cfig.tftpConn[i].port; i++)
				FD_SET(cfig.tftpConn[i].sock, &readfds);

			fdsReady = select(cfig.maxFD, &readfds, NULL, NULL, &tv);

			//if (errno)
			//	printf("%s\n", strerror(errno));

			for (int i = 0; fdsReady > 0 && i < MAX_SERVERS && cfig.tftpConn[i].port; i++)
			{
				if (FD_ISSET(cfig.tftpConn[i].sock, &readfds))
				{
					fdsReady--;
					memset(&req, 0, sizeof(request));
					memset(datain, 0, blksize + 4);
					req.clientsize = sizeof(req.client);
					req.sockInd = i;
					errno = 0;
					req.bytesRecd = recvfrom(cfig.tftpConn[req.sockInd].sock, (char*)datain, blksize + 4, 0, (sockaddr*)&req.client, &req.clientsize);

					if (req.bytesRecd < 4 || errno)
						continue;

					//printf("%u=%u\n", req.bytesRecd, blksize + 4);
					sprintf(req.mapname, "%s:%u", inet_ntoa(req.client.sin_addr), ntohs(req.client.sin_port));
					request *req1 = tftpCache[req.mapname];

					if (!req1)
						tftpCache.erase(req.mapname);

					//printf("%u\n",req1);

					if (req1)
					{
						req1->bytesRecd = req.bytesRecd;

						if (ntohs(datain->opcode) == 3 && req1->opcode == 2)
						{
							if ((WORD)req1->bytesRecd  <= req1->blksize + 4)
							{
								if (req1->attempt <= 3)
								{
									req1->tblock = req1->block + 1;

									if (ntohs(datain->block) == req1->tblock)
									{
										req1->block = req1->tblock;
										req1->fblock++;
										req1->attempt = 0;
										req1->acout.opcode = htons(4);
										req1->acout.block = ntohs(req1->block);
										processRecv(req1);
									}
								}
							}
							else
							{
								req1->serverError.opcode = htons(5);
								req1->serverError.errorcode = htons(4);
								sendto(cfig.tftpConn[req1->sockInd].sock, (const char*)&req1->serverError, strlen(req1->serverError.errormessage) + 5, 0, (sockaddr*)&req1->client, req1->clientsize);
								sprintf(req1->serverError.errormessage, "Error: Incoming Packet too large");
								logMess(req1, 1);
								cleanReq(req1);
							}
						}
						else if (ntohs(datain->opcode) == 4 && req1->opcode == 1)
						{
							if (req1->bytesRecd >= 4)
							{
								if (req1->attempt <= 3)
								{
									if (ntohs(datain->block) == req1->block)
									{
										req1->block++;
										req1->fblock++;
										req1->attempt = 0;
										processSend(req1);
									}
								}
							}
						}
						else if (req1->bytesRecd > (int)sizeof(message))
						{
							req1->serverError.opcode = htons(5);
							req1->serverError.errorcode = htons(4);
							sendto(cfig.tftpConn[req1->sockInd].sock, (const char*)&req1->serverError, strlen(req1->serverError.errormessage) + 5, 0, (sockaddr*)&req1->client, req1->clientsize);
							sprintf(req1->serverError.errormessage, "Error: Incoming Packet too large");
							logMess(req1, 1);
							cleanReq(req1);
						}
						else if (ntohs(datain->opcode) == 1 || ntohs(datain->opcode) == 2)
						{
							cleanReq(req1);
							if (!processNew(&req))
							{
								memcpy(req1, &req, sizeof(request));
							}
						}
						else if (ntohs(datain->opcode) == 5)
						{
							sprintf(req1->serverError.errormessage, "Error %i at Client, %s", ntohs(datain->block), &datain->buffer);
							logMess(req1, 1);
							cleanReq(req1);
						}
						else
						{
							req1->serverError.opcode = htons(5);
							req1->serverError.errorcode = htons(4);
							sprintf(req1->serverError.errormessage, "Unexpected Option Code %u", ntohs(datain->opcode));
							sendto(cfig.tftpConn[req1->sockInd].sock, (const char*)&req1->serverError, strlen(req1->serverError.errormessage) + 5, 0, (sockaddr*)&req1->client, req1->clientsize);
							logMess(req1, 1);
							cleanReq(req1);
						}
					}
					else if (req.bytesRecd > (int)sizeof(message))
					{
						req.serverError.opcode = htons(5);
						req.serverError.errorcode = htons(4);
						sprintf(req.serverError.errormessage, "Error: Incoming Packet too large");
						sendto(cfig.tftpConn[i].sock, (const char*)&req.serverError, strlen(req.serverError.errormessage) + 5, 0, (sockaddr*)&req.client, req.clientsize);
						logMess(&req, 1);
					}
					else if (ntohs(datain->opcode) == 5)
					{
						sprintf(req.serverError.errormessage, "Error %i at Client, %s", ntohs(datain->block), &datain->buffer);
						logMess(&req, 1);
					}
					else if (ntohs(datain->opcode) != 1 && ntohs(datain->opcode) != 2)
					{
						req.serverError.opcode = htons(5);
						req.serverError.errorcode = htons(5);
						sprintf(req.serverError.errormessage, "Unknown transfer ID");
						sendto(cfig.tftpConn[i].sock, (const char*)&req.serverError, strlen(req.serverError.errormessage) + 5, 0, (sockaddr*)&req.client, req.clientsize);
						logMess(&req, 1);
					}
					else if (!processNew(&req))
					{
						request *req1 = (request*)calloc(1, sizeof(request));

						if (!req1)
						{
							sprintf(logBuff,"Memory Error");
							logMess(logBuff, 1);
							continue;
						}

						memcpy(req1, &req, sizeof(request));
						tftpCache[req1->mapname] = req1;
						tftpAge.insert(pair<long, request*>(req1->expiry, req1));
					}
				}
			}

			myMultiMap::iterator p = tftpAge.begin();
			myMultiMap::iterator q;
			time_t currentTime = time(NULL);
			request *req;

			while (p != tftpAge.end())
			{
				req = p->second;

				if (p->first > currentTime)
					break;
				else if (p->first < req->expiry && req->expiry > currentTime)
				{
					q = p;
					p++;
					tftpAge.erase(q);
					tftpAge.insert(pair<long, request*>(req->expiry, req));
				}
				else if (req->expiry <= currentTime && req->attempt >= 3)
				{
					if (req->attempt < UCHAR_MAX)
					{
						req->serverError.opcode = htons(5);
						req->serverError.errorcode = htons(0);

						if (req->fblock && !req->block)
							strcpy(req->serverError.errormessage, "Large File, Block# Rollover not supported by Client");
						else
							strcpy(req->serverError.errormessage, "Timeout");

						sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
						logMess(req, 1);
					}

					q = p;
					p++;
					tftpAge.erase(q);
					tftpCache.erase(req->mapname);
					cleanReq(req);
					free(req);
				}
				else if (req->expiry <= currentTime)
				{
					if (ntohs(req->acout.opcode) == 3)
					{
						if (processSend(req))
							cleanReq(req);
						else
						{
							req->attempt++;
							req->expiry = currentTime + req->timeout;
						}
					}
					else
					{
						errno = 0;
						sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->acout, req->bytesReady, 0, (sockaddr*)&req->client, req->clientsize);
						//errno = WSAGetLastError();

						if (errno)
							cleanReq(req);
						else
						{
							req->attempt++;
							req->expiry = currentTime + req->timeout;
						}
					}
					p++;
				}
				else
					p++;
			}
		}
		while (kRunning);
	}
	else
	{
		if(logBuff[0])
		{
			syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_CRIT), logBuff);
			exit(EXIT_FAILURE);
		}

		/* Our process ID and Session ID */
		pid_t pid, sid;

		/* Fork off the parent process */
		pid = fork();
		if (pid < 0)
		{
			exit(EXIT_FAILURE);
		}
		/* If we got a good PID, then
		we can exit the parent process. */
		if (pid > 0)
		{
			exit(EXIT_SUCCESS);
		}

		/* Change the file mode mask */
		umask(0);

		/* Open any logs here */

		/* Create a new SID for the child process */
		sid = setsid();
		if (sid < 0)
		{
			/* Log the failure */
			exit(EXIT_FAILURE);
		}

		/* Change the current working directory */
		if ((chdir("/")) < 0)
		{
			/* Log the failure */
			exit(EXIT_FAILURE);
		}

		/* Close out the standard file descriptors */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		/* Daemon-specific initialization goes here */
		//Initialize
		init();
		timeval tv;
		fd_set readfds;
		request req;
		datain = (packet*)calloc(1, blksize + 4);
		int fdsReady = 0;

		if (!datain)
		{
			sprintf(logBuff,"Memory Error");
			logMess(logBuff, 0);
			exit(EXIT_FAILURE);
		}

		do
		{
		#ifdef	QNAP_SERVICE_BINDING
			if (g_bHup)
			{
				g_bHup = FALSE;
				closeConn(FALSE);
				init();
			}
		#endif	//QNAP_SERVICE_BINDING

			FD_ZERO(&readfds);

			tv.tv_sec = 1;
			tv.tv_usec = 0;

			for (int i = 0; i < MAX_SERVERS && cfig.tftpConn[i].port; i++)
				FD_SET(cfig.tftpConn[i].sock, &readfds);

			fdsReady = select(cfig.maxFD, &readfds, NULL, NULL, &tv);

			//if (errno)
			//	printf("%s\n", strerror(errno));

			for (int i = 0; fdsReady > 0 && i < MAX_SERVERS && cfig.tftpConn[i].port; i++)
			{
				if (FD_ISSET(cfig.tftpConn[i].sock, &readfds))
				{
					fdsReady--;
					memset(&req, 0, sizeof(request));
					memset(datain, 0, blksize + 4);
					req.clientsize = sizeof(req.client);
					req.sockInd = i;
					errno = 0;
					req.bytesRecd = recvfrom(cfig.tftpConn[req.sockInd].sock, (char*)datain, blksize + 4, 0, (sockaddr*)&req.client, &req.clientsize);

					if (req.bytesRecd < 4 || errno)
						continue;

					//printf("%u=%u\n", req.bytesRecd, blksize + 4);
					sprintf(req.mapname, "%s:%u", inet_ntoa(req.client.sin_addr), ntohs(req.client.sin_port));
					request *req1 = tftpCache[req.mapname];

					if (!req1)
						tftpCache.erase(req.mapname);

					//printf("%u\n",req1);

					if (req1)
					{
						req1->bytesRecd = req.bytesRecd;

						if (ntohs(datain->opcode) == 3 && req1->opcode == 2)
						{
							if ((WORD)req1->bytesRecd  <= req1->blksize + 4)
							{
								if (req1->attempt <= 3)
								{
									req1->tblock = req1->block + 1;

									if (ntohs(datain->block) == req1->tblock)
									{
										req1->block = req1->tblock;
										req1->fblock++;
										req1->attempt = 0;
										req1->acout.opcode = htons(4);
										req1->acout.block = ntohs(req1->block);
										processRecv(req1);
									}
								}
							}
							else
							{
								req1->serverError.opcode = htons(5);
								req1->serverError.errorcode = htons(4);
								sendto(cfig.tftpConn[req1->sockInd].sock, (const char*)&req1->serverError, strlen(req1->serverError.errormessage) + 5, 0, (sockaddr*)&req1->client, req1->clientsize);
								sprintf(req1->serverError.errormessage, "Error: Incoming Packet too large");
								logMess(req1, 1);
								cleanReq(req1);
							}
						}
						else if (ntohs(datain->opcode) == 4 && req1->opcode == 1)
						{
							if (req1->bytesRecd >= 4)
							{
								if (req1->attempt <= 3)
								{
									if (ntohs(datain->block) == req1->block)
									{
										req1->block++;
										req1->fblock++;
										req1->attempt = 0;
										processSend(req1);
									}
								}
							}
						}
						else if (req1->bytesRecd > (int)sizeof(message))
						{
							req1->serverError.opcode = htons(5);
							req1->serverError.errorcode = htons(4);
							sendto(cfig.tftpConn[req1->sockInd].sock, (const char*)&req1->serverError, strlen(req1->serverError.errormessage) + 5, 0, (sockaddr*)&req1->client, req1->clientsize);
							sprintf(req1->serverError.errormessage, "Error: Incoming Packet too large");
							logMess(req1, 1);
							cleanReq(req1);
						}
						else if (ntohs(datain->opcode) == 1 || ntohs(datain->opcode) == 2)
						{
							cleanReq(req1);
							if (!processNew(&req))
							{
								memcpy(req1, &req, sizeof(request));
							}
						}
						else if (ntohs(datain->opcode) == 5)
						{
							sprintf(req1->serverError.errormessage, "Error %i at Client, %s", ntohs(datain->block), &datain->buffer);
							logMess(req1, 1);
							cleanReq(req1);
						}
						else
						{
							req1->serverError.opcode = htons(5);
							req1->serverError.errorcode = htons(4);
							sprintf(req1->serverError.errormessage, "Unexpected Option Code %u", ntohs(datain->opcode));
							sendto(cfig.tftpConn[req1->sockInd].sock, (const char*)&req1->serverError, strlen(req1->serverError.errormessage) + 5, 0, (sockaddr*)&req1->client, req1->clientsize);
							logMess(req1, 1);
							cleanReq(req1);
						}
					}
					else if (req.bytesRecd > (int)sizeof(message))
					{
						req.serverError.opcode = htons(5);
						req.serverError.errorcode = htons(4);
						sprintf(req.serverError.errormessage, "Error: Incoming Packet too large");
						sendto(cfig.tftpConn[i].sock, (const char*)&req.serverError, strlen(req.serverError.errormessage) + 5, 0, (sockaddr*)&req.client, req.clientsize);
						logMess(&req, 1);
					}
					else if (ntohs(datain->opcode) == 5)
					{
						sprintf(req.serverError.errormessage, "Error %i at Client, %s", ntohs(datain->block), &datain->buffer);
						logMess(&req, 1);
					}
					else if (ntohs(datain->opcode) != 1 && ntohs(datain->opcode) != 2)
					{
						req.serverError.opcode = htons(5);
						req.serverError.errorcode = htons(5);
						sprintf(req.serverError.errormessage, "Unknown transfer ID");
						sendto(cfig.tftpConn[i].sock, (const char*)&req.serverError, strlen(req.serverError.errormessage) + 5, 0, (sockaddr*)&req.client, req.clientsize);
						logMess(&req, 1);
					}
					else if (!processNew(&req))
					{
						request *req1 = (request*)calloc(1, sizeof(request));

						if (!req1)
						{
							sprintf(logBuff,"Memory Error");
							logMess(logBuff, 1);
							continue;
						}

						memcpy(req1, &req, sizeof(request));
						tftpCache[req1->mapname] = req1;
						tftpAge.insert(pair<long, request*>(req1->expiry, req1));
					}
				}
			}

			myMultiMap::iterator p = tftpAge.begin();
			myMultiMap::iterator q;
			time_t currentTime = time(NULL);
			request *req;

			while (p != tftpAge.end())
			{
				req = p->second;

				if (p->first > currentTime)
					break;
				else if (p->first < req->expiry && req->expiry > currentTime)
				{
					q = p;
					p++;
					tftpAge.erase(q);
					tftpAge.insert(pair<long, request*>(req->expiry, req));
				}
				else if (req->expiry <= currentTime && req->attempt >= 3)
				{
					if (req->attempt < UCHAR_MAX)
					{
						req->serverError.opcode = htons(5);
						req->serverError.errorcode = htons(0);

						if (req->fblock && !req->block)
							strcpy(req->serverError.errormessage, "Large File, Block# Rollover not supported by Client");
						else
							strcpy(req->serverError.errormessage, "Timeout");

						sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
						logMess(req, 1);
					}

					q = p;
					p++;
					tftpAge.erase(q);
					tftpCache.erase(req->mapname);
					cleanReq(req);
					free(req);
				}
				else if (req->expiry <= currentTime)
				{
					if (ntohs(req->acout.opcode) == 3)
					{
						if (processSend(req))
							cleanReq(req);
						else
						{
							req->attempt++;
							req->expiry = currentTime + req->timeout;
						}
					}
					else
					{
						errno = 0;
						sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->acout, req->bytesReady, 0, (sockaddr*)&req->client, req->clientsize);
						//errno = WSAGetLastError();

						if (errno)
							cleanReq(req);
						else
						{
							req->attempt++;
							req->expiry = currentTime + req->timeout;
						}
					}
					p++;
				}
				else
					p++;
			}
		}
		while (kRunning);

		closeConn();
	}
}

void closeConn(int bExit)
{
	if (bExit)
		kRunning = false;
	sprintf(logBuff, "Closing Network Connections...");
	logMess(logBuff, 1);

	for (int i = 0; i < MAX_SERVERS && cfig.tftpConn[i].server; i++)
		close(cfig.tftpConn[i].sock);

	sprintf(logBuff, "TFTP Server Stopped !");
	logMess(logBuff, 1);

	if (cfig.logfile)
		fclose(cfig.logfile);

	if (bExit)
		exit(EXIT_SUCCESS);
}

void catch_int(int sig_num)
{
	closeConn();
}

int processNew(request *req)
{
	if (cfig.hostRanges[0].rangeStart)
	{
		DWORD iip = ntohl(req->client.sin_addr.s_addr);
		BYTE allowed = false;

		for (WORD j = 0; j <= sizeof(cfig.hostRanges) && cfig.hostRanges[j].rangeStart; j++)
		{
			if (iip >= cfig.hostRanges[j].rangeStart && iip <= cfig.hostRanges[j].rangeEnd)
			{
				allowed = true;
				break;
			}
		}

		if (!allowed)
		{
			req->serverError.opcode = htons(5);
			req->serverError.errorcode = htons(2);
			strcpy(req->serverError.errormessage, "Access Denied");
			sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
			logMess(req, 1);
			cleanReq(req);
			return 2;
		}
	}

	req->block = 0;
	req->blksize = 512;
	req->timeout = timeout;
	req->expiry = time(NULL) + req->timeout;
	req->opcode = ntohs(datain->opcode);
	char *inPtr = (char*)datain;
	*(inPtr + (req->bytesRecd - 1)) = 0;
	inPtr += 2;
	req->filename = inPtr;

	if (!strlen(req->filename) || strlen(req->filename) > UCHAR_MAX)
	{
		req->serverError.opcode = htons(5);
		req->serverError.errorcode = htons(0);
		strcpy(req->serverError.errormessage, "Malformed Request, Invalid/Missing Filename");
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
		logMess(req, 1);
		cleanReq(req);
		return 1;
	}

	inPtr += strlen(inPtr) + 1;
	req->mode = inPtr;

	if (!strlen(req->mode) || strlen(req->mode) > 25)
	{
		req->serverError.opcode = htons(5);
		req->serverError.errorcode = htons(0);
		strcpy(req->serverError.errormessage, "Malformed Request, Invalid/Missing Mode");
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
		logMess(req, 1);
		cleanReq(req);
		return 1;
	}

	inPtr += strlen(inPtr) + 1;

	for (WORD i = 0; i < strlen(req->filename); i++)
		if (req->filename[i] == notFileSep)
			req->filename[i] = fileSep;

	tempbuff[0] = '.';
	tempbuff[1] = '.';
	tempbuff[2] = fileSep;
	tempbuff[3] = 0;

	if (strstr(req->filename, tempbuff))
	{
		req->serverError.opcode = htons(5);
		req->serverError.errorcode = htons(2);
		strcpy(req->serverError.errormessage, "Access violation");
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
		logMess(req, 1);
		cleanReq(req);
		return 1;
	}

	if (req->filename[0] == fileSep)
		req->filename++;

	if (!cfig.homes[0].alias[0])
	{
		if (strlen(cfig.homes[0].target) + strlen(req->filename) >= sizeof(req->path))
		{
			req->serverError.opcode = htons(5);
			req->serverError.errorcode = htons(2);
			strcpy(req->serverError.errormessage, "Filename too large");
			sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
			logMess(req, 1);
			cleanReq(req);
			return 1;
		}
		strcpy(req->path, cfig.homes[0].target);
		strcat(req->path, req->filename);
	}
	else
	{
		char *bname = strchr(req->filename, fileSep);

		if (bname)
		{
			*bname = 0;
			bname++;
		}
		else
		{
			req->serverError.opcode = htons(5);
			req->serverError.errorcode = htons(2);
			sprintf(req->serverError.errormessage, "Missing directory/alias");
			sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
			logMess(req, 1);
			cleanReq(req);
			return 1;
		}

		for (int i = 0; i < 8; i++)
		{
			if (cfig.homes[i].alias[0] && !strcasecmp(req->filename, cfig.homes[i].alias))
			{
				if (strlen(cfig.homes[i].target) + strlen(bname) >= sizeof(req->path))
				{
					req->serverError.opcode = htons(5);
					req->serverError.errorcode = htons(2);
					strcpy(req->serverError.errormessage, "Filename too large");
					sendto(cfig.tftpConn[req->sockInd].sock, (const char*) &req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
					logMess(req, 1);
					cleanReq(req);
					return 1;
				}

				strcpy(req->path, cfig.homes[i].target);
				strcat(req->path, bname);
				break;
			}
			else if (i == 7 || !cfig.homes[i].alias[0])
			{
				req->serverError.opcode = htons(5);
				req->serverError.errorcode = htons(2);
				sprintf(req->serverError.errormessage, "No such directory/alias %s", req->filename);
				sendto(cfig.tftpConn[req->sockInd].sock, (const char*) &req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
				logMess(req, 1);
				cleanReq(req);
				return 1;
			}
		}
	}

    if (ntohs(datain->opcode) == 1)
    {
		if (!cfig.fileRead)
		{
            req->serverError.opcode = htons(5);
            req->serverError.errorcode = htons(2);
            strcpy(req->serverError.errormessage, "GET Access Denied");
            sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
            logMess(req, 1);
            cleanReq(req);
            return 1;
        }

        errno = 0;

		if (*inPtr)
		{
			char *tmp = inPtr;

			while (*tmp)
			{
				if (!strcasecmp(tmp, "blksize"))
				{
					tmp += strlen(tmp) + 1;
					DWORD val = atol(tmp);

					if (val < 512)
						val = 512;
					else if (val > blksize)
						val = blksize;

					req->blksize = val;
					break;
				}

				tmp += strlen(tmp) + 1;
			}
		}

		if (!strcasecmp(req->mode, "netascii") || !strcasecmp(req->mode, "ascii"))
			req->file = fopen(req->path, "rt");
		else
			req->file = fopen(req->path, "rb");

		if (errno || !req->file)
		{
            req->serverError.opcode = htons(5);
            req->serverError.errorcode = htons(1);
            strcpy(req->serverError.errormessage, "No Such File/No Access");
            sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
            logMess(req, 1);
            cleanReq(req);
            return 1;
        }
    }
    else
    {
		if (!cfig.fileWrite && !cfig.fileOverwrite)
		{
            req->serverError.opcode = htons(5);
            req->serverError.errorcode = htons(2);
            strcpy(req->serverError.errormessage, "PUT Access Denied");
            sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
            logMess(req, 1);
            cleanReq(req);
            return 1;
        }

		req->file = fopen(req->path, "rb");

		if (req->file)
		{
			fclose(req->file);
			req->file = NULL;

			if (!cfig.fileOverwrite)
			{
                req->serverError.opcode = htons(5);
                req->serverError.errorcode = htons(6);
                strcpy(req->serverError.errormessage, "File already exists");
                sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
                logMess(req, 1);
                cleanReq(req);
                return 1;
            }
        }
		else if (!cfig.fileWrite)
		{
			req->serverError.opcode = htons(5);
			req->serverError.errorcode = htons(2);
			strcpy(req->serverError.errormessage, "Create File Access Denied");
			sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
			logMess(req, 1);
			cleanReq(req);
			return 1;
		}

		errno = 0;

		if (strcasecmp(req->mode, "netascii") && strcasecmp(req->mode, "ascii"))
			req->file = fopen(req->path, "wb");
		else
			req->file = fopen(req->path, "wt");

		if (errno || !req->file)
		{
			req->serverError.opcode = htons(5);
			req->serverError.errorcode = htons(1);
			strcpy(req->serverError.errormessage, "Invalid Path or No Access");
			sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
			logMess(req, 1);
			cleanReq(req);
			return 1;
		}
	}

	setvbuf(req->file, NULL, _IOFBF, 2 * req->blksize);

	if (*inPtr)
	{
		char *outPtr = req->mesout.buffer;
		req->mesout.opcode = htons(6);
		DWORD val;
		while (*inPtr)
		{
			//printf("%s", inPtr);
			if (!strcasecmp(inPtr, "blksize"))
			{
				strcpy(outPtr, inPtr);
				outPtr += strlen(outPtr) + 1;
				inPtr += strlen(inPtr) + 1;
				val = atol(inPtr);

				if (val < 512)
					val = 512;
				else if (val > blksize)
					val = blksize;

				req->blksize = val;
				sprintf(outPtr, "%lu", val);
				outPtr += strlen(outPtr) + 1;
			}
			else if (!strcasecmp(inPtr, "tsize"))
			{
				strcpy(outPtr, inPtr);
				outPtr += strlen(outPtr) + 1;
				inPtr += strlen(inPtr) + 1;

				if (ntohs(datain->opcode) == 1)
				{
					if (!fseek(req->file, 0, SEEK_END))
					{
						if (ftell(req->file) >= 0)
						{
							req->tsize = ftell(req->file);
							sprintf(outPtr, "%lu", req->tsize);
							outPtr += strlen(outPtr) + 1;
						}
						else
						{
							req->serverError.opcode = htons(5);
							req->serverError.errorcode = htons(2);
							strcpy(req->serverError.errormessage, "Invalid Path or No Access");
							sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
							logMess(req, 1);
							req->attempt = UCHAR_MAX;
							break;
						}
					}
					else
					{
						req->serverError.opcode = htons(5);
						req->serverError.errorcode = htons(2);
						strcpy(req->serverError.errormessage, "Invalid Path or No Access");
						sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
						logMess(req, 1);
						req->attempt = UCHAR_MAX;
						break;
					}
				}
				else
				{
					req->tsize = 0;
					sprintf(outPtr, "%lu", req->tsize);
					outPtr += strlen(outPtr) + 1;
				}
			}
			else if (!strcasecmp(inPtr, "timeout"))
			{
				strcpy(outPtr, inPtr);
				outPtr += strlen(outPtr) + 1;
				inPtr += strlen(inPtr) + 1;
				val = atoi(inPtr);

				if (val < 1)
					val = 1;
				else if (val > UCHAR_MAX)
					val = UCHAR_MAX;

				req->timeout = val;
				req->expiry = time(NULL) + req->timeout;
				sprintf(outPtr, "%lu", val);
				outPtr += strlen(outPtr) + 1;
			}

			inPtr += strlen(inPtr) + 1;

			//printf("=%u\n", val);
		}

		errno = 0;
		req->bytesReady = (DWORD)outPtr - (DWORD)&req->acout;
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->acout, req->bytesReady, 0, (sockaddr*)&req->client, req->clientsize);
		//errno = WSAGetLastError();
	}
	else if (htons(datain->opcode) == 2)
	{
		req->acout.opcode = htons(4);
		req->acout.block = htons(0);
		errno = 0;
		req->bytesReady = 4;
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->acout, req->bytesReady, 0, (sockaddr*)&req->client, req->clientsize);
		//errno = WSAGetLastError();
	}

	if (errno)
	{
		sprintf(req->serverError.errormessage, "Communication Error");
		logMess(req, 1);
		cleanReq(req);
		return errno;
	}

	if (ntohs(datain->opcode) == 1)
	{
		errno = 0;
		req->pkt[0] = (packet*)calloc(1, req->blksize + 4);
		req->pkt[1] = (packet*)calloc(1, req->blksize + 4);

		if (errno || !req->pkt[0] || !req->pkt[1])
		{
			strcpy(req->serverError.errormessage, "Memory Error");
			logMess(req, 1);
			cleanReq(req);
			return 1;
		}

		long ftellLoc = ftell(req->file);

		if (ftellLoc > 0)
		{
			if (fseek(req->file, 0, SEEK_SET))
			{
				req->serverError.opcode = htons(5);
				req->serverError.errorcode = htons(4);
				strcpy(req->serverError.errormessage, "Invalid Path or No Access");
				sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
				logMess(req, 1);
				cleanReq(req);
				return errno;
			}
		}
		else if (ftellLoc < 0)
		{
			req->serverError.opcode = htons(5);
			req->serverError.errorcode = htons(4);
			strcpy(req->serverError.errormessage, "Invalid Path or No Access");
			sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
			logMess(req, 1);
			cleanReq(req);
			return errno;
		}

		req->pkt[0]->opcode = htons(3);
		req->pkt[0]->block = htons(1);
		req->bytesRead[0] = fread(&req->pkt[0]->buffer, 1, req->blksize, req->file);

		if (errno)
		{
			req->serverError.opcode = htons(5);
			req->serverError.errorcode = htons(4);
			strcpy(req->serverError.errormessage, "Invalid Path or No Access");
			sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
			logMess(req, 1);
			cleanReq(req);
			return errno;
		}

		if (req->bytesRead[0] == req->blksize)
		{
			req->pkt[1]->opcode = htons(3);
			req->pkt[1]->block = htons(2);
			req->bytesRead[1] = fread(&req->pkt[1]->buffer, 1, req->blksize, req->file);

			if (errno)
			{
				req->serverError.opcode = htons(5);
				req->serverError.errorcode = htons(4);
				strcpy(req->serverError.errormessage, "Invalid Path or No Access");
				sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
				logMess(req, 1);
				cleanReq(req);
				return errno;
			}

			if (req->bytesRead[1] < req->blksize)
			{
				fclose(req->file);
				req->file = 0;
			}
		}
		else
		{
			fclose(req->file);
			req->file = 0;
		}

		if (!req->bytesReady)
		{
			req->block = 1;
			return processSend(req);
		}
	}
	return 0;
}

int processSend(request *req)
{
	errno = 0;
	req->expiry = time(NULL) + req->timeout;

	if (ntohs(req->pkt[0]->block) == req->block)
	{
		errno = 0;
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)req->pkt[0], req->bytesRead[0] + 4, 0, (sockaddr*)&req->client, req->clientsize);
		memcpy(&req->acout, req->pkt[0], 4);
		//errno = WSAGetLastError();

		if (errno)
		{
			sprintf(req->serverError.errormessage, "Communication Error");
			logMess(req, 1);
			cleanReq(req);
			return errno;
		}

		if (req->file)
		{
			req->tblock = ntohs(req->pkt[1]->block) + 1;
			if (req->tblock == req->block)
			{
				req->pkt[1]->block = htons(++req->tblock);
				req->bytesRead[1] = fread(&req->pkt[1]->buffer, 1, req->blksize, req->file);

				if (errno)
				{
					req->serverError.opcode = htons(5);
					req->serverError.errorcode = htons(4);
					sprintf(req->serverError.errormessage, strerror(errno));
					sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
					logMess(req, 1);
					cleanReq(req);
					return errno;
				}
				else if (req->bytesRead[1] < req->blksize)
				{
					fclose(req->file);
					req->file = 0;
				}
			}
		}
	}
	else if (ntohs(req->pkt[1]->block) == req->block)
	{
		errno = 0;
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)req->pkt[1], req->bytesRead[1] + 4, 0, (sockaddr*)&req->client, req->clientsize);
		memcpy(&req->acout, req->pkt[1], 4);
		//errno = WSAGetLastError();

		if (errno)
		{
			sprintf(req->serverError.errormessage, "Communication Error");
			logMess(req, 1);
			cleanReq(req);
			return errno;
		}

		if (req->file)
		{
			req->tblock = ntohs(req->pkt[0]->block) + 1;
			if (req->tblock == req->block)
			{
				req->pkt[0]->block = htons(++req->tblock);
				req->bytesRead[0] = fread(&req->pkt[0]->buffer, 1, req->blksize, req->file);

				if (errno)
				{
					req->serverError.opcode = htons(5);
					req->serverError.errorcode = htons(4);
					sprintf(req->serverError.errormessage, strerror(errno));
					sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
					logMess(req, 1);
					cleanReq(req);
					return errno;
				}
				else if (req->bytesRead[0] < req->blksize)
				{
					fclose(req->file);
					req->file = 0;
				}
			}
		}
	}
	else //if (ntohs(req->pkt[0]->block) < req->block && ntohs(req->pkt[1]->block) < req->block)
	{
		sprintf(logBuff, "Client %s %s, %li Blocks Served", req->mapname, req->path, req->fblock);
		logMess(logBuff, 2);
		cleanReq(req);
	}

	return 0;
}

int processRecv(request *req)
{
	req->expiry = time(NULL) + req->timeout;
	errno = 0;
	req->bytesReady = 4;
	sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->acout, req->bytesReady, 0, (sockaddr*)&req->client, req->clientsize);
	//errno = WSAGetLastError();
	if (errno)
	{
		sprintf(req->serverError.errormessage, "Communication Error");
		logMess(req, 1);
		cleanReq(req);
		return errno;
	}

	if (req->bytesRecd > 4 && (fwrite(&datain->buffer, req->bytesRecd - 4, 1, req->file) != 1 || errno))
	{
		req->serverError.opcode = htons(5);
		req->serverError.errorcode = htons(3);
		strcpy(req->serverError.errormessage, "Disk full or allocation exceeded");
		sendto(cfig.tftpConn[req->sockInd].sock, (const char*)&req->serverError, strlen(req->serverError.errormessage) + 5, 0, (sockaddr*)&req->client, req->clientsize);
		logMess(req, 1);
		cleanReq(req);
		return 1;
	}

	//printf("%u\n", req->bytesRecd);

	if ((WORD)req->bytesRecd < req->blksize + 4)
	{
		sprintf(logBuff, "Client %s %s, %lu Blocks Received", req->mapname, req->path, req->fblock);
		logMess(logBuff, 2);
		cleanReq(req);
	}

	return 0;
}

void cleanReq(request *req)
{
	if (req->file)
		fclose(req->file);

	if (req->pkt[0])
		free(req->pkt[0]);

	if (req->pkt[1])
		free(req->pkt[1]);

	req->file = 0;
	req->pkt[0] = 0;
	req->pkt[1] = 0;
	req->attempt = UCHAR_MAX;

}

char* myGetToken(char* buff, BYTE index)
{
	while (*buff)
	{
		if (index)
			index--;
		else
			break;

		buff += strlen(buff) + 1;
	}

	return buff;
}

WORD myTokenize(char *target, char *source, char *sep, bool whiteSep)
{
	bool found = true;
	char *dp = target;
	WORD kount = 0;

	while (*source)
	{
		if (sep && sep[0] && strchr(sep, (*source)))
		{
			found = true;
			source++;
			continue;
		}
		else if (whiteSep && *source <= 32)
		{
			found = true;
			source++;
			continue;
		}

		if (found)
		{
			if (target != dp)
			{
				*dp = 0;
				dp++;
			}
			kount++;
		}

		found = false;
		*dp = *source;
		dp++;
		source++;
	}

	*dp = 0;
	dp++;
	*dp = 0;

	return kount;
}

char* myTrim(char *target, char *source)
{
	while ((*source) && (*source) <= 32)
		source++;

	int i = 0;

	for (; i < 511 && source[i]; i++)
		target[i] = source[i];

	target[i] = source[i];
	i--;

	for (; i >= 0 && target[i] <= 32; i--)
		target[i] = 0;

	return target;
}

void mySplit(char *name, char *value, char *source, char splitChar)
{
	int i = 0;
	int j = 0;
	int k = 0;

	for (; source[i] && j <= 510 && source[i] != splitChar; i++, j++)
	{
		name[j] = source[i];
	}

	if (source[i])
	{
		i++;
		for (; k <= 510 && source[i]; i++, k++)
		{
			value[k] = source[i];
		}
	}

	name[j] = 0;
	value[k] = 0;

	myTrim(name, name);
	myTrim(value, value);
	//printf("%s %s\n", name, value);
}

bool getSection(char *sectionName, char *buffer, BYTE serial, char *fileName)
{
	//printf("%s=%s\n",fileName,sectionName);
	char section[128];
	sprintf(section, "[%s]", sectionName);
	myUpper(section);
	FILE *f = fopen(fileName, "rt");
	char buff[512];
	BYTE found = 0;

	if (f)
	{
		while (fgets(buff, 511, f))
		{
			myUpper(buff);
			myTrim(buff, buff);

			if (strstr(buff, section) == buff)
			{
				found++;
				if (found == serial)
				{
					//printf("%s=%s\n",fileName,sectionName);
					while (fgets(buff, 511, f))
					{
						myTrim(buff, buff);

						if (strstr(buff, "[") == buff)
							break;

						if ((*buff) >= '0' && (*buff) <= '9' || (*buff) >= 'A' && (*buff) <= 'Z' || (*buff) >= 'a' && (*buff) <= 'z' || ((*buff) && strchr("/\\?*", (*buff))))
						{
							buffer += sprintf(buffer, "%s", buff);
							buffer++;
						}
					}
					break;
				}
			}
		}
		fclose(f);
	}

	*buffer = 0;
	*(buffer + 1) = 0;
	return (found == serial);
}

char *IP2String(char *target, DWORD ip)
{
	data15 inaddr;
	inaddr.ip = ip;
	sprintf(target, "%u.%u.%u.%u", inaddr.octate[0], inaddr.octate[1], inaddr.octate[2], inaddr.octate[3]);
	return target;
}

char *myUpper(char *string)
{
	char diff = 'a' - 'A';

	WORD len = strlen(string);

	for (int i = 0; i < len; i++)
		if (string[i] >= 'a' && string[i] <= 'z')
			string[i] -= diff;

	return string;
}

char *myLower(char *string)
{
	char diff = 'a' - 'A';

	WORD len = strlen(string);

	for (int i = 0; i < len; i++)
		if (string[i] >= 'A' && string[i] <= 'Z')
			string[i] += diff;

	return string;
}

bool isIP(char *string)
{
	int j = 0;

	for (; *string; string++)
	{
		if (*string == '.' && *(string + 1) != '.')
			j++;
		else if (*string < '0' || *string > '9')
			return 0;
	}

	if (j == 3)
		return 1;
	else
		return 0;
}

DWORD my_inet_addr(char* str)
{
	if (str == NULL)
		return INADDR_ANY;
	DWORD x = inet_addr(str);
	if (x == INADDR_NONE)
		return INADDR_ANY;
	else
		return x;
}

void init()
{
	memset(&cfig, 0, sizeof(cfig));
	char iniStr[4096];
	char name[512];
	char value[512];
	static int port = 69;

	if (verbatim)
	{
		cfig.logLevel = 2;
		printf("%s\n\n", sVersion);
	}
	else if (getSection("LOGGING", iniStr, 1, iniFile))
	{
		char *iniStrPtr = myGetToken(iniStr, 0);

		if (!iniStrPtr[0] || !strcasecmp(iniStrPtr, "None"))
			cfig.logLevel = 0;
		else if (!strcasecmp(iniStrPtr, "Errors"))
			cfig.logLevel = 1;
		else if (!strcasecmp(iniStrPtr, "All"))
			cfig.logLevel = 2;
		else if (!strcasecmp(iniStrPtr, "Debug"))
			cfig.logLevel = 3;
		else
			cfig.logLevel = 255;
	}

	if (!verbatim && cfig.logLevel)
	{
		cfig.logfile = fopen(logFile, "at");

		if (cfig.logfile)
		{
			fclose(cfig.logfile);
			cfig.logfile = fopen(logFile, "at");
			fprintf(cfig.logfile, "%s\n\n", sVersion);
		}
		
        else
        {
            sprintf(iniStr, "Warning: faled to open log file %s", logFile);
            syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_CRIT), iniStr);
            cfig.logfile = 0;
        }

//	file_size = get_file_length(cfig.logfile);
// roylin test
//	sprintf(logBuff, "File Size :%d\n", file_size);
//	logMess(logBuff, 1);	
/////////////
    }

	if (getSection("PORT", iniStr, 1, iniFile))
	{
		char *iniStrPtr = myGetToken(iniStr, 0);
		if (strlen(iniStrPtr)){
			port = atoi(iniStrPtr);
			for (BYTE j = 0; j < MAX_SERVERS; j++){
				cfig.ports[j] = port;
			}
		}
	}

	if (getSection("LISTEN-ON", iniStr, 1, iniFile))
	{
		char *iniStrPtr = myGetToken(iniStr, 0);
		for (int i = 0; i < MAX_SERVERS && iniStrPtr[0]; iniStrPtr = myGetToken(iniStrPtr, 1))
		{
//			WORD port = 69;
			mySplit(name, value, iniStrPtr, ':');
			if (strlen(value))
			{
				port = atoi(value);
			}

			DWORD ip = my_inet_addr(name);

			if (isIP(name) && ip)
			{
				for (BYTE j = 0; j < MAX_SERVERS; j++)
				{
					if (cfig.servers[j] == ip)
						break;
					else if (!cfig.servers[j])
					{
						cfig.servers[j] = ip;
						cfig.ports[j] = port;
						i++;
						break;
					}
				}
			}
			else
			{
				sprintf(logBuff, "Warning: Section [LISTEN-ON], Invalid IP Address %s, ignored", iniStrPtr);
				logMess(logBuff, 1);
			}
		}
	}

#ifdef	QNAP_SERVICE_BINDING
	// Overwrite the orginal "LISTEN-ON" options by the QNAP_SERVICE_BINDING mechanism.
	Get_Binding_Address(&g_tBindAddr);
	if (g_tBindAddr.bEnabled)
	{
		for (int idx=0; (idx < g_tBindAddr.cnIpv4) && (idx < MAX_SERVERS); idx++)
		{
			cfig.servers[idx] = my_inet_addr(g_tBindAddr.ryszIpv4[idx]);
			cfig.ports[idx] = port;
		}
	}
#endif	//QNAP_SERVICE_BINDING

	if (getSection("HOME", iniStr, 1, iniFile))
	{
		char *iniStrPtr = myGetToken(iniStr, 0);
		for (; iniStrPtr[0]; iniStrPtr = myGetToken(iniStrPtr, 1))
		{
			mySplit(name, value, iniStrPtr, '=');
			if (strlen(value))
			{
				if (!cfig.homes[0].alias[0] && cfig.homes[0].target[0])
				{
					sprintf(logBuff, "Section [HOME], alias and bare path mixup, entry %s ignored", iniStrPtr);
					logMess(logBuff, 1);
				}
				else if (strchr(name, notFileSep) || strchr(name, fileSep) || strchr(name, '>') || strchr(name, '<') || strchr(name, '.'))
				{
					sprintf(logBuff, "Section [HOME], invalid chars in alias %s, entry ignored", name);
					logMess(logBuff, 1);
				}
				else if (name[0] && strlen(name) < 64 && value[0])
				{
					for (int i = 0; i < 8; i++)
					{
						if (cfig.homes[i].alias[0] && !strcasecmp(name, cfig.homes[i].alias))
						{
							sprintf(logBuff, "Section [HOME], Duplicate Entry: %s ignored", iniStrPtr);
							logMess(logBuff, 1);
							break;
						}
						else if (!cfig.homes[i].alias[0])
						{
							strcpy(cfig.homes[i].alias, name);
							strcpy(cfig.homes[i].target, value);

							if (cfig.homes[i].target[strlen(cfig.homes[i].target) - 1] != fileSep)
								strcat(cfig.homes[i].target, "/");

							break;
						}
					}
				}
				else
				{
					sprintf(logBuff, "Section [HOME], alias name %s too large", name);
					logMess(logBuff, 1);
				}
			}
			else if (!cfig.homes[0].alias[0] && !cfig.homes[0].target[0])
			{
				strcpy(cfig.homes[0].target, name);

				if (cfig.homes[0].target[strlen(cfig.homes[0].target) - 1] != fileSep)
					strcat(cfig.homes[0].target, "/");
			}
			else if (cfig.homes[0].alias[0])
			{
				sprintf(logBuff, "Section [HOME], alias and bare path mixup, entry %s ignored", iniStrPtr);
				logMess(logBuff, 1);
			}
			else if (cfig.homes[0].target[0])
			{
				sprintf(logBuff, "Section [HOME], Duplicate Path: %s ignored", iniStrPtr);
				logMess(logBuff, 1);
			}
			else
			{
				sprintf(logBuff, "Section [HOME], missing = sign, Invalid Entry: %s ignored", iniStrPtr);
				logMess(logBuff, 1);
			}
		}
	}

	cfig.fileRead = true;

	if (getSection("TFTP-OPTIONS", iniStr, 1, iniFile))
	{
		char *iniStrPtr = myGetToken(iniStr, 0);
		for (; strlen(iniStrPtr); iniStrPtr = myGetToken(iniStrPtr, 1))
		{
			mySplit(name, value, iniStrPtr, '=');

			if (strlen(value))
			{
				if (!strcasecmp(name, "UserName"))
				{
					if (strlen(value) < 128)
					{
						passwd *pwd = getpwnam(value);

						if (pwd)
						{
							cfig.pw_uid = pwd->pw_uid;
							cfig.pw_gid = pwd->pw_gid;
							strcpy(cfig.username, value);

							if (!cfig.homes[0].target[0])
							{
								if (cfig.pw_uid)
									sprintf(cfig.homes[0].target, "%s/", pwd->pw_dir);
								else
									strcpy(cfig.homes[0].target, "/home/");
							}
						}
						else
						{
							sprintf(logBuff, "Section [TFTP-OPTIONS], unknown username: %s, stopping", value);
							logMess(logBuff, 1);
							exit(EXIT_FAILURE);
						}
					}
					else
					{
						sprintf(logBuff, "Section [TFTP-OPTIONS], invalid username: %s, stopping", value);
						logMess(logBuff, 1);
						exit(EXIT_FAILURE);
					}
				}
				else if (!strcasecmp(name, "blksize"))
				{
					DWORD tblksize = atol(value);

					if (tblksize < 512)
						blksize = 512;
					else if (tblksize > 65464)
						blksize = 65464;
					else
						blksize = tblksize;
				}
				else if (!strcasecmp(name, "timeout"))
				{
					timeout = atol(value);
					if (timeout < 1)
						timeout = 1;
					else if (timeout > 255)
						timeout = 255;
				}
				else if (!strcasecmp(name, "Read"))
				{
					if (strchr("Yy", *value))
						cfig.fileRead = true;
					else
						cfig.fileRead = false;
				}
				else if (!strcasecmp(name, "Write"))
				{
					if (strchr("Yy", *value))
						cfig.fileWrite = true;
					else
						cfig.fileWrite = false;
				}
				else if (!strcasecmp(name, "Overwrite"))
				{
					if (strchr("Yy", *value))
						cfig.fileOverwrite = true;
					else
						cfig.fileOverwrite = false;
				}
				else
				{
					sprintf(logBuff, "Warning: unknown option %s, ignored", name);
					logMess(logBuff, 1);
				}
			}
		}
	}

	if (getSection("ALLOWED-CLIENTS", iniStr, 1, iniFile))
	{
		char *iniStrPtr = myGetToken(iniStr, 0);
		for (int i = 0; i < 32 && iniStrPtr[0]; iniStrPtr = myGetToken(iniStrPtr, 1))
		{
			DWORD rs = 0;
			DWORD re = 0;
			mySplit(name, value, iniStrPtr, '-');
			rs = htonl(my_inet_addr(name));

			if (strlen(value))
				re = htonl(my_inet_addr(value));
			else
				re = rs;

			if (rs && rs != INADDR_NONE && re && re != INADDR_NONE && rs <= re)
			{
				cfig.hostRanges[i].rangeStart = rs;
				cfig.hostRanges[i].rangeEnd = re;
				i++;
			}
			else
			{
				sprintf(logBuff, "Section [ALLOWED-CLIENTS] Invalid entry %s in ini file, ignored", iniStrPtr);
				logMess(logBuff, 1);
			}
		}
	}

	int i = 0;

	for (int j = 0; j < MAX_SERVERS; j++)
	{
 		if (j && !cfig.servers[j])
 			break;

 		cfig.tftpConn[i].sock = socket(PF_INET,
		                               SOCK_DGRAM,
		                               IPPROTO_UDP);

		if (cfig.tftpConn[i].sock == INVALID_SOCKET)
		{
			sprintf(logBuff, "Failed to Create Socket");
			logMess(logBuff, 1);
			continue;
		}

		cfig.tftpConn[i].addr.sin_family = AF_INET;

		if (!cfig.ports[j])
			cfig.ports[j] = 69;

		cfig.tftpConn[i].addr.sin_addr.s_addr = cfig.servers[j];
		cfig.tftpConn[i].addr.sin_port = htons(cfig.ports[j]);

		int nRet = bind(cfig.tftpConn[i].sock,
		                      (sockaddr*)&cfig.tftpConn[i].addr,
		                      sizeof(struct sockaddr_in)
		                     );

		if (nRet == SOCKET_ERROR)
		{
			close(cfig.tftpConn[i].sock);
			sprintf(logBuff, "%s Port %u, bind failed, %s", IP2String(tempbuff, cfig.servers[j]), cfig.ports[j], strerror(errno));
			logMess(logBuff, 1);
			continue;
		}

		if (cfig.maxFD < cfig.tftpConn[i].sock)
			cfig.maxFD = cfig.tftpConn[i].sock;

		cfig.tftpConn[i].server = cfig.tftpConn[i].addr.sin_addr.s_addr;
		cfig.tftpConn[i].port = htons(cfig.tftpConn[i].addr.sin_port);
		i++;
	}

	cfig.maxFD++;

	if (!cfig.tftpConn[0].port)
	{
		sprintf(logBuff, "no listening interfaces available, stopping..\n");
		logMess(logBuff, 1);
		exit(EXIT_FAILURE);
	}
	else if (verbatim)
	{
		printf("starting TFTP...\n");
	}
	else
	{
		sprintf(logBuff, "starting TFTP service");
		logMess(logBuff, 1);
	}

	if (cfig.username[0])
	{
		setuid(cfig.pw_uid);
		setgid(cfig.pw_gid);
	}
	else
	{
		passwd *pwd = getpwuid(getuid());
		strcpy(cfig.username, pwd->pw_name);

		if (!cfig.homes[0].target[0])
		{
			if (pwd->pw_uid)
				strcpy(cfig.homes[0].target, pwd->pw_dir);
			else
				strcpy(cfig.homes[0].target, "/home/");
		}
	}

	sprintf(logBuff, "username: %s", cfig.username);
	logMess(logBuff, 1);

	for (int i = 0; i < 8; i++)
		if (cfig.homes[i].target[0])
		{
			sprintf(logBuff, "alias /%s is mapped to %s", cfig.homes[i].alias, cfig.homes[i].target);
			logMess(logBuff, 1);
		}

	if (cfig.hostRanges[0].rangeStart)
	{
		char temp[128];

		for (WORD i = 0; i <= sizeof(cfig.hostRanges) && cfig.hostRanges[i].rangeStart; i++)
		{
			sprintf(logBuff, "%s", "permitted clients: ");
			sprintf(temp, "%s-", IP2String(tempbuff, htonl(cfig.hostRanges[i].rangeStart)));
			strcat(logBuff, temp);
			sprintf(temp, "%s", IP2String(tempbuff, htonl(cfig.hostRanges[i].rangeEnd)));
			strcat(logBuff, temp);
			logMess(logBuff, 1);
		}
	}
	else
	{
		sprintf(logBuff, "%s", "permitted clients: all");
		logMess(logBuff, 1);
	}

	sprintf(logBuff, "max blksize: %u", blksize);
	logMess(logBuff, 1);
	sprintf(logBuff, "default blksize: %u", 512);
	logMess(logBuff, 1);
	sprintf(logBuff, "default timeout: %u", timeout);
	logMess(logBuff, 1);
	sprintf(logBuff, "file read allowed: %s", cfig.fileRead ? "Yes" : "No");
	logMess(logBuff, 1);
	sprintf(logBuff, "file create allowed: %s", cfig.fileWrite ? "Yes" : "No");
	logMess(logBuff, 1);
	sprintf(logBuff, "file overwrite allowed: %s", cfig.fileOverwrite ? "Yes" : "No");
	logMess(logBuff, 1);

	if (!verbatim)
	{
		sprintf(logBuff, "logging: %s", cfig.logLevel > 1 ? "all" : "errors");
		logMess(logBuff, 1);
	}

	for (int i = 0; i < MAX_SERVERS && cfig.tftpConn[i].port; i++)
	{
		sprintf(logBuff, "listening on: %s:%i", IP2String(tempbuff, cfig.tftpConn[i].server), cfig.tftpConn[i].port);
		logMess(logBuff, 1);
	}
}

void logMess(char *logBuff, BYTE logLevel)
{
	if (verbatim)
		printf("%s\n", logBuff);
	else if (cfig.logfile && logLevel <= cfig.logLevel)
	{
		char currentTime[32];
		time_t t = time(NULL);

		check_log_file_size(cfig.logfile);
		tm *ttm = localtime(&t);
		strftime(currentTime, sizeof(currentTime), "%d-%b-%y %X", ttm);
		fprintf(cfig.logfile, "[%s] %s\n", currentTime, logBuff);
		fflush(cfig.logfile);
	}
	else if (logLevel <= cfig.logLevel)
	{
		syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_CRIT), logBuff);
	}
}

void logMess(request *req, BYTE logLevel)
{
	if (verbatim)
	{
		if (!req->serverError.errormessage[0])
			printf(req->serverError.errormessage, strerror(errno));

		if (req->path[0])
			printf("Client %s:%u %s, %s\n", IP2String(tempbuff, req->client.sin_addr.s_addr), ntohs(req->client.sin_port), req->path, req->serverError.errormessage);
		else
			printf("Client %s:%u, %s\n", IP2String(tempbuff, req->client.sin_addr.s_addr), ntohs(req->client.sin_port), req->serverError.errormessage);

	}
	else if (cfig.logfile && logLevel <= cfig.logLevel)
	{
		char currentTime[32];
		time_t t = time(NULL);

		check_log_file_size(cfig.logfile);
		tm *ttm = localtime(&t);
		strftime(currentTime, sizeof(currentTime), "%d-%b-%y %X", ttm);

		if (req->path[0])
			fprintf(cfig.logfile,"[%s] Client %s:%u %s, %s\n", currentTime, IP2String(tempbuff, req->client.sin_addr.s_addr), ntohs(req->client.sin_port), req->path, req->serverError.errormessage);
		else
			fprintf(cfig.logfile,"[%s] Client %s:%u, %s\n", currentTime, IP2String(tempbuff, req->client.sin_addr.s_addr), ntohs(req->client.sin_port), req->serverError.errormessage);

		fflush(cfig.logfile);
	}
	else if (logLevel <= cfig.logLevel)
	{
		char logBuff[256];

		if (!req->serverError.errormessage[0])
			sprintf(req->serverError.errormessage, strerror(errno));

		if (req->path[0])
			sprintf(logBuff,"Client %s:%u %s, %s\n", IP2String(tempbuff, req->client.sin_addr.s_addr), ntohs(req->client.sin_port), req->path, req->serverError.errormessage);
		else
			sprintf(logBuff,"Client %s:%u, %s\n", IP2String(tempbuff, req->client.sin_addr.s_addr), ntohs(req->client.sin_port), req->serverError.errormessage);

		syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_CRIT), logBuff);
	}
}



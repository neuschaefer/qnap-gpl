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

// tftpserver.h

#ifndef LOG_MAKEPRI
#define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))
#endif

#ifndef SIOCGIFCONF
#include <sys/sockio.h>
#endif

#ifndef INADDR_NONE
#define INADDR_NONE ULONG_MAX
#endif

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1


// Enable service binding here.
#define	QNAP_SERVICE_BINDING


//Constants
#define WORD unsigned short
#define BYTE unsigned char
#define DWORD unsigned long
#define SOCKET int
#define MAX_SERVERS 255

//Structs
struct home
{
	char alias[64];
	char target[256];
};

struct tftpConnType
{
	SOCKET sock;
	sockaddr_in addr;
	DWORD server;
	WORD port;
};

struct acknowledgement
{
	WORD opcode;
	WORD block;
};

struct message
{
	WORD opcode;
	char buffer[514];
};

struct packet
{
	WORD opcode;
	WORD block;
	char buffer;
};

struct data12
{
	DWORD rangeStart;
	DWORD rangeEnd;
};

struct tftperror
{
	WORD opcode;
	WORD errorcode;
	char errormessage[508];
};

struct request
{
	char mapname[32];
	BYTE opcode;
	BYTE attempt;
	BYTE sockInd;
	time_t expiry;
	char path[256];
	FILE *file;
	char *filename;
	char *mode;
	DWORD tsize;
	DWORD blksize;
	DWORD timeout;
	DWORD fblock;
	WORD block;
	WORD tblock;
	int bytesRecd;
	WORD bytesRead[2];
	int bytesReady;
	sockaddr_in client;
	socklen_t clientsize;
	packet* pkt[2];
	union
	{
		acknowledgement acout;
		message mesout;
		tftperror serverError;
	};
};

struct data2
{
	tftpConnType tftpConn[MAX_SERVERS];
	DWORD servers[MAX_SERVERS];
	WORD ports[MAX_SERVERS];
	data12 hostRanges[32];
	home homes[8];
	FILE *logfile;
	char username[128];
	uid_t pw_uid;
	uid_t pw_gid;
	char fileRead;
	char fileWrite;
	char fileOverwrite;
	BYTE logLevel;
	int minport;
	int maxport;
	int maxFD;
};

struct data15
{
	union
	{
		//DWORD ip;
		unsigned ip:32;
		BYTE octate[4];
	};
};

//types
typedef map<string, request*> myMap;
typedef multimap<long, request*> myMultiMap;

//Functions
int processNew(request*);
int processSend(request*);
int processRecv(request*);
void runProg();
void init();
void closeConn(int bExit=true);
void catch_int(int sig_num);
void cleanReq(request*);
void logMess(request*, BYTE);
void logMess(char*, BYTE);
void mySplit(char*, char*, char*, char);
extern void getServ(DWORD*, WORD*, const BYTE);
bool getSection(char*, char*, BYTE, char*);
char* myGetToken(char*, BYTE);
char* myTrim(char*, char*);
char* myLower(char* string);
char* myUpper(char* string);
char* myTokenize(char*, char*);
char* IP2String(char*, DWORD);
DWORD my_inet_addr(char*);


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>   
#include <sys/stat.h>
#include <fcntl.h>  
#include <sys/ioctl.h> 

#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpsourcedata.h"
#include <iostream>
#include <string>  
#include <vector>
using namespace std;

#define MAX_PORT_LEN 		8
#define MAX_SESSION_LEN		16 

void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

unsigned b64_encode(const void *src, unsigned len, void *dst)
{
    const unsigned char base64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char *end = (unsigned char *)src + len;
    unsigned char *lend = (unsigned char *)dst + 76;
    unsigned char *s, *p; 
    int pos = 2;
    unsigned long buf = 0;

    s = (unsigned char *)src; 
    p = (unsigned char *)dst;
    while (s < end) {
	buf |= *s << (pos << 3);
	pos--;
	s++;
	/* write it out */
	if (pos < 0) {
	    *p++ = base64[(buf >> 18) & 0x3f];
	    *p++ = base64[(buf >> 12) & 0x3f];
	    *p++ = base64[(buf >> 6) & 0x3f];
	    *p++ = base64[buf & 0x3f];
	    pos = 2;
	    buf = 0;
	}

	if (p >= lend) {
	    *p++ = '\n';
	    lend = p + 76;
	}
    }

    if (pos != 2) {
	*p++ = base64[(buf >> 18) & 0x3f];
	*p++ = base64[(buf >> 12) & 0x3f];
	*p++ = (pos == 0) ? base64[(buf >> 6) & 0x3f] : '=';
	*p++ = '=';
    }

    *p = 0;
    return (p - (unsigned char *)dst);
} 

unsigned b64_decode(const void *src, unsigned len, void *dst)
{
    const unsigned char *srcend = (unsigned char *)src + len;
    unsigned char *s = (unsigned char *)src;
    unsigned char *p = (unsigned char *)dst;
    char c;
    unsigned b = 0;
    unsigned long buf = 0;
    int pos = 0, end = 0;

    if (len <= 0)
        return 0;

    while (s < srcend) {
        c = *s++;
        if (c >= 'A' && c <= 'Z')
            b = c - 'A';
        else if (c >= 'a' && c <= 'z')
            b = c - 'a' + 26;
        else if (c >= '0' && c <= '9')
            b = c - '0' + 52;
        else if (c == '+')
            b = 62;
        else if (c == '/')
            b = 63;
        else if (c == '='){
                        /* end sequence */
            if (!end) {
                if (pos == 2)
                    end = 1;
                else if (pos == 3)
                    end = 2;
                //else
                    //printf("base64: unexpected '='\n");
            }
            b = 0;
        }
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                continue;
        //else
                //printf("base64: Invalid symbol\n");

        /* add it to buffer */
        buf = (buf << 6) + b;
        pos++;
        if (pos == 4) {
            *p++ = (buf >> 16) & 255;
            if (end == 0 || end > 1)
                *p++ = (buf >> 8) & 255;
            if (end == 0 || end > 2)
                *p++ = buf & 255;
                buf = 0;
                pos = 0;
        }
    }
    *p = 0;
    //if (pos != 0)
        //printf("base64: invalid end sequence\n");

    return (p - (unsigned char *)dst);
} 

/**** !!! outdata size must set to MAX_PORT_LEN !!! ****/
int getServerPort(char *indata, char *outdata)
{       
	int i;
	char search01[]="server_port";
	char *tmpstr;
	
	tmpstr = strstr(indata, search01);
	if( tmpstr == NULL ){
		printf("Can not find %s\n", search01);
		return -1;
	}   
	tmpstr += (strlen(search01)+1); 
    for(i=0; i < MAX_PORT_LEN; i++){
    	if( tmpstr[i] == '-' )	break;
    	outdata[i] = tmpstr[i];
    }
    outdata[i]='\0';    
    //GetServerPort = true;
    
	return 0;	
}

/**** !!! outdata size must set to MAX_SESSION_LEN !!! ****/
int getSessionID(char *indata, char *outdata)
{       
	int i;
	char search01[]="Session:";
	char search02[]="session:";
	char *tmpstr;
	
	tmpstr = strstr(indata, search01);
	if( tmpstr == NULL ){
		tmpstr = strstr(indata, search02);
		if( tmpstr == NULL ){
			printf("Can not find %s and %s\n", search01, search02);
			return -1;
		}	
	}   
	tmpstr += (strlen(search01)+1); 
    for(i=0; i < MAX_SESSION_LEN; i++){
    	if( tmpstr[i] == ';' )	break;
    	outdata[i] = tmpstr[i];
    }
    outdata[i]='\0';
	//printf("%s\n",outdata);
	return 0;	
}

string str_trim(const string &s)
{
    int i1 = 0;
    int i2 = s.size() - 1;

    // reverse erase until none space char
    while(i2 >= 0 && (s[i2] == ' ' || s[i2] == '\t'))
	i2--;
    
    i2++;
    // erase until none space char
    while(i1 < i2 && (s[i1] == ' ' || s[i1] == '\t'))
	i1++;

    if(i2 == i1)
	return "";

    if(i1 == 0 && i2 == static_cast<int>(s.size()))
	return s;
    else
	return s.substr(i1, i2 - i1);
}
long seperate(const string &s, vector<string> &v, char delimeter)
{
    char delimeter2 = 0;

    v.clear();
    if(delimeter == ' ')
	delimeter2 = '\t';
    if(delimeter == '\t')
	delimeter2 = ' ';

    for(unsigned i = 0, j = 0; i <= s.size(); i++) {
	if(s[i] == delimeter 
		|| (delimeter2 != 0 && s[i] == delimeter2) 
		|| (s[i] == 0 && i > 0)) {
	    string t = s.substr(j, i - j);

	    if(str_trim(t).length() > 0)
		v.push_back(t);

	    j = i + 1;
	}

	if(i >= s.size())
	    break;
    }

    return v.size();
}

int main(int argc, char *argv[])
{   	
	if(argc!=3){
		puts("input Error");
		return 1;
	}
		
	char auth_org[1024], auth_aft[1024];	
	sprintf(auth_org,"%s:%s",argv[1],argv[2]); 
	b64_encode(auth_org, strlen(auth_org), auth_aft);
	printf("user=%s passwd=%s auth_aft=%s\n",argv[1], argv[2], auth_aft);  
	
	//char *host="172.17.25.10";	//vivo ip 7135	
	char *host="172.17.27.161";		//vivo ip 7138
	       	
	char rtsp_describe[1024], rtsp_setup_video[1024], rtsp_setup_audio[1024], rtsp_play[1024], rtsp_teardown[1024]; 
	char rtsp_in_buf[1024];  	
	unsigned short rtsp_port=554; 
	unsigned short video_client_port=3182, audio_client_port=3184;
	int rtspsock;
	int getByte; 
	
    struct sockaddr_in serv_addr;
    struct hostent *hptr = NULL; 
    
//---------------------------- set RTP connection ----------------------------
  
    RTPSession rtp_video_sess, rtp_audio_sess;
	int status;    
	
	RTPUDPv4TransmissionParams video_transparams, audio_transparams;
	RTPSessionParams sessparams;
	sessparams.SetOwnTimestampUnit(1.0/80.0);
	sessparams.SetAcceptOwnPackets(true);
	video_transparams.SetPortbase(video_client_port);
	status = rtp_video_sess.Create(sessparams,&video_transparams);	
	checkerror(status); 
    
    int mp4file_fd = open("vivo_ip7135_rtp.mp4", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO );
	if ( mp4file_fd < 0 ) {
		close(mp4file_fd); 		
		puts("open mp4file_fd error");
		exit(1);
	}    
#define AUDIO
#ifdef AUDIO
	//RTPSessionParams sessparams;
	//sessparams.SetOwnTimestampUnit(1.0/80.0);
	//sessparams.SetAcceptOwnPackets(true);
	audio_transparams.SetPortbase(audio_client_port);
	status = rtp_audio_sess.Create(sessparams,&audio_transparams);	
	checkerror(status); 
    
    int Audiofile_fd = open("vivo_ip7135_rtp.m4a", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO );
	if ( Audiofile_fd < 0 ) {
		close(Audiofile_fd); 		
		puts("open Audiofile_fd error");
		exit(1);
	} 
#endif	
	                 
    
//---------------------------- set RTSP connection ----------------------------
    rtspsock = socket(AF_INET, SOCK_STREAM, 0);
    if (rtspsock >= 0) {
		memset(&serv_addr, 0, sizeof(struct sockaddr_in));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(rtsp_port);
        hptr = gethostbyname(host);
        if (! hptr) {
	    	unsigned long net_addr = inet_addr(host);
	    	hptr = gethostbyaddr((char *)&net_addr, sizeof(net_addr), AF_INET);
		}
		if (! hptr) 
	    	puts("invalid host");
		else {
	    	memcpy(&serv_addr.sin_addr, (struct in_addr **)hptr->h_addr_list[0], sizeof(struct in_addr));
	    	if( connect(rtspsock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ){ 
				puts("connect Error");      
				exit(1);
			}	
		}
    }
    
    int CSeqID = 1;

	sprintf(rtsp_describe,
    "DESCRIBE rtsp://%s:%d/live.sdp RTSP/1.0\r\n"
    "CSeq: %d\r\n"
    "Accept: application/sdp\r\n"
    "User-Agent: RTPExPlayer\r\n"
    "Bandwidth: 512000\r\n"		
    "Accept-Language: en-GB\r\n"
    "Authorization: Basic %s\r\n\r\n", host, rtsp_port, CSeqID++, auth_aft    
    );
    
    puts("----------------> rtsp describe send");
    printf("%s",rtsp_describe);  
    
    if( send(rtspsock, rtsp_describe, strlen(rtsp_describe), 0) <= 0 ) {
		puts("send data error");
		exit(1);
	}
	puts("----------------> rtsp describe receive");  
    bzero(rtsp_in_buf, sizeof(rtsp_in_buf));   
    if( recv(rtspsock, rtsp_in_buf, sizeof(rtsp_in_buf), 0) <=0 ){
    	puts("recv data error");
		exit(1);    	
    }
    printf("%s",rtsp_in_buf); 



    sprintf(rtsp_setup_video,
    "SETUP rtsp://%s:%d/live.sdp/trackID=1 RTSP/1.0\r\n"
    "CSeq: %d\r\n"   
    "User-Agent: RTPExPlayer\r\n"       
    "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d\r\n\r\n", 
    host, rtsp_port, CSeqID++, video_client_port, video_client_port+1    
    ); 
    puts("----------------> rtsp video setup send");
    printf("%s",rtsp_setup_video);     
    
    if( send(rtspsock, rtsp_setup_video, strlen(rtsp_setup_video), 0) <= 0 ) {
		puts("send RTSP video data error");
		exit(1);
	}
	puts("----------------> rtsp video setup receive"); 
    bzero(rtsp_in_buf, sizeof(rtsp_in_buf));  
    if( recv(rtspsock, rtsp_in_buf, sizeof(rtsp_in_buf), 0) <=0 ){
    	puts("recv RTSP video data error");
		exit(1);    	
    }
    printf("%s",rtsp_in_buf);
    
    //check RTSP response is 200 OK 
    char *tmpstr = strstr(rtsp_in_buf, "RTSP/1.0 200 OK");
	if( tmpstr == NULL ){
		puts("RTSP SETUP video session fail");
		return 1;
	}
    
    char session[MAX_SESSION_LEN];
    char RTCP_videoServerPort[MAX_PORT_LEN];
    getSessionID(rtsp_in_buf, session); 
    getServerPort(rtsp_in_buf, RTCP_videoServerPort);
    printf("session=%s RTCP_videoServerPort=%s\n", session, RTCP_videoServerPort);
   
	//---------------------------- set video RTCP connection ---------------------------- 
    vector<string> ipsubset;
    seperate(host, ipsubset, '.');
    u_int8_t iptable[4];    
    for(int i=0;i<4;i++)	iptable[i] = atoi(ipsubset[i].c_str()); 
      
    RTPIPv4Address RTCP_video_addr(iptable, atoi(RTCP_videoServerPort));
    rtp_video_sess.AddDestination(RTCP_video_addr);
    
 
#ifdef AUDIO
    sprintf(rtsp_setup_audio,
    //"SETUP rtsp://%s:%d/live.sdp/trackID=3 RTSP/1.0\r\n"	//for 7135 audio
    "SETUP rtsp://%s:%d/live.sdp/trackID=4 RTSP/1.0\r\n"	//for 7138 audio
    "CSeq: %d\r\n"
    "Session: %s\r\n"   
    "User-Agent: RTPExPlayer\r\n"       
    "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d\r\n\r\n", 
    host, rtsp_port, CSeqID++, session, audio_client_port, audio_client_port+1    
    ); 
    puts("----------------> rtsp audio setup send");
    printf("%s",rtsp_setup_audio);     
    
    if( send(rtspsock, rtsp_setup_audio, strlen(rtsp_setup_audio), 0) <= 0 ) {
		puts("send RTSP audio data error");
		exit(1);
	}
	puts("----------------> rtsp audio setup receive"); 
    bzero(rtsp_in_buf, sizeof(rtsp_in_buf));  
    if( recv(rtspsock, rtsp_in_buf, sizeof(rtsp_in_buf), 0) <=0 ){
    	puts("recv RTSP audio data error");
		exit(1);    	
    }
    printf("%s",rtsp_in_buf);
    
    //check RTSP response is 200 OK 
    tmpstr = strstr(rtsp_in_buf, "RTSP/1.0 200 OK");
	if( tmpstr == NULL ){
		puts("RTSP SETUP audio session fail");
		return 1;
	}
    
    char RTCP_audioServerPort[MAX_PORT_LEN];
    getServerPort(rtsp_in_buf, RTCP_audioServerPort);
    printf("session=%s RTCP_audioServerPort=%s\n", session, RTCP_audioServerPort);
   
	//---------------------------- set audio RTCP connection ----------------------------
    RTPIPv4Address RTCP_audio_addr(iptable, atoi(RTCP_audioServerPort));
    rtp_audio_sess.AddDestination(RTCP_audio_addr);
#endif 
   
       
	sprintf(rtsp_play,
    "PLAY rtsp://%s:%d/live.sdp RTSP/1.0\r\n"
    "Session: %s\r\n"
    "CSeq: %d\r\n"
    "Range: npt=-\r\n\r\n", host, rtsp_port, session, CSeqID++
    );
    puts("----------------> rtsp play send");
    printf("%s",rtsp_play);      
    
    if( send(rtspsock, rtsp_play, strlen(rtsp_play), 0) <= 0 ) {
		puts("send data error");
		exit(1);
	}
	puts("----------------> rtsp play receive"); 
    bzero(rtsp_in_buf, sizeof(rtsp_in_buf));  
    if( recv(rtspsock, rtsp_in_buf, sizeof(rtsp_in_buf), 0) <=0 ){
    	puts("recv data error");
		exit(1);    	
    }
    printf("%s",rtsp_in_buf);     
  
    
#define MAX_TOTAL_WRITE 1024*500
    bool get_stream = true; 
    int total_write=0;
    
	for (int i = 1 ; get_stream ; i++)
	{ 	
		status = rtp_video_sess.Poll();
		checkerror(status);
		rtp_video_sess.BeginDataAccess();
		// check incoming packets
		if (rtp_video_sess.GotoFirstSourceWithData())
		{ 
			do{
				RTPPacket *pack;
				while(( pack = rtp_video_sess.GetNextPacket()) != NULL ) {
					
					printf("<%d>Get video packet len=%d paload len=%d exten len=%d\n", i, pack->GetPacketLength(), pack->GetPayloadLength(), pack->GetExtensionLength());
					getByte = write(mp4file_fd, pack->GetPayloadData(), pack->GetPayloadLength()); 
					//getByte = write(mp4file_fd, pack->GetPacketData(), pack->GetPacketLength()); 
					total_write += getByte;   
					//printf("total_write=%d\n", total_write);
					delete pack;
					if( total_write > MAX_TOTAL_WRITE)	get_stream = false;
				}  
			}while( rtp_video_sess.GotoNextSourceWithData() );
		} 
		
		
#ifdef AUDIO
		status = rtp_audio_sess.Poll();
		checkerror(status);
		rtp_audio_sess.BeginDataAccess();
		// check incoming packets
		if (rtp_audio_sess.GotoFirstSourceWithData())
		{ 
			do{
				RTPPacket *pack;
				while(( pack = rtp_audio_sess.GetNextPacket()) != NULL ) {
					
					printf("<%d>Get audio packet len=%d paload len=%d exten len=%d\n", i, pack->GetPacketLength(), pack->GetPayloadLength(), pack->GetExtensionLength());
					getByte = write(Audiofile_fd, pack->GetPayloadData(), pack->GetPayloadLength());  
					//getByte = write(Audiofile_fd, pack->GetPacketData(), pack->GetPacketLength()); 
					total_write += getByte;   
					//printf("total_write=%d\n", total_write);
					delete pack;
					if( total_write > MAX_TOTAL_WRITE)	get_stream = false;
				}  
			}while( rtp_audio_sess.GotoNextSourceWithData() );
		}
#endif

		
		//rtp_video_sess.EndDataAccess();
		//rtp_audio_sess.EndDataAccess();		
		usleep(2000);
		//RTPTime::Wait(RTPTime(1,0));
	}
	rtp_video_sess.BYEDestroy(RTPTime(10,0),0,0);	  
    
    
    sprintf(rtsp_teardown,
    "TEARDOWN rtsp://%s:%d/live.sdp RTSP/1.0\r\n"
    "Session: %s\r\n"
    "CSeq: %d\r\n\r\n", host, rtsp_port, session, CSeqID++
    );
    puts("----------------> rtsp teardown send");
    printf("%s",rtsp_teardown);      
    
    if( send(rtspsock, rtsp_teardown, strlen(rtsp_teardown), 0) <= 0 ) {
		puts("send teardown data error");
		exit(1);
	}
	puts("----------------> rtsp teardown receive"); 
    bzero(rtsp_in_buf, sizeof(rtsp_in_buf));  
    if( recv(rtspsock, rtsp_in_buf, sizeof(rtsp_in_buf), 0) <=0 ){
    	puts("recv teardown data error");
		exit(1);    	
    }
    printf("%s",rtsp_in_buf);
    
    	
    close(mp4file_fd); 
    close(rtspsock);  
#ifdef AUDIO    
    close(Audiofile_fd);
#endif        
 
	puts("--------- END ----------");        
    return 0;	
}

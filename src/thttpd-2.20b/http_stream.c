#if 0
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "timers.h"
#include "libhttpd.h"
#include "http_stream.h"
#include "nvr_spec.h"
#include "gtypes.h"
#include "gcom.h"
#include "gmsdbq.h"
#include "log.h"
#include "gmd.h"
#endif

#if !defined(_VIOSTOR_)

static void touch_file( char *path )
{
    FILE *file=fopen( path, "w");
    if( file )
    {
        fclose(file);
    }
}

#include <sys/timeb.h>

struct ttinfo {
       long          tt_gmtoff;
       int           tt_isdst;
       unsigned int  tt_abbrind;
};

static int tzh_ttisgmtcnt;
static int tzh_ttisstdcnt;
static int tzh_leapcnt;
static int tzh_timecnt;
static int tzh_typecnt;
static int tzh_charcnt;
static time_t *trans_time;
static int *local_time_type;
static struct ttinfo *local_time_tab;

static BOOL read_int( FILE *file, int *v )
{
    unsigned int i;
    if( fread( (void *)&i, sizeof(int), 1, file )<=0 )return FALSE;
    i=ntohl(i);
    *v=(int)i;
    return TRUE;
}

static BOOL read_uint( FILE *file, unsigned int *i )
{
    if( fread( (void *)i, sizeof(int), 1, file )<=0 )return FALSE;
    *i=ntohl(*i);
    return TRUE;
}

static int get_lt_idx( time_t now )
{
    if( !local_time_type )
        return 0;

    time_t *t=trans_time;
    int i;
    for( i=0; i<tzh_timecnt; i++, t++ )
    {
        if( now< (*t) )break;
    }
    if( i==0 )  // it should never happend
        i++;
    return local_time_type[i-1];
}

static void get_local_time_info( BOOL verbose )
{
    unsigned char buf[1024];
    time_t *t=trans_time;
    int i;
//fprintf(stderr, "FCLIB:: get_local_time_info() begin.......\n\n.");
    FILE *file=fopen( "/etc/localtime", "rb");
    if( !file )
    {
		file=fopen( "/etc/config/localtime", "rb");
		if(!file)
		{
			fprintf(stderr, "get_local_time_info() failed errno=%d.......\n", errno);
		    return;
		}
    }
    fread( buf, 1, 4, file );
    if(strncmp( (char *)buf, "TZif", 4 ) ) {// ?is a time zone information file
        fclose(file);
        return;
    }
    fread( buf, 1, 16, file ); // skip reserved
    read_int( file, &tzh_ttisgmtcnt );
    if( verbose )
        fprintf( stderr, " UTC/local indicators number=%d\n", tzh_ttisgmtcnt );
    read_int( file, &tzh_ttisstdcnt ); 
    if( verbose )
        fprintf( stderr, " standard/wall indicators number=%d\n", tzh_ttisstdcnt );
    read_int( file, &tzh_leapcnt ); 
    if( verbose )
        fprintf( stderr, "leap seconds number=%d\n", tzh_leapcnt );
    read_int( file, &tzh_timecnt ); 
    if( verbose )
        fprintf( stderr, "transition time number=%d\n", tzh_timecnt );
    read_int( file, &tzh_typecnt ); 
    if( verbose )
        fprintf( stderr, "local time type number=%d\n", tzh_typecnt );
    read_int( file, &tzh_charcnt ); 
    if( verbose )
        fprintf( stderr, "char abbr number=%d\n", tzh_charcnt );
    trans_time=(time_t *)calloc( tzh_timecnt, sizeof(time_t));
    t=trans_time;
    for( i=0; i<tzh_timecnt; i++, t++ )
    {
        read_uint( file, (unsigned int *)t );
        if( verbose )
            fprintf( stderr,"transition time[%d]=%s\n", i, ctime(t));
    }
    
    local_time_type=(int *)calloc( tzh_timecnt, sizeof(int));
    if( local_time_type )
    {
        int *l=local_time_type;
        for( i=0; i<tzh_timecnt; i++, l++ )
        {
            unsigned char u;
            fread( &u, 1, 1, file );
            *l=u;
            if( verbose )
                fprintf( stderr, "dst[%d]=%u\n", i, u);
        }
    }
    
    local_time_tab=(struct ttinfo *)calloc( tzh_typecnt, sizeof(struct ttinfo));
    if( local_time_tab )
    {
        struct ttinfo *f=local_time_tab;
        for( i=0; i<tzh_typecnt; i++, f++ )
        {
            read_int( file, (int *)&f->tt_gmtoff );
            fread( &f->tt_isdst, 1, 1, file );
            fread( &f->tt_abbrind, 1, 1, file );
            if( verbose )
                fprintf( stderr, "[%d] tt_gmtoff=%u, isdst=%u, abbr=%u\n", i, (unsigned int)f->tt_gmtoff, f->tt_isdst, f->tt_abbrind );
        }
    }
    for( i=0; i<tzh_leapcnt; i++ )
    {
        unsigned int u;
        read_uint( file, &u );
        if( verbose )
            fprintf( stderr, "[%d] leap second occurs at %s", i, ctime((time_t *)&u));
    }
    for( i=0; i<tzh_ttisstdcnt; i++ )
    {
        unsigned char u;
        fread( &u, 1, 1, file );
        if( verbose )
            fprintf( stderr, "stand/wall[%d]=%u\n", i, u);
    }
    for( i=0; i<tzh_ttisgmtcnt; i++ )
    {
        unsigned char u;
        fread( &u, 1, 1, file );
        if( verbose )
            fprintf( stderr, "is_gmt[%d]=%u\n", i, u);
    }
    int len=fread( buf, 1, 1024, file );
    if( verbose )
        fprintf( stderr, "data remain=%d\n", len );
    fclose(file);
}

static long do_get_time_zone( time_t now )
{
    if( !local_time_tab )
        return 0;

    int i=get_lt_idx( now );
    return local_time_tab[i].tt_gmtoff;
}

static TIMESTAMP_t conv_to_local_time( TIMESTAMP_t t )
{
    return t+((TIMESTAMP_t)1000)*do_get_time_zone( (time_t)(t/1000) );
}

TIMESTAMP_t http_stream_current_time_stamp( void )
{
    static BOOL first=TRUE;
    if( first )
    {
        // get time zone information
        get_local_time_info(0);
        first=FALSE;
    }
 
    struct timeval tp;
    gettimeofday( &tp, NULL);
    long long msecs=((tp.tv_usec-(tp.tv_usec%1000))/1000)%1000;
    return msecs+((long long)tp.tv_sec)*1000;
}

static void get_date_and_time_from_tm( TIMESTAMP_t time_stamp, struct tm *tm )
{
    time_stamp-=(time_stamp%1000);
    time_t time=(time_t)(time_stamp/1000);
    localtime_r( &time, tm );
}

void http_stream_make_time_string2( char *date_time, TIMESTAMP_t time_stamp )
{
    if( time_stamp<=0)
    {
        strcpy( date_time, "00/00/00-00:00:00");
    }
    struct tm tm, *t=&tm;
    get_date_and_time_from_tm( time_stamp, t );
    sprintf( date_time, "%02d/%02d/%02d-%02d:%02d:%02d.%03d",
    t->tm_year-100,
    t->tm_mon+1,
    t->tm_mday,
    t->tm_hour,
    t->tm_min,
    t->tm_sec,
    (int)(time_stamp%1000));
}

void http_stream_log_Console( char *fmt, ...)
{
}

void http_stream_log_Debug( char *fmt, ...)
{
}


int http_stream_smon_set_s( int idx, char *state )
{
    return 0;
}

#include <stdlib.h>
#include <dlfcn.h>

#define CMSF_LIB_PATH "/usr/lib/libcmsf.so.0.0"

#define CMSF_CALL_COM( fname,t ) \
    if(!cmsf_lib)\
    cmsf_lib = dlopen ( CMSF_LIB_PATH, RTLD_NOW|RTLD_GLOBAL );\
    if(!cmsf_lib)return -1;\
    void *f=dlsym( cmsf_lib, fname);\
    if(!f)return -2;

#define CMSF_CALL( fname,t ) CMSF_CALL_COM( fname,t ) return ((t)f)
#define CMSF_CALL2( fname,t ) CMSF_CALL_COM( fname,t ) ret=((t)f)

static void *cmsf_lib=NULL;

static BOOL __installed=FALSE;

static int ptz_c_install(void)
{
    int ret;
    CMSF_CALL2("_Z16cmsf_ptz_installv",int (*)(void))();
    if(ret!=0)return ret;
    __installed=TRUE;
    return 0;
}

static int ptz_c_refresh_channel_info(void)
{
    if(!__installed )
        return 0;
    CMSF_CALL("_Z29cmsf_ptz_refresh_channel_infov",int (*)(void))();
}

static int ptz_c_open( int channel_id, int command_id, char *arg)
{
    if(!__installed )
        return 0;
    CMSF_CALL("_Z13cmsf_ptz_openiiPc",int (*)(int,int,char*))(channel_id,command_id,arg);
}

static int ptz_c_close( int id)
{
    if(!__installed )
        return 0;
    CMSF_CALL("_Z14cmsf_ptz_closei",int (*)(int))(id);
}

static int ptz_c_next( int id, int *fd, int *state )
{
    if(!__installed )
        return 0;
    CMSF_CALL("_Z13cmsf_ptz_nextiPiS_",int (*)(int,int*,int*))(id,fd,state);
}

static int msb_ptz_c_get_result( int id, void *result, int *res_len)
{
    if(!__installed )
        return 0;
    CMSF_CALL("_Z19cmsf_ptz_get_resultiPvPi",int (*)(int,void*,int*))(id,result,res_len);
}

static int ptz_c_check_auth(char *user, int ch_id)
{
    CMSF_CALL("_Z19cmsf_check_ptz_authPci", int (*)(char *, int))(user, ch_id);
}

static int ptz_c_uninstall(void)
{
    int ret;
    if (!__installed) return 0;
    CMSF_CALL2("_Z18cmsf_ptz_uninstallv",int (*)(void))();
    if(ret!=0)return ret;
    __installed=FALSE;
    if (cmsf_lib)
    {
        dlclose(cmsf_lib);
        cmsf_lib = NULL;
    }

    return 0;
}

#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define TERMINATE_SRPC_SESSION  0xfffffff0
#define MAX_SRPC_CLIENT_NUM	128

static struct srpc_clnt_st
{
    int used;
    int socket;
} __srpc_client_tab[MAX_SRPC_CLIENT_NUM];

static int alloc_client_table(void)
{
    int i;
    for( i=0; i<MAX_SRPC_CLIENT_NUM; i++ )
    {
        if( !__srpc_client_tab[i].used )
        {
            __srpc_client_tab[i].used=1;
            return i;
        }
    }
    return -1;
}

static int read_data( int sock, unsigned char *data, int len )
{
    struct timeval timeout;
    fd_set rd_fdset;
    timeout.tv_sec=3;
    timeout.tv_usec=0;
    FD_ZERO(&rd_fdset);
    FD_SET( sock, &rd_fdset );

    if( (select(FD_SETSIZE, &rd_fdset, NULL, NULL, &timeout))<=0 )
    {
        return -1;
    }
    return read( sock, data, len );
}

static int write_data( int sock, unsigned char *data, int len )
{
    struct timeval timeout;
    fd_set wr_fdset;
    timeout.tv_sec=3;
    timeout.tv_usec=0;
    FD_ZERO(&wr_fdset);
    FD_SET( sock, &wr_fdset );

    if( (select(FD_SETSIZE, NULL, &wr_fdset, NULL, &timeout))<=0 )
    {
        return -1;
    }
    return write( sock, data, len );
}

static int send_socket_packet( int sock, unsigned char *data, int len )
{    
    int slen=0;
    if( sock<0 )return -1;
    while( len>0 )
    {
        slen= write_data( sock, data, len );
        //DEBUGMSG("write byte = %d\n",slen);
        if( slen<0 )
        {
            close( sock );
            return -1;
        }
        data+=slen;
        len-=slen;
    } 
    return 0;
} 

static int recv_socket_packet( int sock, unsigned char *data, int len )
{    
    int slen=0;   
    if( sock<0 )return -1;
    while( len>0 )
    {
        slen = read_data( sock, data, len );
        if( slen<=0 )
        {   
	    close(sock);
            return -1;
        }
        data+=slen;
        len-=slen;
    }
    return len;     
} 

#define SRPC_SOCK(i) 	__srpc_client_tab[i].socket
#define FREE_SRPC(i) 	__srpc_client_tab[i].used=0;
#define COMMON_HAED	int sock;\
			if(client_id<0||client_id>=MAX_SRPC_CLIENT_NUM)\
			    return -1;\
		 	sock=SRPC_SOCK(client_id);	

static int send_packet( int client_id, unsigned char *data, int len )
{
    int sock=SRPC_SOCK(client_id);	
    int r=send_socket_packet( sock, data, len );
    if( r<0)
	FREE_SRPC(client_id);
    return r;
}

static int recv_packet( int client_id, unsigned char *data, int len )
{
    int sock=SRPC_SOCK(client_id);	
    int r=recv_socket_packet( sock, data, len );
    if( r<0)
	FREE_SRPC(client_id);
    return r;
}

static int srpc_client_create( char *srpc_path)
{
    struct sockaddr_un un_name;
	
    int client_id=alloc_client_table();
    if(client_id<0)return client_id;
    SRPC_SOCK(client_id)=socket( AF_LOCAL,SOCK_STREAM, 0 );
    un_name.sun_family = AF_LOCAL;
    strcpy( un_name.sun_path, srpc_path );
    
    if(connect( SRPC_SOCK(client_id), (struct sockaddr *)&un_name, sizeof(un_name)))
    {   
        close(SRPC_SOCK(client_id));
	FREE_SRPC(client_id);
        return -1;
    }
    return client_id;	
}

static int srpc_client_destroy(int client_id)
{    
    COMMON_HAED;
    int num = TERMINATE_SRPC_SESSION;
    send_packet( client_id, (unsigned char *)&num, sizeof(int) );
    FREE_SRPC(client_id);
    close(SRPC_SOCK(client_id));
    return 0;
}


static int srpc_client_send_command(int client_id, int com)
{
    COMMON_HAED;
    return send_packet( client_id, (unsigned char *)&com, sizeof(int) );
}

static int srpc_client_send_int(int client_id, int data)
{
    COMMON_HAED;
    return send_packet( client_id, (unsigned char *)&data, sizeof(int) );
}

static int srpc_client_send_string( int client_id, char *str )
{    
    COMMON_HAED;
    int len=str?strlen(str):0; 
    srpc_client_send_int( client_id, len);
    return send_packet( client_id, (unsigned char *)str, len );	
}

static int srpc_client_send_data( int client_id, unsigned char *data, int len )
{    
    COMMON_HAED;
    if( len==0 )return -1;
    return send_packet( client_id, data, len );
}

static int srpc_client_recv_int( int client_id, int *data)
{
    COMMON_HAED;
    return recv_packet( client_id, (unsigned char *)data, sizeof(int) );
}

#define NVR_SHLIB_CALL( lib_path, lib, r1, r2, fname, t, ... ) \
    if(!lib)\
    lib = dlopen ( lib_path, RTLD_NOW|RTLD_GLOBAL );\
    if(!lib)return r1;\
    void *f=dlsym( lib, fname);\
    if(!f)return r2; \
    return ((t)f) (  __VA_ARGS__ );


#define SMDLIB_LIB_PATH "/usr/lib/libsmd.so.0.0"
#define SMDLIB_CALL1( fname, t, ... ) NVR_SHLIB_CALL(SMDLIB_LIB_PATH, smdlib_lib, -1,-2, fname, t, __VA_ARGS__ )
#define SMDLIB_CALL2( fname, t, ... ) NVR_SHLIB_CALL(SMDLIB_LIB_PATH, smdlib_lib, NULL, NULL,  fname, t, __VA_ARGS__ )

static void *smdlib_lib=NULL;

int http_stream_smd_take(TSharedObjectId id )
{
    SMDLIB_CALL1( "smd_take", int (*)(TSharedObjectId), id );
}

int http_stream_smd_return(TSharedObjectId id )
{
    SMDLIB_CALL1( "smd_return", int (*)(TSharedObjectId), id );
}

unsigned char * http_stream_smd_get(TSharedObjectId id )
{
    SMDLIB_CALL2( "smd_get", unsigned char * (*)(TSharedObjectId), id );
}

int http_stream_smd_put(TSharedObjectId id, unsigned char *addr )
{
    SMDLIB_CALL1( "smd_put", int (*)(TSharedObjectId,unsigned char *), id, addr );
}

int http_stream_smd_lock ( TSmdType type )
{
    SMDLIB_CALL1( "smd_lock", int (*)(TSmdType), type );
}

int http_stream_smd_unlock ( TSmdType type )
{
    SMDLIB_CALL1( "smd_unlock", int (*)(TSmdType), type );
}

#define MSQLIB_LIB_PATH "/usr/lib/libmsq.so.0.0"
#define MSQLIB_CALL1( fname, t, ... ) NVR_SHLIB_CALL(MSQLIB_LIB_PATH, msqlib_lib, -1,-2, fname, t, __VA_ARGS__ )
#define MSQLIB_CALL2( fname, t, ... ) NVR_SHLIB_CALL(MSQLIB_LIB_PATH, msqlib_lib, , ,  fname, t, __VA_ARGS__ )

static void *msqlib_lib=NULL;

void http_stream_gsq_peek( TQID id, int session_id, TSharedObjectId *object, int *is_oos )
{
    MSQLIB_CALL2( "gsq_peek", int (*)(TQID id, int session_id, TSharedObjectId *object, int *is_oos), id, session_id, object, is_oos);
}

int http_stream_gsq_is_data_available( TQID id, int session_id, int *no )
{
    MSQLIB_CALL1( "gsq_is_data_available", int (*)(TQID id, int session_id, int *no), id, session_id, no);
}

#define GMDLIB_LIB_PATH "/usr/lib/libgmd.so.0.0"
#define GMDLIB_CALL1( fname, t, ... ) NVR_SHLIB_CALL(GMDLIB_LIB_PATH, gmdlib_lib, -1,-2, fname, t, __VA_ARGS__ )
#define GMDLIB_CALL2( fname, t, ... ) NVR_SHLIB_CALL(GMDLIB_LIB_PATH, gmdlib_lib, , ,  fname, t, __VA_ARGS__ )

static void *gmdlib_lib=NULL;
static void *gmdlib_lib_path= "/usr/lib/libgmd.so.0.0";

static int check_and_link_all_libs(void)
{
    static char *all_sys_libs[]= {
        "/usr/lib/libstdc++.so",
	"/usr/lib/libixml.so",
	"/usr/lib/libsqlite3.so",
	"/usr/lib/libjrtp-3.3.0.so",
	"/usr/lib/libsyslib.so",
	"/usr/lib/libnmac.so"
    };
    static char *all_libs[]= {
	"vos",
	"smd",
	"msq",
	"utils",
	"log",
	"xmlcfg",
	"msbinf",
	"fclib",
	"naslib",
	"mslib",
	"sclib"
    };
    int i;
    char path[128];
    void *lib = NULL;
#if 0 // BUG!!!
sprintf( path, "cat /proc/%d/maps > /tmp/maps", getpid());
system(path);
#endif
    for( i=0; i<sizeof(all_libs)/sizeof(char *); i++ )
    {
	sprintf( path, "/usr/lib/lib%s.so.0.0", all_libs[i]);
	LOG_CONSOLE("will do link %s", path);
        lib = dlopen ( path, RTLD_LAZY|RTLD_GLOBAL );
	if( !lib )
	{
	    LOG_DEBUG_("link %s failed, error=[%s]", path, dlerror());
	    return -1;
	}
        if(!strcmp(all_libs[i], "utils"))
            gmdlib_lib=lib;
        else if(!strcmp(all_libs[i], "smd"))
            smdlib_lib=lib;
        else if(!strcmp(all_libs[i], "msq"))
            msqlib_lib=lib;
        else {
            dlclose(lib);
            lib = NULL;
        }

	LOG_CONSOLE("link %s succ", path);
    }
    for( i=0; i<sizeof(all_sys_libs)/sizeof(char *); i++ )
    {
	strcpy( path, all_sys_libs[i]);
	LOG_CONSOLE("will do link %s", path);
        lib = dlopen ( path, RTLD_LAZY|RTLD_GLOBAL );
	if( !lib )
	{
	    LOG_DEBUG_("link %s failed, error=[%s]", path, dlerror());
	    return -1;
	}
	LOG_CONSOLE("link %s succ", path);
	dlclose(lib);
	lib = NULL;
    }
    return 0;
}
static void check_shlib_for_http_stream(void)
{
    FILE *file=fopen( gmdlib_lib_path, "r");
    if(!file)
    {
        // SSPro V2 or V3
        check_and_link_all_libs();
        return;
    }
    // SSPro V4 for QVR
    char SS_version[16];
    Get_Private_Profile_String( "SurveillanceStation", "Version", "1.0.0", SS_version, sizeof(SS_version), "/etc/config/qpkg.conf");
    SS_version[1]='\0';
    if( atoi(SS_version) < 5 )
    {
	fclose(file);
	check_and_link_all_libs();
	return;
    }
    fclose(file);
}

int http_stream_gmd_init(void)
{
    check_shlib_for_http_stream();
    GMDLIB_CALL1( "gmd_init", int (*)());
}

void http_stream_gmd_uninit(void)
{
    GMDLIB_CALL2( "gmd_uninit", int (*)());
}

int http_stream_gmd_lock(int val)
{
    GMDLIB_CALL1( "gmd_lock", int (*)(int), val);
}

int http_stream_gmd_unlock(int val)
{
    GMDLIB_CALL1( "gmd_unlock", int (*)(int), val);
}

#else
#include "time_com.h"
#include "gmd.h"
#endif

pthread_mutex_t http_req_mutex = PTHREAD_MUTEX_INITIALIZER;
//#define LOCK()          pthread_mutex_lock( &http_req_mutex )
//#define UNLOCK()        pthread_mutex_unlock( &http_req_mutex )

static long long __http_live_count=0;
long long __ptz_live_count=0;
static BOOL  __result_hand_send=TRUE;
static int in_processing_reconfig=0;
static int proc_close_session_thread_created=0;

#define WATCHDOG_POLL_FQ        60

static void start_http_streaming(void);
static void stop_http_streaming(void);

static int proc_status_dump(void)
{
	static const char *http_status_req_file_path="/tmp/httpd_status_req";
	FILE *file=fopen( http_status_req_file_path, "r");
	if(!file)return -1;
	fclose(file);
	unlink( http_status_req_file_path );
	file=fopen( "/tmp/httpd_status_result", "w");
	if(!file)return -2;
	http_stream_conn_dump(file);
	fclose(file);
	return 0;
}

#define HTTPD_DIAGNOSIS_FILE    "/tmp/.thttpd_diagnosis"

static void check_diagnosis_file(void)
{
    FILE *file=fopen( HTTPD_DIAGNOSIS_FILE, "r");
    if( file )
    {
        fclose(file);
        diagnosis_enabled=1;
        return;
    }
    diagnosis_enabled=0;
}

static int stop_watchdog=0;

static void *proc_watchdog( void* arg )
{
    long long prec_http_live_count=-1;
    long long prec_ptz_live_count=-1;
    int ptz_no_respone_count=0;
    while(1)
    {
        int i;
        check_diagnosis_file();
        for( i=0; i<WATCHDOG_POLL_FQ; i++ )
        {
            if( stop_watchdog==1 )
            {
                stop_watchdog=2;
                return NULL;
            }
            if( (i%5)==0)
                touch_file("/tmp/.thttpd_alive");
            sleep(1);
            if(proc_status_dump()==0)break;
        }
        if( __http_live_count==prec_http_live_count)
        {
LOG_DEBUG_("http main thread live report failed");
            touch_file("/tmp/httpd_is_dead");
            exit(0);
        }
#if 0
        prec_http_live_count=__http_live_count;
        if( __ptz_live_count==prec_ptz_live_count)
        {
            ptz_no_respone_count++;
            if( ptz_no_respone_count >= 2 )
            {
LOG_DEBUG_("http ptz thread live report failed");
                touch_file("/tmp/ptz_is_dead");
                exit(0);
            }
        }
        else
            ptz_no_respone_count=0;
        prec_ptz_live_count=__ptz_live_count;
#endif
    }
    return NULL;
}
static void  start_http_watchdog(void)
{
    pthread_attr_t att;
    pthread_t thread;
    pthread_attr_init( &att);
    pthread_attr_setdetachstate( &att, PTHREAD_CREATE_DETACHED );
    pthread_create(&thread, &att, proc_watchdog, NULL);

}

static void stop_http_watchdog(void)
{
    stop_watchdog=1;
    while( stop_watchdog!=2 )usleep(100000);
    stop_watchdog=0;
}

#if 0
static void *refresh_shm_record(void)
{    
	extern void uman_check_auto_logout(void);
	while(1){
		sleep(2);
		uman_check_auto_logout();
	}	
}

static void start_auto_logout_checker(void)
{
    pthread_attr_t att;
    pthread_t thread;
    pthread_attr_init( &att);
    pthread_attr_setdetachstate( &att, PTHREAD_CREATE_DETACHED );
    pthread_create(&thread, &att, refresh_shm_record, NULL);
}
#endif

#if defined(_VIOSTOR_)
#if 1
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>
#include <execinfo.h>

static int seg_processed=0;

#if defined(NVR_DEBUG)
extern void dbg_file_dump(FILE * file);
#endif

static void handle_seg( int sig )
{
    FILE *file;
    char fname[128], tstr[32];;
    int num;
    if(seg_processed!=0)
        exit(0);
    seg_processed=1;
    TIMESTAMP_t ts=current_time_stamp();
    make_time_string2(tstr, ts );
    num=ts%32;
    sprintf( fname, "/data/.thttpd_%02d", num);
    file=fopen( fname, "w");
    if(file)
    {
        Dl_info info;
        void *array[64];
        int i;
        const char *mod;
        int size=backtrace( array, 64 );

        fprintf( file, "recv sig %d pid=%d on %s\n", sig, getpid(), tstr);
        for (i = 0; i < size; i++)
        {
            dladdr(array[i], &info);
            mod = (info.dli_fname && *info.dli_fname) ? info.dli_fname : "(vdso)";
            if (info.dli_saddr)
                fprintf( file, "%d: %s (%s+0x%lx) [%p](pid=%d)\n", i, mod, info.dli_sname, (long unsigned int)((char *) array[i] - (char *) info.dli_saddr), array[i], getpid());
            else
                fprintf(file, "%d: %s (%p+0x%lx) [%p](pid=%d)\n", i, mod, info.dli_fbase, (long unsigned int)((char *) array[i] - (char *) info.dli_fbase), array[i], getpid());
        }
#if defined(NVR_DEBUG)
        dbg_file_dump(file);
#endif
        fclose(file);
    }
    exit(0);
}
#else
static void handle_seg( int sig )
{
#if 1
    static int count=0;
    char file[64];
    sprintf(file,"/tmp/http_seg_fault_%d_%d", vos_gettid(),count++ );
    touch_file(file);
#endif
    _exit(0);
}
#endif
#endif

// external functions

static int __http_stream_started=0;
static int __smon_ok=0;

int http_stream_smon()
{
#if defined(_VIOSTOR_)
	LOG_DEBUG_("http_stream_smon-start__smon_ok=%d\n", __smon_ok);
	if(!__smon_ok)
    {
        static struct smon_stat smon_states[]= {
            { "state", st_Str }
        };
        if(smon_init()==0)
        {
            smon_reset(SMON_PROC_THTTPD);
            smon_register( SMON_PROC_THTTPD, vos_gettid(), "MainThread", sizeof(smon_states)/sizeof(struct smon_stat), smon_states );
            __smon_ok=1;
        }
    }
	LOG_DEBUG_("http_stream_smon-end__smon_ok=%d\n", __smon_ok);
#endif
    return 0;
}

int http_stream_start( int check )
{
#if 0
    if(!__smon_ok)
    {
        static struct smon_stat smon_states[]= {
            { "state", st_Str }
        };
        if(smon_init()==0)
        {
            smon_reset(SMON_PROC_THTTPD);
            smon_register( SMON_PROC_THTTPD, vos_gettid(), "MainThread", sizeof(smon_states)/sizeof(struct smon_stat), smon_states );
            __smon_ok=1;
        }
    }
#endif
    if( __http_stream_started)return 0;
    if( check )
    {
        FILE *file=fopen( "/home/httpd/cgi-bin/getstream.cgi", "r");
        if(!file)return -1;
        fclose(file);
    }
    start_http_watchdog();
    start_http_streaming();
    __http_stream_started=1;
    return 0;
}
 
int http_stream_check( void )
{
    if(!__http_stream_started)return 0;
    if(!__result_hand_send )
        usleep(30000);
    __http_live_count++;
    http_stream_conn_check_and_clear_all();
    return 0;
}

int http_stream_live_report( httpd_conn *hc )
{
    __http_live_count++;
    if( !hc || hc->conn_fd<0)return 1;
    return 0;
}

static int initialized=0;

int http_stream_stop( void )
{
    if(__http_stream_started==0)return 0;
    unlink("/home/httpd/cgi-bin/ptz.cgi");
    initialized = 0;
    stop_http_watchdog();
    stop_http_streaming();
#if !defined(_VIOSTOR_)
    ptz_c_uninstall();
    if (smdlib_lib)
    {
        dlclose(smdlib_lib);
        smdlib_lib = NULL;
    }
    if (msqlib_lib)
    {
        dlclose(msqlib_lib);
        msqlib_lib = NULL;
    }
    if (gmdlib_lib)
    {
        dlclose(gmdlib_lib);
        gmdlib_lib = NULL;
    }
#endif
    __http_stream_started=0;
    return 0;
}

void http_stream_init_hc( httpd_conn *hc, int cnum )
{
    // added by Jeff on 2007/9/13
    hc->is_http_stream_req = 0;
#ifdef NVR_PTZ
    hc->is_ptz_req = 0;
    hc->ptz_tab_idx = -1;
#endif
    hc->connect_tab_idx=cnum;
    //hc->instance_id=0;
    hc->mark_terminate=0;
}

static BOOL handle_send( httpd_conn *hc, int *proc )
{
    *proc=0;
    if( hc && hc->is_http_stream_req )
    {
        *proc=1;
        return handle_http_streaming(hc);
    }
#ifdef NVR_PTZ
    else if( hc && hc->is_ptz_req )
    {
        *proc=1;
        return ptz_reply_result(hc);
    }
#endif
    return TRUE;
}
int http_stream_handle_send( httpd_conn *hc )
{
    int proc;
    __result_hand_send=handle_send( hc, &proc );
    return proc;
}

int http_stream_handle_signal( void )
{
    //for debug testing by bruce
#if defined(_VIOSTOR_)
    signal( SIGSEGV, handle_seg );
    signal( SIGABRT, handle_seg );
#endif
    return 0;
}

static void start_http_streaming(void)
{
    //gsq_init();
    //smd_init(smdtp_SharedMemory);
    GMD_INIT();
#ifdef NVR_PTZ
    // ptz thread removed, merged into thttpd main thread
    //ptz_init();
#endif
}

static void stop_http_streaming(void)
{
    //stream_terminated=1;
    http_stream_conn_terminate_all(-1, FALSE);
    sleep(1);
    GMD_UNINIT();
}

void http_stream_channel_reconfig( TIMESTAMP_t ts)
{
    static TIMESTAMP_t last_check_time=0;
    FILE *file;
    if(in_processing_reconfig)return;
    if( last_check_time==0 || ts-last_check_time>500)
    {
        int channel_id=-1;
		int r;
        last_check_time=ts;
        file=fopen( HTTP_STREAMING_TASK_END_NOTIFY, "r" );
        if(!file)return;
        in_processing_reconfig=1;
        fscanf( file, "%d", &channel_id );
        fclose(file);
	 	if((r=http_stream_conn_is_closed(channel_id)))
		{
		    unlink(HTTP_STREAMING_TASK_END_NOTIFY);
		}
#ifdef NVR_PTZ
		if( channel_id>=0) {
	        ptz_refresh_channel_info();
		}
#endif
		if(!r)
	        http_stream_conn_terminate_all( channel_id, TRUE );
    }
    // if close_session_thread is not created set in_processing_reconfig=0
    if(!proc_close_session_thread_created)
    {
        in_processing_reconfig=0;
        proc_close_session_thread_created=0;
    }
}

//#define DBG_SMD
#ifdef DBG_SMD
// for debugging smd
#define MAX_DBG_OBJ_NUM		30
static struct _dbg_st
{
    int taked;
    long long id;
    TIMESTAMP_t ts
} __dbg_tab[MAX_DBG_OBJ_NUM];

static void debug_take_object( long long id, TIMESTAMP_t ts )
{
    int i,j;
    for( i=0; i<MAX_DBG_OBJ_NUM; i++ )
    {
        if( __dbg_tab[i].taked==0 )
        {
	    __dbg_tab[i].taked=1;
	    __dbg_tab[i].id=id;
	    __dbg_tab[i].ts=ts;
            for( j=0; j<MAX_DBG_OBJ_NUM; j++ )
		if(  __dbg_tab[j].taked==1 && ts-__dbg_tab[j].ts>60000 )
		{
                    LOG_DEBUG_( "object %LX stay %d msec to long \n", __dbg_tab[j].id, ts-__dbg_tab[j].ts);
    		    exit(0);
		}
	    return;
        }
    }
    LOG_DEBUG_( "object table overflow...\n", id);
    for( i=0; i<MAX_DBG_OBJ_NUM; i++ )
    {
        LOG_DEBUG_( "unfree object %LX\n", __dbg_tab[i].id);
    }
    exit(0);
}

static void debug_return_object( long long id )
{
    int i=0;
    for( i=0; i<MAX_DBG_OBJ_NUM; i++ )
    {
        if( __dbg_tab[i].taked==1 && __dbg_tab[i].id==id )
        {
	    __dbg_tab[i].taked=0;
	    return;
        }
    }
    LOG_DEBUG_( "free untaken object %LX", id);
    exit(0);
}
#endif

// added by Jeff on 2007/8/2 for low memory update check
#define LOW_MEM_UPATE_INDICATOR "/var/.low_mem_update"
#ifndef HTTP_LIVE_STREAMING
typedef long long TIMESTAMP_t;
static TIMESTAMP_t current_time_stamp( void )
{
    struct timeval tp;
    gettimeofday( &tp, NULL);
    long long msecs=((tp.tv_usec-(tp.tv_usec%1000))/1000)%1000;
    return msecs+((long long)tp.tv_sec)*1000;
}
#endif

#if 0
static int low_mem_update_check( httpd_conn* hc )
{
    static TIMESTAMP_t last_check_time=0;
    static in_low_mem_update=0;
    if(!strncmp(hc->encodedurl+9,"miscupdate_status.cgi",21))
    {
	return 0;
    }
    if(!in_low_mem_update)
    {
        TIMESTAMP_t now=current_time_stamp();
        if( last_check_time==0 || now-last_check_time>2500 )
        {
            FILE *file=fopen(LOW_MEM_UPATE_INDICATOR, "r");
            if(file)
            {
                in_low_mem_update=1;
                fclose(file);
            }
        }
    }
    if(!in_low_mem_update)
        return 0;
    // CGIs for monitor, reply bad request
    if(!strncmp(hc->encodedurl+9,"notify.cgi",10)||
       !strncmp(hc->encodedurl,"/get_init_info.cgi",18)||
       !strncmp(hc->encodedurl,"/q_status.cgi",13)||
       !strncmp(hc->encodedurl+9,"mrec.cgi",8)||
       !strncmp(hc->encodedurl+9,"qlive.cgi",9)||
       !strncmp(hc->encodedurl+9,"qadmst.cgi",10))
    {
	httpd_send_err(
	    hc, 403, err403title, "",
	    ERROR_FORM( err403form, "The requested URL '%.80s' is currently not available.\n" ),
	    hc->encodedurl );
        return 1;
    }
    else
    {
        char s[1024];
        //sprintf( s, "/cgi-bin/misc.cgi?function=MISC&subfun=UPDATE&counter=%d", (int)(current_time_stamp()/1000));
        sprintf( s, "function=MISC&subfun=UPDATE&counter=%d", (int)(current_time_stamp()/1000));
	hc->expnfilename=strdup("/home/httpd/cgi-bin/misc.cgi");
  	hc->query=strdup( s );
    }
    return 0;
}
#endif

static int nvr_stream_request(httpd_conn* hc);
#ifdef NVR_PTZ
static int nvr_ptz_request(httpd_conn* hc);
#endif

static void reply_for_http_stream_control( httpd_conn* hc )
{
    char reply[32];
    sprintf( reply, "HTTP/1.0 200 OK%c%c%c%c", 13, 10, 13, 10 );
    write( hc->conn_fd, reply, strlen(reply) );
}

int http_stream_cgi_req( httpd_conn* hc, int *processed )
{
    *processed=0;
    if(!strcmp(hc->expnfilename,"cgi-bin/getstream.cgi"))
    {
        *processed=1; 
        return nvr_stream_request(hc);
    }
#ifdef NVR_PTZ
    if(!strcmp(hc->expnfilename,"cgi-bin/ptz.cgi"))
    {
        *processed=1;
        return nvr_ptz_request(hc);
    }
#endif
#ifdef DYNAMIC_HTTP_STREAM
    if(!strcmp(hc->expnfilename,"cgi-bin/http_stream_start.cgi"))
    {
        *processed=1;
        http_stream_start(0);
        system("/bin/touch /home/httpd/cgi-bin/getstream.cgi;chmod 755 /home/httpd/cgi-bin/getstream.cgi;/bin/touch /home/httpd/cgi-bin/ptz.cgi;chmod 755 /home/httpd/cgi-bin/ptz.cgi");
        reply_for_http_stream_control( hc );
        return 0;
    }
    if(!strcmp(hc->expnfilename,"cgi-bin/http_stream_stop.cgi"))
    {
        *processed=1;
        http_stream_stop();
        system("/bin/rm -rf /home/httpd/cgi-bin/getstream.cgi;/bin/rm -rf /home/httpd/cgi-bin/ptz.cgi;");
        reply_for_http_stream_control( hc );
        return 0;
    }
#endif
    return 0;
}

// added by Jeff on 2007/9/12 for NVR HTTP-based streaming

#if 1
#define LOCK()		pthread_mutex_lock( &crct_mutex )
#define UNLOCK()	pthread_mutex_unlock( &crct_mutex )
#else
#define LOCK()		pthread_mutex_lock( &crct_mutex );LOG_CONSOLE("l s");
#define UNLOCK()	LOG_CONSOLE("u s");pthread_mutex_unlock( &crct_mutex )
#endif

//#define CHK_DOS
#define MAX_STREAM_NO		16

#define RET_HS_CONTINUE		-2

//static long long cur_instance_id=1;

static pthread_mutex_t crct_mutex = PTHREAD_MUTEX_INITIALIZER;

extern void http_stream_conn_start( httpd_conn *hc );
extern void http_stream_conn_clear( httpd_conn *hc );
extern BOOL http_stream_conn_is_exceed_stream_no(void);
extern int http_stream_conn_is_closed(int channel_id);

static void reset_stream_io_info( THttpStreamCtrl *crct )
{
    crct->pkt_idx=-1;
    crct->packed_frame_object_id=-1;
    crct->pkt_object_id=-1;
    crct->cur_packed_frame=NULL;
    crct->cur_timed_data=NULL;
    crct->cur_data=NULL;
    crct->cur_data_to_read=0;
    crct->cur_header_data=NULL;
    crct->cur_header_data_to_read=0;
    crct->frame_header_written_ok=0;
}

#define MAX_NVR_STREAM_URL_LEN		64

static int parse_nvr_stream_req( httpd_conn *hc, int *channel_id, int *stream_id)
{
    char url[MAX_NVR_STREAM_URL_LEN+1], *p, *ptr;
    int ch_ok=0, st_ok=0;
    *stream_id=0;
    *channel_id=-1;
    if(strlen(hc->decodedurl)>MAX_NVR_STREAM_URL_LEN )
        return -1;
    strcpy(url,hc->decodedurl);
    p=strtok_r( url, "?", &ptr);
    if(!p)return -2;
    while((p=strtok_r( NULL, "&", &ptr)))
    {
        int n, l, v;
 	int arg=-1;
        if(!strncmp(p, "ch", 2 ))
        {
            arg=0; 
	    ch_ok=1;
	    n=2;
        }
        else if(!strncmp(p, "channel_id", 10 ))
        {
            arg=0; 
	    ch_ok=1;
	    n=10;
        }
        else if(!strncmp(p, "stream_id", 9 ))
        {
            arg=1; 
	    st_ok=1;
	    n=9;
        }
        if( arg<0)continue;
        l=strlen(p)-n;
	p+=n;
        while(l>0 && *p!='=' ){p++;l--; }
        if(l<=1)return -4;
        v=atoi(p+1); 
        if(arg==0)*channel_id=v;
        else *stream_id=v;
    }
    return ch_ok?0:-3;
}


static pthread_mutex_t srpc_mutex = PTHREAD_MUTEX_INITIALIZER;

#define SRPC_LOCK()		pthread_mutex_lock( &srpc_mutex )
#define SRPC_UNLOCK()		pthread_mutex_unlock( &srpc_mutex )

//#define NO_SRPC_TO_NVRD

static int open_media_stream( int channel_id, int stream_id, int *qid, int *sess_idx )
{
#ifdef NO_SRPC_TO_NVRD
*qid=channel_id;
if( !gsq_ready( *qid ))
{
*qid=-1;
*sess_idx=-1;
    return -1;
}
*sess_idx=gsq_create_session(*qid);
return *sess_idx<0?-2:0;
#endif
//static int count=0;
    int ret=-1;
    int id=srpc_client_create( CMSVR_SRPC_PATH );
#if 0
{
char cmd[256];
sprintf(cmd, "ls -l /proc/%d/fd|grep socket > /tmp/sock%03dB", getpid(),count);
system(cmd);
}
#endif
    if( id<0 )
    {
LOG_CONSOLE("open_media_stream: srpc_client_create failed!");
        return -1;
    }
    srpc_client_send_command( id, CANONICAL_COMMAND_OPEN_MEDIA_STREAM);
    if(srpc_client_send_int( id, channel_id )<0)return -1;
    if(srpc_client_send_int( id, stream_id )<0)return -1;
    if(srpc_client_recv_int( id, &ret )<0)return -1;
    if(srpc_client_recv_int( id, qid )<0)return -1;
    if(srpc_client_recv_int( id, sess_idx )<0)return -1;
    srpc_client_destroy(id);
#if 0
{
char cmd[256];
sprintf(cmd, "ls -l /proc/%d/fd|grep socket > /tmp/sock%03dE", getpid(),count);
system(cmd);
count++;
}
#endif

//LOG_CONSOLE("open_media_stream, ch-%d(st=%d)qid=%d, sess_idx=%d", channel_id, stream_id, *qid, *sess_idx);
    return ret;
}

static int close_media_stream( int channel_id, int qid, int sess_idx )
{
#ifdef NO_SRPC_TO_NVRD
if( !gsq_ready( qid ))
{
return -1;
}
if( qid>=0 && sess_idx>=0)
    gsq_destroy_session(qid,sess_idx);
return 0;
#endif
    int ret;
    int id=srpc_client_create( CMSVR_SRPC_PATH );
    if( id<0 )
    {
LOG_CONSOLE("close_media_stream: srpc_client_create failed!!");
        return -1;
    }
//LOG_CONSOLE("will close_media_stream, ch-%d()qid=%d, sess_idx=%d", channel_id, qid, sess_idx);
    if(srpc_client_send_command( id, CANONICAL_COMMAND_CLOSE_MEDIA_STREAM)<0)return -1;
    if(srpc_client_send_int( id, channel_id )<0)return -1;
    if(srpc_client_send_int( id, qid )<0)return -1;
    if(srpc_client_send_int( id, sess_idx )<0)return -1;
    if(srpc_client_recv_int( id, &ret )<0)return -1;
    srpc_client_destroy(id);
//LOG_CONSOLE("end close_media_stream, ch-%d()qid=%d, sess_idx=%d", channel_id, qid, sess_idx);
    return 0;
}

static void count_fps( httpd_conn *hc )
{
    char s1[64], s2[64];
    MAKE_TIME_STRING2( s1, hc->crct.start_time);
    MAKE_TIME_STRING2( s2, hc->crct.last_write_time);
//LOG_DEBUG_("ch-%d fn=%d, dt=%dms, fps=%9.6f, ts=(%s~%s)", hc->crct.channel_id, (int)hc->crct.cur_frame_num, (int)(hc->crct.last_write_time-hc->crct.start_time), 1000.0*((float)hc->crct.cur_frame_num)/((float)(hc->crct.last_write_time-hc->crct.start_time)), s1, s2);
LOG_CONSOLE("ch-%d fn=%d, dt=%dms, fps=%9.6f", hc->crct.channel_id, (int)hc->crct.cur_frame_num, (int)(hc->crct.last_write_time-hc->crct.start_time), 1000.0*((float)hc->crct.cur_frame_num)/((float)(hc->crct.last_write_time-hc->crct.start_time)));
    hc->crct.cur_frame_num=0;
    hc->crct.start_time=CURRENT_TIME_STAMP();
}

static int streamer_cur_tab_idx=-1;
//static int stream_terminated=0;

#if 0
static int set_wr_fdset( fd_set *wr_fdset)
{
    int i, max=-1;
int num=0;
    FD_ZERO(wr_fdset);
    LOCK();
    for( i=0; i<MAX_NVR_HTTP_STREAM_NUM; i++ )
    {
        if(hc->crct.used && hc->conn_fd>=0 )
        {
num++;
//LOG_CONSOLE("select sock %d", hc->conn_fd);
	    FD_SET( hc->conn_fd, wr_fdset );
            if( hc->conn_fd> max )max=hc->conn_fd;
        }
    }
    UNLOCK();
//if(num>0)LOG_CONSOLE("select num=%d", num);
    return max;
}
#endif

static void prepare_stream_data( httpd_conn *hc )
{
    int offset;
//LOG_CONSOLE("prepare_stream_data ch-%d pkt_idx=%d",  hc->crct.channel_id, hc->crct.pkt_idx);
//LOG_CONSOLE("ch-%d frame %X frame_extra_info_len=%d", hc->crct.channel_id, hc->crct.cur_packed_frame,  hc->crct.cur_packed_frame->frame_extra_info_len);
     hc->crct.pkt_object_id=PKT_OBJECT( hc->crct.cur_packed_frame, hc->crct.pkt_idx);
#if 0
{
char s[32+1];
get_longlong_hex_str( hc->crct.pkt_object_id, s );
LOG_CONSOLE("pkt_object_id=%s", s);
}
#endif
    hc->crct.cur_timed_data=(TTimedData *)SMD_GET( hc->crct.pkt_object_id);
    offset=PKT_OBJECT_INFO( hc->crct.cur_packed_frame, hc->crct.pkt_idx).offset;
    hc->crct.cur_data=hc->crct.cur_timed_data->data+hc->crct.cur_packed_frame->frame_extra_info_len+offset;
    hc->crct.cur_data_to_read=hc->crct.cur_timed_data->size-hc->crct.cur_packed_frame->frame_extra_info_len-offset;
//LOG_CONSOLE("data to read=%d, offset=%d", hc->crct.cur_data_to_read, offset);
}

static void free_stream_data( httpd_conn *hc )
{
    if( !hc || hc->crct.packed_frame_object_id<0 )return;
    TSmdType smd_type=GET_SMD_TYPE( hc->crct.packed_frame_object_id);
    SMD_PUT( hc->crct.pkt_object_id, (unsigned char *)hc->crct.cur_timed_data );
    SMD_PUT( hc->crct.packed_frame_object_id, (unsigned char *)hc->crct.cur_packed_frame );
    SMD_LOCK(smd_type);
    SMD_RETURN( hc->crct.packed_frame_object_id);
#ifdef DBG_SMD
debug_return_object( hc->crct.packed_frame_object_id);
#endif
    SMD_UNLOCK(smd_type);
    reset_stream_io_info(&hc->crct);
}

#define QVG_FRM_HDR(f)    ((TQVG_FRM_HDR *)(hc->crct.frame_header))->f

static void make_frame_header( httpd_conn *hc)
{
    int extra_len, frame_size;
//LOG_CONSOLE("width=%d", hc->crct.cur_packed_frame->width);
//LOG_CONSOLE("height=%d", hc->crct.cur_packed_frame->height);
    //strcpy( (char *)&QVG_FRM_HDR(FourccCode), (char *)hc->crct.cur_packed_frame->fourcc );
    memcpy( (char *)&QVG_FRM_HDR(FourccCode), (char *)hc->crct.cur_packed_frame->fourcc , sizeof(int) );
#ifdef USE_BIG_ENDIAN
    QVG_FRM_HDR(dwFlags)=b2l4(IS_FILE_HEAD_FLAG(hc->crct.cur_packed_frame->flags));
    QVG_FRM_HDR(dwWidth)=b2l4(hc->crct.cur_packed_frame->width);
    QVG_FRM_HDR(dwHeight)=b2l4(hc->crct.cur_packed_frame->height);
    QVG_FRM_HDR(llTimestamp)=b2l8(conv_to_local_time(hc->crct.cur_packed_frame->time_stamp));
#else
    QVG_FRM_HDR(dwFlags)=IS_FILE_HEAD_FLAG(hc->crct.cur_packed_frame->flags);
    QVG_FRM_HDR(dwWidth)=hc->crct.cur_packed_frame->width;
    QVG_FRM_HDR(dwHeight)=hc->crct.cur_packed_frame->height;
    QVG_FRM_HDR(llTimestamp)=conv_to_local_time(hc->crct.cur_packed_frame->time_stamp);
#endif
    QVG_FRM_HDR(szOSDText[0])='\0';
    if( hc->crct.cur_packed_frame-> media_type==MEDIA_TYPE_AUDIO ) // audio
    {
 	TAudioInfo *p=(TAudioInfo *)&hc->crct.frame_header[sizeof(TQVG_FRM_HDR)];
//LOG_CONSOLE("fcc=[%s]", hc->crct.cur_packed_frame->fourcc);
#ifdef USE_BIG_ENDIAN
        p->sample_rate=b2l4(hc->crct.cur_packed_frame->sample_rate);
        p->bits_per_sample=b2l2(hc->crct.cur_packed_frame->bits_per_sample);
        p->audio_channels=b2l2(hc->crct.cur_packed_frame->audio_channels);
#else
        p->sample_rate=hc->crct.cur_packed_frame->sample_rate;
        p->bits_per_sample=hc->crct.cur_packed_frame->bits_per_sample;
        p->audio_channels=hc->crct.cur_packed_frame->audio_channels;
#endif
        extra_len=sizeof(TAudioInfo);
//LOG_DEBUG_("audio extra=%d", extra_len);
        QVG_FRM_HDR(dwWidth)=0;
        QVG_FRM_HDR(dwHeight)=0;
    }
    else
    {
        if( hc->crct.cur_packed_frame->fisheye_type>=0 )
        {
#define FISH_EYE_EX_HDR_LEN     (2+4+2+4)
            static unsigned char __fish_eye_hdr[FISH_EYE_EX_HDR_LEN]= {
#ifdef USE_BIG_ENDIAN
                0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00
#else
                0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00
#endif
                };
            extra_len=FISH_EYE_EX_HDR_LEN;
            BYTE *p=(BYTE *)&hc->crct.frame_header[sizeof(TQVG_FRM_HDR)];
            memcpy( p, __fish_eye_hdr, extra_len );
	    int ftype = hc->crct.cur_packed_frame->fisheye_type;
#ifdef USE_BIG_ENDIAN
	    p[8]=ftype;
	    p[9]=ftype>>8;
	    p[10]=ftype>>16;
	    p[11]=ftype>>24;
            //p[8]=hc->crct.cur_packed_frame->fisheye_type;
#else
	    p[11]=ftype;
	    p[10]=ftype>>8;
	    p[9]=ftype>>16;
	    p[8]=ftype>>24;
            //p[11]=hc->crct.cur_packed_frame->fisheye_type;
#endif

        }
        else
        {
            extra_len=0;
        }
    }
    frame_size=hc->crct.cur_packed_frame->frame_size+extra_len;
#ifdef USE_BIG_ENDIAN
    QVG_FRM_HDR(dwReserved)=b2l4(extra_len);
    QVG_FRM_HDR(dwFrameDataSize)=b2l4(frame_size);
#else
    QVG_FRM_HDR(dwReserved)=extra_len;
    QVG_FRM_HDR(dwFrameDataSize)=frame_size;
#endif
    hc->crct.cur_header_data=hc->crct.frame_header;
    hc->crct.cur_header_data_to_read=sizeof(TQVG_FRM_HDR)+extra_len;
//LOG_CONSOLE("end make_frame_header..., len=%d", hc->crct.cur_header_data_to_read);
}

// return value
//     0: not complete
//     1: complete
//    -1: error occurs
static int __write_stream_data_out( httpd_conn *hc, TIMESTAMP_t ts, int *dnum )
{
    *dnum=0;
    if(!hc->crct.frame_header_written_ok )
    {
 	if( hc->crct.cur_header_data_to_read==0 )
        {
//LOG_CONSOLE("will make_frame_header...");
  	    make_frame_header(hc);
 	}
        int l=write( hc->conn_fd, hc->crct.cur_header_data, hc->crct.cur_header_data_to_read);
        if(l<0)
        {
//LOG_DEBUG_("write frame header failed=%d, errno=%d, sock=%d", l, errno, hc->conn_fd);
	    return -1;
        }
        else if(l==0)
        {
	    return 0;
	}
 	hc->crct.cur_header_data+=l;
	hc->crct.cur_header_data_to_read-=l;
    	//hc->crct.actually_written=TRUE;
//LOG_CONSOLE("write ts succ(l=%d)", l);
	{
#if 0
	char s[64];
	make_time_string2( s, ts );
LOG_CONSOLE("ts=%s", s);
#endif
    	hc->crct.last_write_time=ts;
	if( hc->crct.cur_header_data_to_read!=0 )
	    return 0;
        hc->crct.frame_header_written_ok=1;
if( ts-hc->crct.last_write_time>500)
{
LOG_CONSOLE("ch-%d w1 %dms", hc->crct.channel_id, (int)(ts-hc->crct.last_write_time));
}
	}
    }
    while(1)
    {
        int l;
//LOG_CONSOLE("will write %d", hc->crct.cur_data_to_read);
        l=write( hc->conn_fd, hc->crct.cur_data, hc->crct.cur_data_to_read);
#if 0
TIMESTAMP_t t2=current_time_stamp();
if( t2-ts>100)
{
LOG_CONSOLE("ch-%d w dt=%d", hc->crct.channel_id, (int)(t2-t2));
}
ts=t2;
#endif
        if( l<0)
        {
	    if( errno==EAGAIN )return 0;
//LOG_DEBUG_("write data failed on ch-%d(sock=%d,hc=%X),ret=%d, errno=%d, ptr=%X, len=%d", hc->crct.channel_id, hc->conn_fd, hc, l, errno, hc->crct.cur_data, hc->crct.cur_data_to_read);
	    count_fps( hc );
	    return -1;
	}
        else if( l==0)return 0;
	*dnum+=l;
    	//hc->crct.actually_written=TRUE;
#if 1
if( ts-hc->crct.last_write_time>500)
{
//LOG_DEBUG_("ch-%d w2 %dms", hc->crct.channel_id, (int)(ts-hc->crct.last_write_time));
}
    	hc->crct.last_write_time=ts;
#endif
        if( l!=hc->crct.cur_data_to_read )
        {
	    hc->crct.cur_data+=l;
//LOG_CONSOLE("will update %d as minus %d", hc->crct.cur_data_to_read, l);
	    hc->crct.cur_data_to_read-=l;
	    return 0; 
        }
	hc->crct.pkt_idx++;
        if( hc->crct.pkt_idx>=hc->crct.cur_packed_frame->total_object_num )
        {
	    free_stream_data(hc);
            hc->crct.frame_header_written_ok=0;
 	    hc->crct.cur_header_data_to_read=0; 
	    return 1; 
        }
        prepare_stream_data(hc);
    }
    return 1; // should never occur
}

#define CUR_GLB_SEQ_NO    hc->crct.cur_glb_seq_no[hc->crct.cur_packed_frame->media_type==MEDIA_TYPE_VIDEO?0:1]

#define MAX_LAG_TIME		10000

static int is_needed_to_skipped( httpd_conn *hc, TIMESTAMP_t ts )
{
#if 0
    if( hc->crct.cur_packed_frame->media_type==MEDIA_TYPE_AUDIO)
	return TRUE;
#endif
    if( ts-hc->crct.cur_packed_frame->time_stamp>MAX_LAG_TIME )
	return TRUE;
	
    if( (CUR_GLB_SEQ_NO<0 || CUR_GLB_SEQ_NO+1!=hc->crct.cur_packed_frame->glb_seq_no) && !IS_KEY_FRAME_FLAG(hc->crct.cur_packed_frame->flags) )
    {
//LOG_DEBUG_("ch-%d skip due to out-of-seq %Ld, %Ld", hc->crct.channel_id, CUR_GLB_SEQ_NO, hc->crct.cur_packed_frame->glb_seq_no );
	return TRUE;
    }
    return FALSE;
}

static int write_stream_data_out( httpd_conn *hc, TIMESTAMP_t ts, int *dnum )
{
    int r;
//LOG_CONSOLE("begin write_stream_data_out %d", i);
    r=__write_stream_data_out( hc, ts, dnum );
//LOG_CONSOLE("end write_stream_data_out %d=%d", i, r);
    return r;
}
static int __read_stream_data( httpd_conn *hc, TIMESTAMP_t ts )
{
    int is_oos;
//LOG_CONSOLE("ch-%d  will read stream from q", hc->crct.channel_id);
    hc->crct.packed_frame_object_id=-1;
    GMD_LOCK( hc->crct.qid );
    GSQ_PEEK( hc->crct.qid, hc->crct.sess_idx, &hc->crct.packed_frame_object_id, &is_oos );
    if( hc->crct.packed_frame_object_id<0)
    {
LOG_DEBUG_("read stream from q failed");
        GMD_UNLOCK( hc->crct.qid );
        return -1;
    }
#if 0
LOG_CONSOLE("packed_frame_object_id=%16LX", hc->crct.packed_frame_object_id);
#endif
    TSmdType smd_type=GET_SMD_TYPE( hc->crct.packed_frame_object_id);
    SMD_LOCK(smd_type);
    SMD_TAKE( hc->crct.packed_frame_object_id);
#ifdef DBG_SMD
debug_take_object( hc->crct.packed_frame_object_id, ts);
#endif
    SMD_UNLOCK(smd_type);
    GMD_UNLOCK( hc->crct.qid );
    hc->crct.cur_packed_frame=(TPackedFrame *)SMD_GET( hc->crct.packed_frame_object_id);
//LOG_CONSOLE("cur_packed_frame=%X", (unsigned int)hc->crct.cur_packed_frame);
//LOG_CONSOLE("frame %X frame_extra_info_len=%d", hc->crct.cur_packed_frame,  hc->crct.cur_packed_frame->frame_extra_info_len);
//LOG_CONSOLE("frame %X total_object_num=%d", hc->crct.cur_packed_frame,  hc->crct.cur_packed_frame->total_object_num);
//LOG_CONSOLE("frame %X frame_size=%d", hc->crct.cur_packed_frame,  hc->crct.cur_packed_frame->frame_size);
#if 0
{
char s[32+1];
get_longlong_hex_str( hc->crct.cur_packed_frame->umsid, s );
LOG_CONSOLE("%d frame %X umsid=%s", i, hc->crct.cur_packed_frame, s );
}
#endif
    if( !hc->crct.cur_packed_frame || is_needed_to_skipped(hc,ts))
    {
//LOG_DEBUG_("frame skipped cur_packed_frame=%X, media_type=%d", hc->crct.cur_packed_frame, hc->crct.cur_packed_frame->media_type);
        SMD_PUT( hc->crct.packed_frame_object_id, (unsigned char *)hc->crct.cur_packed_frame);
        SMD_LOCK(smd_type);
        SMD_RETURN( hc->crct.packed_frame_object_id);
#ifdef DBG_SMD
debug_return_object( hc->crct.packed_frame_object_id);
#endif
        SMD_UNLOCK(smd_type);
//LOG_CONSOLE("reset_stream_io_info");
	CUR_GLB_SEQ_NO=-1;
	reset_stream_io_info(&hc->crct);
	return RET_HS_CONTINUE;
    }
    hc->crct.pkt_idx=0;
    CUR_GLB_SEQ_NO=hc->crct.cur_packed_frame->glb_seq_no;

    hc->crct.cur_frame_num++;
#if 0
{
    //if( hc->crct.cur_frame_num%100==0 )
    if( hc->crct.last_write_time-hc->crct.start_time>10*1000)
    {
	count_fps( hc );
    }
}
#endif
//LOG_CONSOLE("ch-%d will prepare_stream_data", hc->crct.channel_id);
    prepare_stream_data(hc);
    return 0;
}

static int read_stream_data( httpd_conn *hc, TIMESTAMP_t ts )
{
    int r;
    r=__read_stream_data( hc, ts );
    return r;
}

static int stream_data_is_ready( httpd_conn *hc )
{
    return hc->crct.packed_frame_object_id<0?0:1;
}

struct media_stream_st
{
    int channel_id;
    int qid;
    int sess_idx;
    BOOL reconfig;
};

static void * proc_close_session( void *args )
{
    struct media_stream_st *ms=(struct media_stream_st *)args;
    if(!args)
    {
        in_processing_reconfig=0;
        return NULL;
    }
    int reconfig=ms->reconfig;
    SRPC_LOCK();
    close_media_stream(  ms->channel_id,  ms->qid, ms->sess_idx );
    SRPC_UNLOCK();
    if(reconfig )
    {
        FILE *file=fopen( HTTP_STREAMING_TASK_END_NOTIFY, "r" );
	int channel_id;
        if(!file)
        {
	     in_processing_reconfig=0;
	     free(args);
             return NULL;
        }
        fscanf( file, "%d", &channel_id );
        fclose(file);
	if( channel_id==ms->channel_id )
            unlink(HTTP_STREAMING_TASK_END_NOTIFY);
	else if( channel_id<0 )
	{
	    // if all media streams closed..
	    if(http_stream_conn_is_closed(-1))
                unlink(HTTP_STREAMING_TASK_END_NOTIFY);
	}
    }
    in_processing_reconfig=0;
    free(args);
    return NULL;
}

static void terminate_stream( httpd_conn *hc, BOOL do_clear, BOOL due_to_reconfig )
{
    struct media_stream_st *ms;
    pthread_attr_t att;
    pthread_t thread;
    if( !hc || hc->crct.qid<0 || hc->crct.sess_idx<0 )
	return;
    free_stream_data(hc);
    ms=(struct media_stream_st *)malloc(sizeof(*ms));
    if(!ms)return;
    ms->reconfig=due_to_reconfig;
    ms->channel_id=hc->crct.channel_id;
    ms->qid=hc->crct.qid;
    ms->sess_idx=hc->crct.sess_idx;
    pthread_attr_init( &att);
    pthread_attr_setdetachstate( &att, PTHREAD_CREATE_DETACHED );
    if(!pthread_create(&thread, &att, proc_close_session, (void *)ms))
    {
        proc_close_session_thread_created=1;
    }
    if( do_clear )
    {
//LOG_CONSOLE("will http_stream_conn_cleari q-%d sess-%d", hc->crct.qid, hc->crct.sess_idx);
        http_stream_conn_clear( hc );
//LOG_CONSOLE("end http_stream_conn_clear");
    }
}

void terminate_http_stream( httpd_conn *hc, BOOL do_clear, BOOL due_to_reconfig )
{
    LOCK();
    terminate_stream( hc, do_clear, due_to_reconfig );
    UNLOCK();
}

#if 0
static void process_stream_timeout(void)
{
    int i;
    LOCK();
    for( i=0; i<maxconnects; i++ )
    {
	c = &connects[cnum];
	if( !c || !c->hc->is_http_stream_req || c->hc->conn_fd<0 )continue;
#if 0
{
LOG_CONSOLE("%d time diff=%d", i, (int)(current_time_stamp()-hc->crct.last_write_time));
}
#endif
    	if( c->hc->crct.last_write_time<current_time_stamp()-2000)
	{
LOG_CONSOLE("idx=%d, terminate_stream due to no write over 3 seconds, sock=%d", i, hc->conn_fd);
	    terminate_stream( hc, TRUE, FALSE);
//exit(0);
	}
    }
    UNLOCK();
}
#endif

void check_and_http_stream_conn_clear( httpd_conn *hc )
{
    LOCK();
    if( hc->is_http_stream_req )
    {
        terminate_stream( hc, FALSE, FALSE);
    }
    UNLOCK();
}

#if 0
#define ADD_SCK(k) { sprintf(ptr, "%d ", k ); ptr+=strlen(ptr);}
#define ADD_SCK2(k) { sprintf(ptr2, "%d ", k ); ptr2+=strlen(ptr2);}
static BOOL process_stream_io(fd_set *wr_fdset, TIMESTAMP_t ts)
{
static int poll_count=0;
char s[256], *ptr=s;
char s2[256], *ptr2=s2;
    int i, pnum=0;
    BOOL not_available=TRUE;
s[0]='\0';
poll_count++;
    LOCK();
    for( i=0; i<MAX_NVR_HTTP_STREAM_NUM; i++ )
    {
        if(!hc->crct.used || hc->conn_fd<0 )continue;
#if 0
{
LOG_CONSOLE("%d time diff=%d", i, (int)(current_time_stamp()-hc->crct.last_write_time));
}
    	if( hc->crct.last_write_time<current_time_stamp()-5000)
	{
LOG_CONSOLE("idx=%d, terminate_stream due to no write over 5 seconds, sock=%d", i, hc->conn_fd);
	    terminate_stream(i);
	}
#endif
        if(FD_ISSET( hc->conn_fd,wr_fdset ))
        {
	    int write_result=1;
ADD_SCK2( hc->conn_fd);
            //hc->crct.actually_written=FALSE;
//LOG_CONSOLE("idx=%d, socket wr ready", i, hc->conn_fd);
            if( stream_data_is_ready(i))
	    {
                write_result=write_stream_data_out( i, ts );
		if( write_result==1 )
		{
ADD_SCK( hc->conn_fd);
		    not_available=FALSE;
		    pnum++;
		}
	    }
            if(write_result==0)
            {
#if 0
                if(!hc->crct.actually_written )
		{
LOG_CONSOLE("idx=%d, terminate_stream due to no actual write, sock=%d", i, hc->conn_fd);
	    	    terminate_stream(i);
		}
#endif
 	 	continue;
	    }
            else if(write_result==1)
	    {
                int no;
                if( hc->crct.qid>=0 && hc->crct.sess_idx>=0 && gsq_is_data_available( hc->crct.qid, hc->crct.sess_idx, &no )&& read_stream_data(i)==0)
                {
//LOG_CONSOLE("idx=%d, socket wr stream out", i, hc->conn_fd);
                    write_result=write_stream_data_out(i, ts);
		    if( write_result==1 )
		    {
ADD_SCK( hc->conn_fd);
		        not_available=FALSE;
		        pnum++;
		    }
                }
            }
            //if(!hc->crct.actually_written || write_result==-1)
            if( write_result==-1)
	    {
LOG_CONSOLE("idx=%d, terminate_stream on ch-%d due to write error, sock=%d", i, hc->crct.channel_id,  hc->conn_fd);
	    	terminate_stream(i, TRUE, FALSE);
	    }
        }
    }
    UNLOCK();
#if 1
{
    if(!not_available)
    {
//log_Enable(TRUE);
//LOG_CONSOLE("p=%d ms(%s),n=%d,wsock=%s, psock=%s, polls=%d", (int)(current_time_stamp()-ts), not_available?"F":"T", pnum, s, s2, poll_count);
//log_Enable(FALSE);
poll_count=0;
    }
}
#endif
    return not_available;
}
#endif

BOOL handle_http_streaming( httpd_conn *hc )
{
    int write_result=1;
    int read_result=0;
    TIMESTAMP_t ts=CURRENT_TIME_STAMP();
    BOOL data_available=FALSE;
#if 0
        if( ts-hc->crct.last_stream_chk_time>500)
	{
LOG_CONSOLE("ch-%d(fd=%d) st %dms", hc->crct.channel_id, hc->conn_fd, (int)(ts-hc->crct.last_stream_chk_time));
	}
    	hc->crct.last_stream_chk_time=ts;
#endif
    LOCK();
    if( stream_data_is_ready(hc))
    {
	int dnum;
        ts=CURRENT_TIME_STAMP();
        write_result=write_stream_data_out( hc, ts, &dnum );
        if(write_result==0)
        {
//LOG_DEBUG_("n1 %d", hc->conn_fd);
            UNLOCK();
	    return TRUE; //dnum==0?FALSE:TRUE;
        }
//LOG_DEBUG_("ch-%d o1 %d", hc->crct.channel_id);
        data_available=TRUE;
    }
    if(write_result==1)
    {
        int no;
	int r=-1;
        ts=CURRENT_TIME_STAMP();
        if( hc->crct.qid>=0 && hc->crct.sess_idx>=0 && (r=GSQ_IS_DATA_AVAILABLE( hc->crct.qid, hc->crct.sess_idx, &no ))&& (read_result=read_stream_data(hc,ts))==0)
        {
//LOG_DEBUG_("socket wr stream out", hc->conn_fd);
//LOG_DEBUG_("ch-%d o2", hc->crct.channel_id);
	    int dnum;
            write_result=write_stream_data_out(hc, ts, &dnum);
	    data_available=TRUE;
  	}
	else
	{
//LOG_CONSOLE("n2 %d(r=%d)", hc->conn_fd, r);
	}
#if 0
{
    if( r==1 )
    {
        if( ts-hc->crct.last_stream_ok_time>500)
	{
LOG_CONSOLE("ch-%d sok %dms", hc->crct.channel_id, (int)(ts-hc->crct.last_stream_ok_time));
	}
    	hc->crct.last_stream_ok_time=ts;
    }
}
#endif
    }
    if( write_result==-1 || (read_result<0 && read_result!=RET_HS_CONTINUE)  )
    {
//LOG_DEBUG_("terminate_stream on ch-%d due to write/read_stream error, sock=%d", hc->crct.channel_id,  hc->conn_fd);
    	terminate_stream(hc, TRUE, FALSE);
        //http_stream_conn_clear( hc );
//LOG_CONSOLE("end terminate_stream");
    }
    UNLOCK();
    return data_available;
}

#define HTTP_STREAM_TIMEOUT	3

static BOOL http_streaming_ready(void)
{
    FILE *file=fopen(NVR_RESET_STREAMING, "r");
    if(!file)return TRUE;
    fclose(file);
    return FALSE;
}

#if 0
static void http_stream_channel_reconfig(TIMESTAMP_t ts)
{
    static TIMESTAMP_t last_check_time=0;
    FILE *file;
LOG_CONSOLE("http_stream_channel_reconfig");
    if( last_check_time==0 || ts-last_check_time>500)
    {
	int channel_id=-1;
        last_check_time=ts;
	file=fopen( HTTP_STREAMING_TASK_END_NOTIFY, "r" );
	if(!file)return;
	fscanf( file, "%d", &channel_id );
	fclose(file);
        http_stream_conn_terminate_all( channel_id, TRUE );
    }
}

static void *http_streamer_execute( void *args )
{
    TIMESTAMP_t prev_ts=0;
    struct timeval timeout;
    timeout.tv_sec=HTTP_STREAM_TIMEOUT;
    timeout.tv_usec=0;
LOG_CONSOLE("http_streamer man loop");
    while(!stream_terminated)
    {
	fd_set wr_fdset;
	int n;
	TIMESTAMP_t ts;
	n=set_wr_fdset(&wr_fdset);
	if( n<0 )
	{
	    sleep(HTTP_STREAM_TIMEOUT);
	    continue;
	}
	if( !http_streaming_ready())
	{
LOG_CONSOLE("http_streaming_ready FALSE, http_stream_conn_terminate_all");
	    http_stream_conn_terminate_all(-1, FALSE);
	}
	ts=current_time_stamp();
	if( prev_ts>0 && ts-prev_ts>1000)
	{
LOG_CONSOLE("lag too long=%d", (int)(ts-prev_ts));
	}
	prev_ts=ts;
	http_stream_channel_reconfig(ts);
        if( (select( n+1,NULL,&wr_fdset,NULL, &timeout))>0 )
        {
	    //static TIMESTAMP_t t=0;
	    if(process_stream_io(&wr_fdset, ts))
		usleep(30000); 
#if 0
if( ts>0 &&(t==0 || ts-t>1000*10 ))
{
show_all_connection();
t=ts;
}
#endif
 	   
	}
        process_stream_timeout();
    } 
}

static void http_streamer_run(int i)
{
    static int executed=0;
    if( !executed )
    {
        executed=1;
        pthread_attr_t att;
        pthread_attr_init( &att);
        pthread_attr_setdetachstate( &att, PTHREAD_CREATE_DETACHED );
        pthread_t thread;
        pthread_create(&thread, &att, http_streamer_execute, NULL);
    }
    // notify http_streamer changed
}
#endif

#define STREAMING_TIMEOUT	30

void check_and_terminate_timeout_http_streams(httpd_conn* hc, TIMESTAMP_t t)
{

    if( hc->crct.last_write_time>0 && t-hc->crct.last_write_time>STREAMING_TIMEOUT*1000)
    {
LOG_CONSOLE("terminate ch-%d (q-%d,sess-%d) due to timeout", hc->crct.channel_id, hc->crct.qid, hc->crct.sess_idx);
    	terminate_stream(hc, TRUE, FALSE);
        //http_stream_conn_clear( hc );
    }
}

#define PTZ_ERR_ILLEGAL_ARGS		0
#define PTZ_ERR_CHANNEL_LOCKED		1
#define PTZ_ERR_SOURCE_ERROR		2
#define PTZ_ERR_PERMISSION_DENIED	3


static void ptz_reply_error_directly( httpd_conn* hc, int err )
{
        char reply[128];
        static char *err_str[]= { "Illegal Arguments",
				  "Channel Locked",
				  "Media Source Error",
				  "Permission Denied",
				  "",
				};
	if( err<0 || err>=sizeof(err_str)/sizeof(char *))
	    return;
        sprintf( reply, "HTTP/1.0 504 %s%c%c%c%c", err_str[err], 13, 10, 13, 10 );
        write( hc->conn_fd, reply, strlen(reply) );
}

static void * proc_nvr_stream_request( void *args )
{
    httpd_conn* hc= (httpd_conn*)args;
    int channel_id=-1, stream_id=0, tab_idx=-1, qid=-1, sess_idx=-1, ret=0;
//log_Enable(TRUE);
    // parsing url to get socket/channel_id/sess_idx
    if((hc?hc->conn_fd:-1)<0)
	return NULL;
//LOG_CONSOLE("will do parse_nvr_stream_req, sock=%d(hc=%X)", hc?hc->conn_fd:-1, hc);
    if((ret=parse_nvr_stream_req(hc, &channel_id, &stream_id))<0)
    {
        // return failure    
LOG_DEBUG_("parse_nvr_stream_req failed");
        hc->mark_terminate=0;
        http_stream_conn_clear(hc);
        return NULL;
    }
//LOG_CONSOLE("decodedurl=[%s], ch=%d, str=%d, ret=%d", hc->decodedurl, channel_id, stream_id, ret );
#if 0
FILE *f=fopen( "/tmp/http_test", "a+");
if(f)
{
fprintf(f, "decodedurl=[%s], ch=%d, str=%d, ret=%d\n", hc->decodedurl, channel_id, stream_id, ret );
fclose(f);
}
#endif
    // get qid/sess_idx
//LOG_CONSOLE("will open_media_stream...");
    SRPC_LOCK();
    if(open_media_stream( channel_id, stream_id, &qid, &sess_idx ))
    {
LOG_CONSOLE("open_media_stream failed, will reply_error on sock %d(%X)", hc?hc->conn_fd:-1, (unsigned int)hc);
        SRPC_UNLOCK();
	//reply_error( hc, 1 );
        hc->mark_terminate=0;
        http_stream_conn_clear(hc);
        return NULL;
    }
    SRPC_UNLOCK();
    // if (qid,sess_idx) exists, fail out
    if(http_stream_conn_media_stream_exist(qid,sess_idx))
    {
LOG_DEBUG_("http_stream_conn_media_stream_exist ch-%d,st-%d, q-%d,sess-%d", channel_id, stream_id, qid, sess_idx );
        http_stream_conn_clear(hc);
        return NULL;
    }
    if( hc->mark_terminate )
    {
LOG_DEBUG_("thttpd: do terminate for mark on fd=%d(hc=%08X", hc->conn_fd, hc);
         hc->mark_terminate=0;
         http_stream_conn_clear(hc);
         return NULL;	         return NULL;
    }
//LOG_CONSOLE("open_media_stream succ!!!");
//LOG_CONSOLE("will do http_streamer_run on socket=%d", hc->conn_fd );
    {
        char reply[32];
	int r;
        sprintf( reply, "HTTP/1.0 200 OK%c%c%c%c", 13, 10, 13, 10 );
        r=write( hc->conn_fd, reply, strlen(reply) );
	if( r<0)
	{
LOG_CONSOLE("reply http error, errno=%d", errno);
            hc->mark_terminate=0;
            http_stream_conn_clear(hc);
	    return NULL;
	}
    }
    fcntl( hc->conn_fd, F_SETFL, O_NONBLOCK );
    LOCK();
    hc->crct.channel_id=channel_id;
    hc->crct.stream_id=stream_id;
    hc->crct.qid=qid;
    hc->crct.sess_idx=sess_idx;
    hc->crct.last_write_time=CURRENT_TIME_STAMP();
    hc->crct.start_time=hc->crct.last_write_time;
    hc->crct.session_start_time=hc->crct.last_write_time;
    hc->crct.last_stream_ok_time=hc->crct.last_write_time;
    hc->crct.last_stream_chk_time=hc->crct.last_write_time;
    hc->crct.cur_frame_num=0;
    hc->crct.cur_glb_seq_no[0]=-1;
    hc->crct.cur_glb_seq_no[1]=-1;
    reset_stream_io_info(&hc->crct);
    UNLOCK();
    http_stream_conn_start(hc);
    return NULL;
}

static int nvr_stream_request(httpd_conn* hc)
{
    pthread_attr_t att;
    pthread_t thread;
    static TIMESTAMP_t last_req_time[MAX_NVR_CHANNEL_NO][MAX_STREAM_NO], ts;
    int channel_id=-1, stream_id=0;
//LOG_CONSOLE("nvr_stream_request");
    if(parse_nvr_stream_req(hc, &channel_id, &stream_id)<0|| channel_id<0 || channel_id>=MAX_NVR_CHANNEL_NO || stream_id<0 || stream_id>=MAX_STREAM_NO)
    {
	//reply_error( hc, 0 );
        //http_stream_conn_clear(hc);
LOG_CONSOLE("parse_nvr_stream_req failed");
        return -3;
    }
#ifdef CHK_DOS
    ts=CURRENT_TIME_STAMP();
    if( last_req_time[channel_id][stream_id]>0 && ts-last_req_time[channel_id][stream_id]<2000)
    {
LOG_CONSOLE("req rejected due to DOS");
	//reply_error( hc, 0 );
        //http_stream_conn_clear(hc);
	return -4;
    }
    last_req_time[channel_id][stream_id]=ts;
#endif
    if(http_stream_conn_is_exceed_stream_no() && strcmp(httpd_ntoa(&hc->client_addr),"127.0.0.1"))
    {
LOG_CONSOLE("nvr_stream_request sock=%d(hc=%X), excced max num", hc->conn_fd, hc);
	//reply_error( hc, 0 );
        //http_stream_conn_clear(hc);
        return -1000;
    }
//LOG_CONSOLE("nvr_stream_request sock=%d", hc->conn_fd);
    if( !http_streaming_ready())
    {
	//reply_error( hc, 0 );
        //http_stream_conn_clear(hc);
	return -2;
    }
    pthread_attr_init( &att);
    pthread_attr_setdetachstate( &att, PTHREAD_CREATE_DETACHED );
    hc->is_http_stream_req=1;
    hc->crct.last_write_time=0;
    hc->crct.start_time=0;
    hc->crct.session_start_time=0;
    hc->crct.last_stream_ok_time=0;
    hc->crct.last_stream_chk_time=0;
    hc->crct.qid=-1;
    hc->crct.sess_idx=-1;
    //hc->instance_id=cur_instance_id++;
    hc->responselen=0;
    pthread_create(&thread, &att, proc_nvr_stream_request, (void *)hc);
    hc->status = 200;
    hc->bytes = CGI_BYTECOUNT;
    hc->should_linger = 0;
    return 0;
}

#ifdef NVR_PTZ

#define MAX_NVR_PTZ_REQ_NUM	16

// Note: To protect the control table access by both the main thread and ptz thread
//       without using locks, a rule has been established such that
//       1) state is only set by ptz thread
//       2) main thread process reply only when state becomes DONE or ERROR
//       3) ptz thread access fd when only state=READ or WRITE
//       4) is_timeout is set by main thread, and referenced by ptz thread
//       5) assure that 'used may be not reliable, but no harm on anything'
static struct 
{
    httpd_conn* hc;
    int msb_id; 
    int fd;
    int state;
    BOOL used;
    TIMESTAMP_t start_time;
    BOOL is_timeout;
} ptz_control_tab[MAX_NVR_PTZ_REQ_NUM];

//static BOOL ptz_fd_select_changed=FALSE;

static BOOL ptz_channel_used( int channel_id)
{
    int i;
    for( i=0; i<MAX_NVR_PTZ_REQ_NUM; i++ )
    {
        if(ptz_control_tab[i].used && ptz_control_tab[i].hc->ptz_channel_id==channel_id )
	{
LOG_CONSOLE("idx=%d ptz channel %d used on cmd %d", i, ptz_control_tab[i].hc->ptz_channel_id, ptz_control_tab[i].hc->ptz_command_id);
            return TRUE;
	}
    }
    return FALSE;
}

static int set_ptz_fdset( fd_set *rd_fdset, fd_set *wr_fdset, fd_set *ex_fdset )
{
    int i, max=-1;
    FD_ZERO(rd_fdset);
    FD_ZERO(wr_fdset);
    FD_ZERO(ex_fdset);
    for( i=0; i<MAX_NVR_PTZ_REQ_NUM; i++ )
    {
        if( !ptz_control_tab[i].used || (ptz_control_tab[i].state!=NVR_PTZ_STATE_READ&&ptz_control_tab[i].state!=NVR_PTZ_STATE_WRITE))
            continue;
        if( ptz_control_tab[i].is_timeout )
        {
            ptz_control_tab[i].state=NVR_PTZ_STATE_ERROR;
            ptz_control_tab[i].start_time=0;
            continue;
        }
	int fd=ptz_control_tab[i].fd;
        if(ptz_control_tab[i].used && fd>=0 )
        {
	    if( ptz_control_tab[i].state==NVR_PTZ_STATE_READ )
	    {
//LOG_DEBUG_("R%d", fd);
LOG_DIAGNOSIS("b-sr%d|%d,ch-%d", i, fd, ptz_control_tab[i].hc->ptz_channel_id);
		if(__smon_ok)
SMON_SET_S( 0, "b-sr");
	        FD_SET( fd, rd_fdset );
	        FD_SET( fd, ex_fdset );
                if( fd> max )max=fd;
		if(__smon_ok)
SMON_SET_S( 0, "e-sr");
LOG_DIAGNOSIS("e-sr");
            }
	    else if( ptz_control_tab[i].state==NVR_PTZ_STATE_WRITE )
	    {
LOG_DIAGNOSIS("b-sw%d|%d,ch-%d", i, fd, ptz_control_tab[i].hc->ptz_channel_id);
if(__smon_ok)
SMON_SET_S( 0, "b-sw");
//LOG_DEBUG_("WE=%d", fd );
	        FD_SET( fd, wr_fdset );
	        FD_SET( fd, ex_fdset );
                if( fd> max )max=fd;
if(__smon_ok)
SMON_SET_S( 0, "e-sw");
LOG_DIAGNOSIS("e-er");
//LOG_DEBUG_("W%d", fd);
	    }
        }
    }
    return max;
}

static void ptz_destroy_by_idx( int i )
{
//LOG_CONSOLE("D%d|%d", i, ptz_control_tab[i].fd);
    ptz_c_close(ptz_control_tab[i].msb_id);
    ptz_control_tab[i].hc=NULL;
    ptz_control_tab[i].msb_id=-1;
    ptz_control_tab[i].fd=-1;
    ptz_control_tab[i].state=-1;
    ptz_control_tab[i].start_time=0;
    ptz_control_tab[i].is_timeout=0;
    ptz_control_tab[i].used=FALSE;
    //ptz_fd_select_changed=TRUE;
}

static void process_ptz_error( int i)
{
    char reply[128];
    int r;
//LOG_CONSOLE(" reply errorto conn_fd=%d", ptz_control_tab[i].hc->conn_fd);
    sprintf( reply, "HTTP/1.0 506 PTZ Request Failed.%c%c%c%c", 13, 10, 13, 10 );
    r=write( ptz_control_tab[i].hc->conn_fd, reply, strlen(reply) );
    if( r<0)
    {
//LOG_CONSOLE("reply http for ptz error, errno=%d", errno);
    }
    // clear connection!
    http_stream_conn_clear_ptz( ptz_control_tab[i].hc );
LOG_CONSOLE("reply_ptz_error on %d", i);
    ptz_destroy_by_idx(i);
}

static void reply_ptz_succ( int msb_id, int ptz_tab_idx )
{
    char result[MAX_PTZ_RESULT_LEN];
    int res_len=MAX_PTZ_RESULT_LEN;
    char reply[256+MAX_PTZ_RESULT_LEN];
    int r;
    if(msb_ptz_c_get_result( msb_id, (void *)result, &res_len))
    {
	strcpy( result, "no result");
    }
    else
    {
	result[res_len]='\0';
    }
    sprintf( reply, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n", strlen(result)+4, result );
//LOG_CONSOLE("reply=[%s]", reply);

    r=write( ptz_control_tab[ptz_tab_idx].hc->conn_fd, reply, strlen(reply) );
    if( r<0)
    {
LOG_CONSOLE("reply http error, errno=%d", errno);
    }
    else
    {
    }
   // clear connection!
   http_stream_conn_clear_ptz( ptz_control_tab[ptz_tab_idx].hc );
LOG_CONSOLE("reply_ptz_succ on %d", ptz_tab_idx);
   ptz_destroy_by_idx(ptz_tab_idx);
}

static void proc_ptz_name_resolv(void)
{
    int i;
    for( i=0; i<MAX_NVR_PTZ_REQ_NUM; i++ )
    {
        if (!ptz_control_tab[i].used) {
            continue;
        }

	if( ptz_control_tab[i].state==NVR_PTZ_STATE_RESOLV_NAME )
        {
    	    int ptz_state=ptz_control_tab[i].state;
LOG_DIAGNOSIS("b-nr%d,ch-%d", i, ptz_control_tab[i].hc->ptz_channel_id);
if(__smon_ok)
{
char s[32];
sprintf( s, "b-nr%d_%d_%08X",  i, ptz_control_tab[i].hc->ptz_channel_id, ptz_control_tab[i].hc );
SMON_SET_S( 0, s);
}
    	    ptz_c_next( ptz_control_tab[i].msb_id, &ptz_control_tab[i].fd, &ptz_state );
if(__smon_ok)
SMON_SET_S( 0, "e-nr");
LOG_DIAGNOSIS("e-nr");
    	    ptz_control_tab[i].state=ptz_state;
        }
    }
}

static void proc_ptz_fdset( fd_set *rd_fdset, fd_set *wr_fdset, fd_set *ex_fdset )
{
    int i;
    for( i=0; i<MAX_NVR_PTZ_REQ_NUM; i++ )
    {
        int fd=ptz_control_tab[i].fd;
        if (!ptz_control_tab[i].used) {
            continue;
        }

	if( fd<0 )
	{
#if 0
            if( ptz_control_tab[i].state!=NVR_PTZ_STATE_RESOLV_NAME )
            {
//LOG_CONSOLE("fd=%d for %d", fd, i );
            //ptz_control_tab[i].used=FALSE;
LOG_DIAGNOSIS("ptz:error%d|%d,ch-%d", i, fd, ptz_control_tab[i].hc->ptz_channel_id);
	        ptz_control_tab[i].state=NVR_PTZ_STATE_ERROR;
	    //process_ptz_error(i);
            }
#endif
	    continue;
	}
        else if( fd>=1024)
        {
LOG_DEBUG_( "BUG fd=%d, idx=%d, state=%d, ch-%d", fd, i, ptz_control_tab[i].state, ptz_control_tab[i].hc->ptz_channel_id );
             continue;
        }

        if(FD_ISSET( fd,ex_fdset ))
	{
LOG_DIAGNOSIS("ptz:error(2)%d|%d,ch-%d", i, fd, ptz_control_tab[i].hc->ptz_channel_id);
	    ptz_control_tab[i].state=NVR_PTZ_STATE_ERROR;
	    //process_ptz_error(i);
	    continue;
	}
        else if(FD_ISSET( fd,rd_fdset )|| FD_ISSET( fd,wr_fdset ))
	{
//LOG_CONSOLE("will do ptz_c_next for sock %d", ptz_control_tab[i].fd);
LOG_DIAGNOSIS("bn%d|%d,ch-%d", i, fd, ptz_control_tab[i].hc->ptz_channel_id);
    	    int ptz_state=ptz_control_tab[i].state;
            int r = 0;
            if( ptz_control_tab[i].is_timeout)
    	        ptz_control_tab[i].state=NVR_PTZ_STATE_ERROR;
            else
            {
    	        r=ptz_c_next( ptz_control_tab[i].msb_id, &ptz_control_tab[i].fd, &ptz_state );
    	        ptz_control_tab[i].state=ptz_state;
            }
LOG_DIAGNOSIS("en%d=%d,s=%d", fd, r, ptz_control_tab[i].state);
//LOG_CONSOLE("end do ptz_c_next for sock %d", ptz_control_tab[i].fd);
#if 0
	    if( ptz_control_tab[i].fd<0 && ptz_control_tab[i].state!=NVR_PTZ_STATE_RESOLV_NAME )
	    {
                //ptz_control_tab[i].used=FALSE;
LOG_DIAGNOSIS("ptz:error(3)%d|%d,ch-%d", i, fd, ptz_control_tab[i].hc->ptz_channel_id);
	        ptz_control_tab[i].state=NVR_PTZ_STATE_ERROR;
	        //process_ptz_error(i);
	        continue;
	    }
	    if( ptz_control_tab[i].state==NVR_PTZ_STATE_ERROR )
	    {
	        ptz_control_tab[i].state=NVR_PTZ_STATE_ERROR;
	        //process_ptz_error(i);
	    }
	    else if( ptz_control_tab[i].state==NVR_PTZ_STATE_DONE )
	    {
//LOG_CONSOLE("will do reply_ptz_succ for sock %d", ptz_control_tab[i].fd);
		//reply_ptz_succ( ptz_control_tab[i].msb_id, i );
//LOG_CONSOLE("end do reply_ptz_succ for sock %d", ptz_control_tab[i].fd);
	    }
#endif
	}
    }
}

#define WTIME		50000

static void *proc_ptz( void )
{
    struct timeval timeout;
    timeout.tv_sec=0;
    timeout.tv_usec=WTIME;
    fd_set wr_fdset, rd_fdset, ex_fdset;
    int n;
    int i;
#if defined(_VIOSTOR_)
    static struct smon_stat smon_states[]= {
        { "state", st_Str }
    };
#endif
    //smon_register( SMON_PROC_THTTPD, vos_gettid(), "PTZThread", sizeof(smon_states)/sizeof(struct smon_stat), smon_states );

//LOG_CONSOLE("will do ptz_c_install()");
#if defined(_VIOSTOR_)
    while( memory_insufficient())
    {
	sleep(5);
    }
#endif
    // wait for HTTP/PTZ enabled
    while(1)
    {
        FILE *file=fopen("/home/httpd/cgi-bin/ptz.cgi", "r");
        if(file)
        {
            fclose(file);
            break;
        }
        __ptz_live_count++;
        sleep(2);
    }
    while(ptz_c_install())
    {
//LOG_DEBUG_("ptz_c_install() failed");
        __ptz_live_count++;
        sleep(2);
    }
//LOG_CONSOLE("will do ptz_c_refresh_channel_info()");
    ptz_c_refresh_channel_info();
//LOG_CONSOLE("end do ptz_c_refresh_channel_info()");
    for( i=0; i<MAX_NVR_PTZ_REQ_NUM;i++)
	ptz_control_tab[i].used=FALSE;
//LOG_CONSOLE("proc_ptz enter main loop");
    while(1)
    {
        int r;
        extern long long __ptz_live_count;
        __ptz_live_count++;
        proc_ptz_name_resolv(); 
	n=set_ptz_fdset(&rd_fdset, &wr_fdset, &ex_fdset);
        //ptz_fd_select_changed=false;
 	if( n<=0)
  	{
//LOG_DEBUG_("proc_ptz wait for 0.5 sec");
//LOG_DEBUG_("w");
	    usleep(WTIME);
	    continue;
	}
//SMON_SET_S( 0, "b-z-s");
        if( ((r=select( n+1,&rd_fdset,&wr_fdset,&ex_fdset, &timeout)))>0 )
	{
//SMON_SET_S( 0, "e-z-G");
//LOG_DEBUG_("s");
	    proc_ptz_fdset(&rd_fdset, &wr_fdset, &ex_fdset);
	}
	else if( r<0 )
	{
//SMON_SET_S( 0, "e-z-E");
LOG_DEBUG_("httpd/ptz:sel=%d,eno=%d", r, errno);
	    usleep(WTIME);
	}
	else
	{
//SMON_SET_S( 0, "e-z-N");
	}
    }
    return NULL;
}

//static int initialized=0;

void new_ptz_init(void)
{
    struct stat st;
    int i = 0;
    if (stat("/home/httpd/cgi-bin/ptz.cgi", &st))
        return;

    if(!initialized)
    {
        if(ptz_c_install())return;
        ptz_c_refresh_channel_info();
        for( i=0; i<MAX_NVR_PTZ_REQ_NUM;i++)
        {
            ptz_control_tab[i].hc=NULL;
            ptz_control_tab[i].msb_id=-1;
            ptz_control_tab[i].fd=-1;
            ptz_control_tab[i].state=-1;
            ptz_control_tab[i].start_time=0;
            ptz_control_tab[i].is_timeout=0;
            ptz_control_tab[i].used=FALSE;
        }
	initialized=1;
LOG_DEBUG_("new_ptz_init() done");
    }
}

void http_stream_handle_ptz(void)
{
    fd_set wr_fdset, rd_fdset, ex_fdset;
#if 0
    static int initialized=0;
#endif
    int n, r;
    struct timeval timeout;
    if( !__http_stream_started)return;
    timeout.tv_sec=0;
    timeout.tv_usec=WTIME;
    __ptz_live_count++;
// marked by Hugo. 2012-04-30, move to new_ptz_init()
#if 0
    if(!initialized)
    {
        FILE *file=fopen("/home/httpd/cgi-bin/ptz.cgi", "r");
        int i;
        if(!file)return;
        fclose(file);
        if(ptz_c_install())return;
        ptz_c_refresh_channel_info();
        for( i=0; i<MAX_NVR_PTZ_REQ_NUM;i++)
        {
            ptz_control_tab[i].hc=NULL;
            ptz_control_tab[i].msb_id=-1;
            ptz_control_tab[i].fd=-1;
            ptz_control_tab[i].state=-1;
            ptz_control_tab[i].start_time=0;
            ptz_control_tab[i].is_timeout=0;
	    ptz_control_tab[i].used=FALSE;
        }
	initialized=1;
    }
#endif
    proc_ptz_name_resolv(); 
    n=set_ptz_fdset(&rd_fdset, &wr_fdset, &ex_fdset);
    if( n<=0)return;
    if( ((r=select( n+1,&rd_fdset,&wr_fdset,&ex_fdset, &timeout)))>0 )
    {
        proc_ptz_fdset(&rd_fdset, &wr_fdset, &ex_fdset);
    }
    else if( r<0 )
    {
LOG_DEBUG_("httpd/ptz:sel=%d,eno=%d", r, errno);
    }
}

void ptz_init(void)
{
    pthread_attr_t att;
    pthread_t thread;
    pthread_attr_init( &att);
    pthread_attr_setdetachstate( &att, PTHREAD_CREATE_DETACHED );
    pthread_create(&thread, &att, proc_ptz, NULL);
}

int ptz_create(httpd_conn *hc)
{
    int i;
    int msb_id; 
    int fd=-1;
    int state;

    if((msb_id=ptz_c_open( hc->ptz_channel_id, hc->ptz_command_id, hc->ptz_arg))<0)return -1;
    if(ptz_c_next( msb_id, &fd, &state ))
    {
        ptz_reply_error_directly( hc, PTZ_ERR_SOURCE_ERROR );
        ptz_c_close(msb_id);
        return -2;
    }
    for( i=0; i<MAX_NVR_PTZ_REQ_NUM;i++)
    {
	if(!ptz_control_tab[i].used )
	{
//LOG_DEBUG_("ptz_create %d, fd=%d, hc=%x", i, fd, hc);
	    ptz_control_tab[i].hc=hc;
	    ptz_control_tab[i].msb_id=msb_id;
	    ptz_control_tab[i].fd=fd;
	    ptz_control_tab[i].state=state;
	    ptz_control_tab[i].start_time=CURRENT_TIME_STAMP();
	    ptz_control_tab[i].is_timeout=0;
	    ptz_control_tab[i].used=TRUE;
            hc->ptz_tab_idx=i;
//LOG_CONSOLE("C%d|fd=%d,s=%d,cmd=%d, hc=%08X", i, ptz_control_tab[i].fd, state,  hc->ptz_command_id, hc);
            if( state==NVR_PTZ_STATE_DONE )
            {
                hc->is_ptz_req=TRUE;
	        //reply_ptz_succ( msb_id, i );
	        return 0;
    	    }
    	    //ptz_fd_select_changed=TRUE;
	    return 0;
	}
    }
    ptz_c_close(msb_id);
    return -3;
}


#if 0
int ptz_destroy(httpd_conn *hc)
{
    int i;

    for( i=0; i<MAX_NVR_PTZ_REQ_NUM;i++)
	if(ptz_control_tab[i].used && ptz_control_tab[i].hc==hc )
        {
	    return ptz_destroy_by_idx(i);
        }
    return -1;
}
#endif

BOOL ptz_reply_result(httpd_conn *hc)
{
    int i;
//LOG_DEBUG_("ptz_reply_result hc=%08X, idx=%d", hc, hc->ptz_tab_idx );
    if((i=hc->ptz_tab_idx)<0 )
        return FALSE;
//LOG_DEBUG_("idx=%d hc=%08X,rl=%d,fd=%d|%c,s=%d", i, hc, hc->ptz_tab_idx, ptz_control_tab[i].fd, ptz_control_tab[i].used?'T':'F', ptz_control_tab[i].state);
    if( ptz_control_tab[i].state==NVR_PTZ_STATE_DONE )
    {
        reply_ptz_succ( ptz_control_tab[i].msb_id, i );
        return TRUE;
    }
    else if( ptz_control_tab[i].state==NVR_PTZ_STATE_ERROR )
    {
        process_ptz_error(i);
        return TRUE;
    }
    return FALSE;
}

static int parse_nvr_ptz_req( httpd_conn *hc, int *channel_id, int *command_id, char *arg_str)
{
// Hugo modify for bug 27998
// "/cgi-bin/ptz.cgi?ch=1&command=22&arg="  length is 37
//    char url[MAX_NVR_STREAM_URL_LEN+1], *p;
    char url[MAX_PTZ_ARG_LEN+1+37], *p, *ptr;
    int ch_ok=0, command_ok=0;
    *channel_id=-1;
    *command_id=-1;
    *arg_str='\0';

    if(strlen(hc->decodedurl)>MAX_PTZ_ARG_LEN+37 )
        return -1;
    strcpy(url,hc->decodedurl);
    p=strtok_r( url, "?", &ptr);
    if(!p)return -2;
    while((p=strtok_r( NULL, "&", &ptr)))
    {
        int n, l, v;
 	int arg=-1;
        if(!strncmp(p, "ch", 2 ))
        {
            arg=0; 
	    ch_ok=1;
	    n=2;
        }
        else if(!strncmp(p, "command", 7 ))
        {
            arg=1; 
	    command_ok=1;
	    n=7;
        }
        else if(!strncmp(p, "arg", 3 ))
        {
            arg=2; 
	    n=3;
        }
        if( arg<0)continue;
        l=strlen(p)-n;
	p+=n;
        while(l>0 && *p!='=' ){p++;l--; }
        if(l<=1)return -4;
	if( arg==0 || arg==1 )
	{
            v=atoi(p+1); 
            if(arg==0)*channel_id=v;
            else *command_id=v;
	}
	else
	    strncpy( arg_str, p+1, MAX_PTZ_ARG_LEN-1 );
    }
    return ch_ok&&command_ok?0:-3;
}

static int nvr_ptz_request(httpd_conn *hc)
{
//LOG_CONSOLE("begin nvr_ptz_request");
    int r, ptz_channel_id, ptz_command_id;
    char ptz_arg[MAX_PTZ_ARG_LEN];
    char authinfo[500] = {0};
    if((r=parse_nvr_ptz_req(hc, &ptz_channel_id, &ptz_command_id, ptz_arg))<0|| ptz_channel_id<0 || ptz_channel_id>=MAX_NVR_CHANNEL_NO)
    {
LOG_CONSOLE("nvr_ptz_request failed=%d, ch=%d,command=%d!", r, ptz_channel_id, ptz_command_id);
        ptz_reply_error_directly( hc, PTZ_ERR_ILLEGAL_ARGS );
 	return -1;
    }
    // Hugo Liao add 2014-11-24: fix 45222, check permission in thttpd
    if (strcmp(hc->authorization, "Basic mjptzqnap2009") &&
        strcmp(hc->authorization, "Basic mjptzqnap209Opo6bc6p2qdtPQ=="))
    {
        int l = b64_decode( &(hc->authorization[6]), authinfo, sizeof(authinfo)-1 );
        char *pass = strstr(authinfo, ":");
        authinfo[l] = '\0';
        if (pass) *pass = '\0';

        if (ptz_c_check_auth(authinfo, ptz_channel_id) == 0)
        {
            ptz_reply_error_directly( hc, PTZ_ERR_PERMISSION_DENIED );
            return -1;
        }
    }
    // End

    if( ptz_channel_used(ptz_channel_id))
    {
LOG_CONSOLE("ch-%d ptz locked", ptz_channel_id );
        ptz_reply_error_directly( hc, PTZ_ERR_CHANNEL_LOCKED );
 	return -1;
    }
    hc->ptz_channel_id=ptz_channel_id;
    hc->ptz_command_id=ptz_command_id;
    strcpy(  hc->ptz_arg, ptz_arg );
//LOG_CONSOLE("begin ptz_create");
    if(ptz_create(hc))return -2;
    hc->is_ptz_req=TRUE;
    hc->responselen=0;
    hc->status = 200;
    hc->bytes = CGI_BYTECOUNT;
    hc->should_linger = 0;
//LOG_DEBUG_("end nvr_ptz_request succ!");
    return 0;
}

int ptz_refresh_channel_info(void)
{
    return ptz_c_refresh_channel_info();
}

void check_ptz_timeout( httpd_conn *hc, TIMESTAMP_t t )
{
    int i=hc->ptz_tab_idx;
    if( i<0 || i>=MAX_NVR_PTZ_REQ_NUM )return;
    if( ptz_control_tab[i].used && ptz_control_tab[i].start_time>0 && t-ptz_control_tab[i].start_time>15000)
    {
        ptz_control_tab[i].is_timeout=1;
LOG_CONSOLE("ch-%d command=%d ptz timeout", hc->ptz_channel_id, hc->ptz_command_id);
    }
}

#endif


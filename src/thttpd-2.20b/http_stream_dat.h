#ifndef __HTTP_STREAM_DAT_H__
#define __HTTP_STREAM_DAT_H__

#include <pthread.h>
#include <stdint.h>

#if defined(_VIOSTOR_)

#include "csrpc.h"
#include "gmsdbq.h"
#include "mdata.h"
#include "cmapcmd.h"
#include "log.h"
#include "smon.h"
#ifdef NVR_PTZ
#include "ptz_c.h"
#endif

#define CURRENT_TIME_STAMP	current_time_stamp
#define MAKE_TIME_STRING2	make_time_string2
#define LOG_DEBUG_		log_Debug
#define LOG_CONSOLE		log_Console
#define SMON_SET_S		smon_set_s

#define SMD_TAKE		smd_take
#define SMD_RETURN		smd_return
#define SMD_GET			smd_get
#define SMD_PUT			smd_put
#define SMD_LOCK		smd_lock
#define SMD_UNLOCK		smd_unlock

#define GMD_INIT		gmd_init
#define GMD_UNINIT		gmd_uninit
#define GMD_LOCK		gmd_lock
#define GMD_UNLOCK		gmd_unlock

#define GSQ_PEEK		gsq_peek
#define GSQ_IS_DATA_AVAILABLE	gsq_is_data_available

#else

#include <stdarg.h>
#include "gtypes.h"
#include "mdata.h"
#include "nvr_ptz.h"
#include "cmapcmd.h"

typedef long long TSharedObjectId;
typedef int TSmdType;

#ifdef USE_BIG_ENDIAN
#define GET_SMD_TYPE(id) ((TSmdType)((((unsigned char *)&id)[0])))
#else
#define GET_SMD_TYPE(id) ((TSmdType)((((unsigned char *)&id)[7])))
#endif

#define TQID    int

extern TIMESTAMP_t http_stream_current_time_stamp( void );
extern void http_stream_make_time_string2( char *date_time, TIMESTAMP_t time_stamp );
extern void http_stream_log_Console( char *fmt, ...);
extern void http_stream_log_Debug( char *fmt, ...);
extern int http_stream_smon_set_s( int idx, char *state );

extern int http_stream_smd_take(TSharedObjectId );
extern int http_stream_smd_return(TSharedObjectId );
extern unsigned char * http_stream_smd_get(TSharedObjectId id );
extern int http_stream_smd_put(TSharedObjectId id, unsigned char *addr );
extern int http_stream_smd_lock ( TSmdType type );
extern int http_stream_smd_unlock ( TSmdType type );

extern int http_stream_gmd_init(void);
extern void http_stream_gmd_uninit(void);
extern int http_stream_gmd_lock(int val);
extern int http_stream_gmd_unlock(int val);

extern void http_stream_gsq_peek( TQID id, int session_id, TSharedObjectId *object, int *is_oos );
extern int http_stream_gsq_is_data_available( TQID id, int session_id, int *no );

#define CURRENT_TIME_STAMP	http_stream_current_time_stamp
#define MAKE_TIME_STRING2	http_stream_make_time_string2
#define LOG_DEBUG_		http_stream_log_Debug
#define LOG_CONSOLE		http_stream_log_Console
#define SMON_SET_S		http_stream_smon_set_s

#define SMD_TAKE		http_stream_smd_take
#define SMD_RETURN		http_stream_smd_return
#define SMD_GET			http_stream_smd_get
#define SMD_PUT			http_stream_smd_put
#define SMD_LOCK		http_stream_smd_lock
#define SMD_UNLOCK		http_stream_smd_unlock

#define GMD_INIT		http_stream_gmd_init
#define GMD_UNINIT		http_stream_gmd_uninit
#define GMD_LOCK		http_stream_gmd_lock
#define GMD_UNLOCK		http_stream_gmd_unlock

#define GSQ_PEEK		http_stream_gsq_peek
#define GSQ_IS_DATA_AVAILABLE	http_stream_gsq_is_data_available

#endif

//#define MAX_NVR_HTTP_STREAM_NUM         (20*4)
//
#if !defined(MAX_NVR_HTTP_STREAM_NUM)
    #if defined(NSS_V2)
        #define MAX_NVR_HTTP_STREAM_NUM		60
    #else
        #define MAX_NVR_HTTP_STREAM_NUM		8
    #endif
#endif

#define HTTP_REQ_POLL_TIMEOUT		300 // msec
#define MAX_EX_HDR_SIZE			64

typedef struct
{
    int32_t sample_rate;
    int16_t bits_per_sample;
    int16_t audio_channels;
}__attribute__((packed)) TAudioInfo;

/* HTTP Streaming Control */
typedef struct clinet_req_ctrl_st
{
    int channel_id;
    int stream_id;
    int qid;
    int sess_idx;
    //httpd_conn* hc;
    // media data reading control
    TSharedObjectId packed_frame_object_id;
    TPackedFrame *cur_packed_frame;
    TSharedObjectId pkt_object_id;
    TTimedData *cur_timed_data;
    int pkt_idx;
    int cur_data_to_read;
    unsigned char *cur_data;
    int cur_header_data_to_read;
    unsigned char *cur_header_data;
    // media data stream out control
    BOOL frame_header_written_ok;
    TIMESTAMP_t last_write_time;
    TIMESTAMP_t session_start_time;
    TIMESTAMP_t start_time;
    TIMESTAMP_t last_stream_ok_time;
    TIMESTAMP_t last_stream_chk_time;
    long long cur_frame_num;
    unsigned char frame_header[sizeof(TQVG_FRM_HDR)+MAX_EX_HDR_SIZE];
    long long cur_glb_seq_no[2]; // 0:video, 1:audio
    //TQVG_FRM_HDR frame_header;
    //BOOL actually_written;
} THttpStreamCtrl;

#ifdef NVR_PTZ
#include "nvr_ptz.h"
#endif

#define HTTP_TREAM_DATA_FIELDS \
    int is_http_stream_req;\
    int connect_tab_idx;\
    THttpStreamCtrl crct;\
    int is_ptz_req;\
    int ptz_channel_id;\
    int ptz_command_id;\
    char ptz_arg[MAX_PTZ_ARG_LEN];\
    int ptz_tab_idx;\
    int mark_terminate;

#if 0
extern void check_and_terminate_timeout_http_streams(httpd_conn* hc, TIMESTAMP_t t);
extern BOOL handle_http_streaming( httpd_conn *hc );
extern void terminate_http_stream( httpd_conn *hc, BOOL do_clear, BOOL due_to_reconfig );
#ifdef NVR_PTZ
int ptz_init(void);
int ptz_create(httpd_conn *hc);
int ptz_destroy(httpd_conn *hc);
int ptz_request(httpd_conn *hc);
int ptz_refresh_channel_info(void);
int ptz_reply_result(httpd_conn *hc);
#endif
#endif

// added by Jeff on 2010/8/10 for diagnosis
#define HTTPD_DIAGNOSIS_FILE    "/tmp/.thttpd_diagnosis"
extern int diagnosis_enabled;

#define LOG_DIAGNOSIS(format, ...) if(diagnosis_enabled)LOG_DEBUG_(format, ## __VA_ARGS__)


#endif

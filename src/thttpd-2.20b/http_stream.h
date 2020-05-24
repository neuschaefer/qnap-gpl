#ifndef __HTTP_STREAM_H_
#define __HTTP_STREAM_H_

#define CNST_WAIT_FOR_CLEAR 5
#define CNST_WAIT_FOR_START_STREAMING 6

int http_stream_smon();
int http_stream_start( int check );
int http_stream_stop( void );
int http_stream_check( void );
int http_stream_live_report( httpd_conn *hc );
void http_stream_init_hc( httpd_conn *hc, int cnum );
int http_stream_handle_send( httpd_conn *hc );
int http_stream_handle_signal( void );
int http_stream_cgi_req( httpd_conn* hc, int *processed );
void http_stream_channel_reconfig( TIMESTAMP_t ts);
void http_stream_handle_ptz(void);
void new_ptz_init(void);

#define HR_LOCK()          pthread_mutex_lock( &http_req_mutex )
#define HR_UNLOCK()        pthread_mutex_unlock( &http_req_mutex )
extern pthread_mutex_t http_req_mutex;
#endif

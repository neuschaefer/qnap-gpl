#ifndef __HTTP_STREAM_CONN_H__
#define __HTTP_STREAM_CONN_H__

int http_stream_conn_handle_read(  connecttab *c, int *fdwatch_recompute );
int http_stream_conn_idle_read_connection(connecttab *c);
int http_stream_conn_idle_send_connection(connecttab *c);
int http_stream_conn_check_timer( void *data, void *func);
void http_stream_conn_start( httpd_conn *hc );
void http_stream_conn_clear( httpd_conn *hc );
void http_stream_conn_terminate_all( int channel_id, BOOL due_to_reconfig );
int http_stream_conn_is_closed(int channel_id);
void http_stream_conn_close_all( void );
BOOL http_stream_conn_media_stream_exist( int qid, int sess_idx );
#ifdef NVR_PTZ
void http_stream_conn_clear_ptz( httpd_conn *hc );
#endif

BOOL http_stream_conn_is_exceed_stream_no(void);
void http_stream_conn_check_and_clear_all(void);
void http_stream_conn_dump( FILE *out_file );


#endif

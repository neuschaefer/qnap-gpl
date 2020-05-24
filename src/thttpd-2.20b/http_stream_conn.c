#if defined(_VIOSTOR_)
#include "log.h"
#include "time_com.h"
#endif

int http_stream_conn_handle_read(  connecttab *c, int *fdwatch_recompute )
{
    // added by Jeff on 2007/9/13
    if( c->hc->is_http_stream_req )
    {
        *fdwatch_recompute = 1;
//LOG_DEBUG_("fdwatch_recompute for http_stream_req");
	HR_LOCK();
	if( c->conn_state!= CNST_SENDING && c->conn_state!= CNST_WAIT_FOR_CLEAR && c->conn_state!= CNST_FREE )
        {
            c->start_stream_time=CURRENT_TIME_STAMP();
            c->conn_state=CNST_WAIT_FOR_START_STREAMING;
        }
        //c->conn_state= CNST_SENDING;
	HR_UNLOCK();
        if( c->idle_read_timer )
        {
            tmr_cancel(c->idle_read_timer);
    	    c->idle_read_timer = (Timer*) 0;
        }
        if( c->idle_send_timer )
        {
            tmr_cancel(c->idle_send_timer);
    	    c->idle_send_timer = (Timer*) 0;
        }
        return 1;
    }
#ifdef NVR_PTZ
    else if( c->hc->is_ptz_req )
    {
        if( c->idle_read_timer )
        {
            tmr_cancel(c->idle_read_timer);
    	    c->idle_read_timer = (Timer*) 0;
        }
        if( c->idle_send_timer )
        {
            tmr_cancel(c->idle_send_timer);
    	    c->idle_send_timer = (Timer*) 0;
        }
        c->conn_state= CNST_SENDING;
#if defined(NVR_DEBUG)
{
        extern void log_set_send( void *hc, int fd, int type );
        log_set_send( c->hc, c->hc->conn_fd, 2 );
}
#endif
        *fdwatch_recompute = 1;
        return 1;
    }
#endif
    return 0;
}

int http_stream_conn_idle_read_connection(connecttab *c)
{
    // added by Jeff on 2007/9/13
    if( c->hc->is_http_stream_req )
    {
        if(c->hc->conn_fd>=0 && c->idle_read_timer )
        {
            tmr_cancel(c->idle_read_timer);
            c->idle_read_timer = (Timer*) 0;
        }
        return 1;
    }
#ifdef NVR_PTZ
    else if( c->hc->is_ptz_req )
    {
        if(c->hc->conn_fd>=0 && c->idle_read_timer)
        {
            tmr_cancel(c->idle_read_timer);
            c->idle_read_timer = (Timer*) 0;
        }
        return 1;
    }
#endif
    return 0;
}

int http_stream_conn_idle_send_connection(connecttab *c)
{
    // added by Jeff on 2007/9/13
    if( c->hc->is_http_stream_req )
    {
        return 1;
    }
#ifdef NVR_PTZ
    else if( c->hc->is_ptz_req )
    {
        return 1;
    }
#endif
    return 0;

}

int http_stream_conn_check_timer( void *data, void *func)
{
    if( func!=(void *)idle_read_connection)
	return 0;
    connecttab* c=(connecttab*) data;
    if( c && c->hc && c->hc->is_http_stream_req )
    {
        if(c->idle_read_timer )
        {
            tmr_cancel(c->idle_read_timer);
            c->idle_read_timer = (Timer*) 0;
        }
	return 1;
    }
    return 0;
}

void http_stream_conn_start( httpd_conn *hc )
{
    connecttab* c;
    c = &connects[hc->connect_tab_idx];
    if( c )
    {
	HR_LOCK();
        c->conn_state=CNST_SENDING;
#if defined(NVR_DEBUG)
{
        extern void log_set_send( void *hc, int fd, int type );
        log_set_send( c->hc, c->hc->conn_fd, 1 );
}
#endif
	HR_UNLOCK();
        fdwatch_recompute = 1;
    }
}

void http_stream_conn_clear( httpd_conn *hc )
{
    connecttab* c;
    c = &connects[hc->connect_tab_idx];
    if( c )
    {
        HR_LOCK();
        c->conn_state=CNST_WAIT_FOR_CLEAR;
        HR_UNLOCK();
    }
#if 0
    struct timeval tv; 
    hc->is_http_stream_req=0;
    (void) gettimeofday( &tv, (struct timezone*) 0 );
    //LOG_CONSOLE("clear_connection for http streaming sock %d", hc->conn_fd);
    do_clear_connection( c, &tv );
#endif
}

// channel_id==-2 don't close_media_session on nvrd since nvrd has restarted and lost all previous session info
void http_stream_conn_terminate_all( int channel_id, BOOL due_to_reconfig )
{
    int i;
    connecttab* c;
    for( i=0; i<maxconnects; i++ )
    {
        c = &connects[i];
        if( c && c->hc && c->hc->is_http_stream_req && (channel_id<0 ||c->hc->crct.channel_id==channel_id))
        {
            if( c->conn_state ==CNST_WAIT_FOR_START_STREAMING )
            {
//LOG_DEBUG_("thttpd: (terminate-all) set mark on fd=%d(hc=%08X", c->hc->conn_fd, c->hc);
                c->hc->mark_terminate=1;
            }
            else
            {
    	    struct timeval tv; 
	    c->hc->is_http_stream_req=0;
            (void) gettimeofday( &tv, (struct timezone*) 0 );
	    if( channel_id!=-2) {
	        terminate_http_stream(c->hc, FALSE, due_to_reconfig);
	    }
            clear_connection( c, &tv );
            }
        }
    }
}

// close all streaming sockets when a child process is forked

void http_stream_conn_close_all( void )
{
    int i;
    connecttab* c;
    for( i=0; i<maxconnects; i++ )
    {
        c = &connects[i];
        if( c && c->hc && c->hc->is_http_stream_req &&  c->hc->conn_fd > 0 && !( c->hc->conn_fd == STDIN_FILENO || c->hc->conn_fd == STDOUT_FILENO || c->hc->conn_fd == STDERR_FILENO ))
        {
            close( c->hc->conn_fd);
            c->hc->conn_fd=-1;
        }
    }
}

int http_stream_conn_is_closed(int channel_id)
{
    int cnum;
    connecttab* c;
    for ( cnum = 0; cnum < maxconnects; ++cnum )
    {
	c = &connects[cnum];
	if(c && c->hc && c->hc->is_http_stream_req )
	{
	    if(channel_id<0 || channel_id==c->hc->crct.channel_id ) {
	        return 0;
	    }
	}
    }
    return 1;
}

BOOL http_stream_conn_media_stream_exist( int qid, int sess_idx )
{
    int i;
    connecttab* c;
    for( i=0; i<maxconnects; i++ )
    {
        c = &connects[i];
        if( c && c->hc && c->hc->is_http_stream_req && c->hc->crct.qid==qid && c->hc->crct.sess_idx==sess_idx )
        {
            return TRUE;
        }
    }
    return FALSE;
}

#define _A(a)	c->hc->crct.a

void http_stream_conn_dump( FILE *out_file )
{
    int cnum;
    connecttab* c;
    int state_num[7];
    int i, idx=0;
    static char st_str[]= { 'F', 'R', 'S', 'P', 'L', 'H', 'W' };
    for( i=0; i<7; i++ )state_num[i]=0;
    LOG_DEBUG_("dump http streaming info...");
    fprintf( out_file, "dump http streaming info...\n");
    for ( cnum = 0; cnum < maxconnects; ++cnum )
    {
	c = &connects[cnum];
	if(c && c->hc && c->hc->is_http_stream_req )
	{
            //if( c->conn_state == CNST_WAIT_FOR_CLEAR)
            char s[64],s2[64];
	    MAKE_TIME_STRING2( s, _A(last_write_time));
	    MAKE_TIME_STRING2( s2, _A(start_time));
	    LOG_DEBUG_( "%2d>%c(sk=%d)ch-%d st-%d q-%2d ss-%d cr=%d,%d fn=%Ld,sq=%Ld,%Ld,lt=%s,a=%s", idx++, st_str[c->conn_state], c->hc->conn_fd, _A(channel_id), _A(stream_id), _A(qid), _A(sess_idx), _A(cur_data_to_read), _A(cur_header_data_to_read), _A(cur_frame_num), _A(cur_glb_seq_no[0]), _A(cur_glb_seq_no[1]), s, httpd_ntoa( &c->hc->client_addr ));
	    fprintf( out_file, "%3d>%c(sk=%4d)ch-%2d st-%d q-%2d ss-%d cr=%d,%d fn=%12Ld lt=%s st=%s\ta=%s\tseq=%Ld,%Ld\n", idx++, st_str[c->conn_state], c->hc->conn_fd, _A(channel_id), _A(stream_id), _A(qid), _A(sess_idx), _A(cur_data_to_read), _A(cur_header_data_to_read), _A(cur_frame_num), s, s2, httpd_ntoa( &c->hc->client_addr ), _A(cur_glb_seq_no[0]), _A(cur_glb_seq_no[1]));
	}
        state_num[c->conn_state]++;
    }
    LOG_DEBUG_("dump http conn info...numconnects=%d", numconnects);
    fprintf( out_file, "dump http conn info...numconnects=%d\n", numconnects);
    for ( cnum = 0; cnum < maxconnects; ++cnum )
    {
	c = &connects[cnum];
        if( c->conn_state != CNST_FREE )
	{
            LOG_DEBUG_( "%4d> %c(sk=%d,hc=%X)", cnum, st_str[c->conn_state], c->hc?c->hc->conn_fd:-1, c->hc );
            fprintf( out_file, "%4d> %c(sk=%4d,hc=%08X)\n", cnum, st_str[c->conn_state], c->hc?c->hc->conn_fd:-1, c->hc );
	}
    }
    char s[64];
    MAKE_TIME_STRING2( s, CURRENT_TIME_STAMP());
    fprintf( out_file, "LAST UPDATE TIM=%s\n", s );
    //LOG_DEBUG_("F=%d R=%d S=%d P=%d L=%d H=%d W=%d", state_num[0], state_num[1], state_num[2], state_num[3], state_num[4], state_num[5], state_num[6] );
    //LOG_DEBUG_("");
    //LOG_DEBUG_("");
}

#ifdef NVR_PTZ
static void check_and_clear_ptz_connections(void)
{
    int cnum;
    connecttab* c;
    for ( cnum = 0; cnum < maxconnects; ++cnum )
    {
        c = &connects[cnum];
        if(c && c->hc && c->hc->is_ptz_req && c->conn_state == CNST_WAIT_FOR_CLEAR)
            {
                struct timeval tv;
                c->hc->is_ptz_req=0;
                (void) gettimeofday( &tv, (struct timezone*) 0 );
                clear_connection( c, &tv );
            }
    }
}
#endif

void http_stream_conn_check_and_clear_all(void)
{
    int cnum;
    static int count=0;
    static TIMESTAMP_t prev_time=0;
    connecttab* c;
    TIMESTAMP_t t;
#ifdef NVR_PTZ
    check_and_clear_ptz_connections();
#endif
    count=(count+1)%10;
    if( count!=0)return;
    t=CURRENT_TIME_STAMP();
    http_stream_channel_reconfig(t);
    for ( cnum = 0; cnum < maxconnects; ++cnum )
    {
	c = &connects[cnum];
#ifdef NVR_PTZ
	if(c && c->hc && (c->hc->is_http_stream_req||c->hc->is_ptz_req) )
#else
	if(c && c->hc && c->hc->is_http_stream_req )
#endif
	{
	    if(c->hc->is_http_stream_req )
            {
                if( c->conn_state ==CNST_WAIT_FOR_START_STREAMING )
                {
                    if( t-c->start_stream_time>30000)
                    {
//LOG_DEBUG_("thttpd: set mark on fd=%d(hc=%08X) due to time-out", c->hc->conn_fd, c->hc);
                        c->hc->mark_terminate=1;
                    }
                }
                else
                {
//LOG_CONSOLE("will check_and_terminate_timeout_http_streams");
                    check_and_terminate_timeout_http_streams(c->hc,t);
//LOG_CONSOLE("end check_and_terminate_timeout_http_streams");
                }
            }
	    else 
            {
		    extern void check_ptz_timeout(httpd_conn *hc, TIMESTAMP_t t);
	            if( c->conn_state == CNST_READING || c->conn_state == CNST_SENDING)
                        check_ptz_timeout(c->hc,t);
            }
            if( c->conn_state == CNST_WAIT_FOR_CLEAR)
	    {
   		struct timeval tv; 
   		c->hc->is_http_stream_req=0;
#ifdef NVR_PTZ
   		c->hc->is_ptz_req=0;
#endif
    		(void) gettimeofday( &tv, (struct timezone*) 0 );
		//LOG_CONSOLE("will clear_connection for http streaming sock %d, hc=%08X", c->hc->conn_fd, c->hc);
    		clear_connection( c, &tv );
		//LOG_CONSOLE("end clear_connection for http streaming sock %d, hc=%08X", c->hc->conn_fd, c->hc);
	    }
	}
    }
    if( prev_time==0 || t-prev_time>10000)
    {
	prev_time=t;
	//http_stream_conn_dump();
    }
}

#ifdef NVR_PTZ
void http_stream_conn_clear_ptz( httpd_conn *hc )
{
    int cnum;
    connecttab* c;
    for ( cnum = 0; cnum < maxconnects; ++cnum )
    {
	//LOG_DEBUG_("clear_ptz_connection: cnum=%d, max=%d", cnum, maxconnects);
	c = &connects[cnum];
	//LOG_DEBUG_("c = %x", c);
	if( c->hc==hc && c->hc->is_ptz_req )
	{
            c->conn_state = CNST_WAIT_FOR_CLEAR;
            return;
#if 0
   	    struct timeval tv; 
   	    c->hc->is_ptz_req=0;
    	    (void) gettimeofday( &tv, (struct timezone*) 0 );
    	    clear_connection( c, &tv );
#endif
	}
    }
}
#endif

BOOL http_stream_conn_is_exceed_stream_no(void)
{
    int i, num=0;
    connecttab* c;
    for( i=0; i<maxconnects; i++ )
    {
        c = &connects[i];
        if( c && c->hc && c->hc->is_http_stream_req && c->hc->conn_fd>=0 )
        {
	    num++;
        }
    }
    return ( num>=MAX_NVR_HTTP_STREAM_NUM);
}

void show_all_connection()
{
    int cnum;
    connecttab* c;
    //LOG_CONSOLE("all sockets:(maxconnects=%d)", maxconnects );
    for ( cnum = 0; cnum < maxconnects; ++cnum )
    {
	c = &connects[cnum];
	if(c && c->hc )
	{
	    //LOG_CONSOLE("socket %d", c->hc->conn_fd );
	}
    }
}


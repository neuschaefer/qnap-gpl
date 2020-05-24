#ifndef DETAIL_H
#define DETAIL_H
/*
 *	detail.h	Routines to handle detail files.
 *
 * Version:	$Id: detail.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
 *
 */

#include <freeradius-devel/ident.h>
RCSIDH(detail_h, "$Id: detail.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $")

typedef enum detail_state_t {
  STATE_UNOPENED = 0,
  STATE_UNLOCKED,
  STATE_HEADER,
  STATE_READING,
  STATE_QUEUED,
  STATE_RUNNING,
  STATE_NO_REPLY,
  STATE_REPLIED
} detail_state_t;

typedef struct listen_detail_t {
	fr_event_t	*ev;	/* has to be first entry (ugh) */
	int		delay_time;
	char		*filename;
	char		*filename_work;
	VALUE_PAIR	*vps;
	FILE		*fp;
	detail_state_t 	state;
	time_t		timestamp;
	time_t		running;
	fr_ipaddr_t	client_ip;
	int		load_factor; /* 1..100 */
	int		signal;
	int		poll_interval;
	int		retry_interval;

	int		has_rtt;
	int		srtt;
	int		rttvar;
	struct timeval  last_packet;
	RADCLIENT	detail_client;
} listen_detail_t;

int detail_recv(rad_listen_t *listener,
		RAD_REQUEST_FUNP *pfun, REQUEST **prequest);
int detail_send(rad_listen_t *listener, REQUEST *request);
void detail_free(rad_listen_t *this);
int detail_print(rad_listen_t *this, char *buffer, size_t bufsize);
int detail_encode(UNUSED rad_listen_t *this, UNUSED REQUEST *request);
int detail_decode(UNUSED rad_listen_t *this, UNUSED REQUEST *request);
int detail_parse(CONF_SECTION *cs, rad_listen_t *this);

#endif /* DETAIL_H */

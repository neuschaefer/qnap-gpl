#ifndef VMPS_H
#define VMPS_H
/*
 *	vmps.h	Routines to handle VMPS sockets.
 *
 * Version:	$Id: vmps.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
 *
 */

#include <freeradius-devel/ident.h>
RCSIDH(vmps_h, "$Id: vmps.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $")

int vqp_socket_recv(rad_listen_t *listener,
		    RAD_REQUEST_FUNP *pfun, REQUEST **prequest);
int vqp_socket_send(rad_listen_t *listener, REQUEST *request);
int vqp_socket_encode(UNUSED rad_listen_t *listener, REQUEST *request);
int vqp_socket_decode(UNUSED rad_listen_t *listener, REQUEST *request);
int vmps_process(REQUEST *request);

#endif /* VMPS_H */

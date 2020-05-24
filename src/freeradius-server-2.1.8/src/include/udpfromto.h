#ifndef UDPFROMTO_H
#define UDPFROMTO_H
/*
 * Version:	$Id: udpfromto.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
 *
 */

#include <freeradius-devel/ident.h>
RCSIDH(udpfromtoh, "$Id: udpfromto.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $")

#include <freeradius-devel/autoconf.h>
#include <freeradius-devel/libradius.h>

#ifdef WITH_UDPFROMTO
int udpfromto_init(int s);
int recvfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t *fromlen,
	       struct sockaddr *to, socklen_t *tolen);
int sendfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t fromlen,
	       struct sockaddr *to, socklen_t tolen);
#endif

#endif

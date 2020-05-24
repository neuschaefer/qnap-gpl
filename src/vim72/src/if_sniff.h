/*
 * if_sniff.h Interface between Vim and SNiFF+
 *
 * $Id: if_sniff.h,v 1.1.1.1 2010/02/01 12:15:01 kent Exp $
 */

#ifndef __if_sniff_h__
#define __if_sniff_h__

extern int  want_sniff_request;
extern int  sniff_request_waiting;
extern int  sniff_connected;
extern int  fd_from_sniff;
extern void sniff_disconnect __ARGS((int immediately));
extern void ProcessSniffRequests __ARGS((void));
extern void ex_sniff __ARGS((exarg_T *eap));

#endif
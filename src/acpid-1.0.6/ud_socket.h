/*
 *$Id: ud_socket.h,v 1.1.1.1 2008/03/26 03:26:13 jimmychang Exp $
 */

#ifndef UD_SOCKET_H__
#define UD_SOCKET_H__

#include <sys/socket.h>
#include <sys/un.h>

int ud_create_socket(const char *name);
int ud_accept(int sock, struct ucred *cred);
int ud_connect(const char *name);

#endif

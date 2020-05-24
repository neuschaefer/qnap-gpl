#ifndef _SMBD_CONNECTION_H_
#define _SMBD_CONNECTION_H_

#ifdef QNAPNAS

#include "vfs.h"
#include "auth/common_auth.h"

void conn_reload(struct smbd_server_connection *sconn);

int write_connlog_common(connection_struct* conn, 
			 const char* fname, 
			 int type, 
			 int action);

int write_connlog_rmdir(connection_struct* conn, 
			const char* directory);

int write_connlog_write(connection_struct* conn, 
			const char* fname);

int write_connlog_unlink(connection_struct* conn, 
			 const char* fname);

int write_connlog_read(connection_struct* conn, 
		       const char* fname);

int write_connlog_mv(connection_struct* conn, 
		     const char* oldname, 
		     const char* newname);

int write_connlog_mkdir(connection_struct* conn, 
			const char* fname);

int write_connlog_add(connection_struct* conn, 
		      const char* fname);

int write_connlog_delonclose(connection_struct* conn, 
			     const char* fname);

int write_connlog_login(const struct auth_usersupplied_info* user_info, 
			int type, 
			int action);

#endif /* QNAPNAS */

#endif /* _SMBD_CONNECTION_H_ */

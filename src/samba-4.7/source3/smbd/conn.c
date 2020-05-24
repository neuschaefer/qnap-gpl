/*
   Unix SMB/CIFS implementation.
   Manage connections_struct structures
   Copyright (C) Andrew Tridgell 1998
   Copyright (C) Alexander Bokovoy 2002
   Copyright (C) Jeremy Allison 2010

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "includes.h"
#include "smbd/smbd.h"
#include "smbd/globals.h"
#include "lib/util/bitmap.h"

#ifdef QNAPNAS
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "connection.h"
#include "tsocket.h"
#endif /* QNAPNAS */

/****************************************************************************
 Return the number of open connections.
****************************************************************************/

int conn_num_open(struct smbd_server_connection *sconn)
{
	return sconn->num_connections;
}

/****************************************************************************
 Check if a snum is in use.
****************************************************************************/

bool conn_snum_used(struct smbd_server_connection *sconn,
		    int snum)
{
	struct connection_struct *conn;

	for (conn=sconn->connections; conn; conn=conn->next) {
		if (conn->params->service == snum) {
			return true;
		}
	}

	return false;
}

/****************************************************************************
 Find first available connection slot, starting from a random position.
 The randomisation stops problems with the server dieing and clients
 thinking the server is still available.
****************************************************************************/

connection_struct *conn_new(struct smbd_server_connection *sconn)
{
	connection_struct *conn;

	if (!(conn=talloc_zero(NULL, connection_struct)) ||
	    !(conn->params = talloc(conn, struct share_params)) ||
	    !(conn->vuid_cache = talloc_zero(conn, struct vuid_cache)) ||
	    !(conn->connectpath = talloc_strdup(conn, "")) ||
	    !(conn->origpath = talloc_strdup(conn, ""))) {
		DEBUG(0,("TALLOC_ZERO() failed!\n"));
		TALLOC_FREE(conn);
		return NULL;
	}
	conn->sconn = sconn;
	conn->force_group_gid = (gid_t)-1;

	DLIST_ADD(sconn->connections, conn);
	sconn->num_connections++;

	return conn;
}

/****************************************************************************
 Clear a vuid out of the connection's vuid cache
****************************************************************************/

#ifdef QNAPNAS
void conn_clear_vuid_cache(connection_struct *conn, uint64_t vuid)
#else /* QNAPNAS */
static void conn_clear_vuid_cache(connection_struct *conn, uint64_t vuid)
#endif /* QNAPNAS */
{
	int i;

	for (i=0; i<VUID_CACHE_SIZE; i++) {
		struct vuid_cache_entry *ent;

		ent = &conn->vuid_cache->array[i];

		if (ent->vuid == vuid) {
			ent->vuid = UID_FIELD_INVALID;
			/*
			 * We need to keep conn->session_info around
			 * if it's equal to ent->session_info as a SMBulogoff
			 * is often followed by a SMBtdis (with an invalid
			 * vuid). The debug code (or regular code in
			 * vfs_full_audit) wants to refer to the
			 * conn->session_info pointer to print debug
			 * statements. Theoretically this is a bug,
			 * as once the vuid is gone the session_info
			 * on the conn struct isn't valid any more,
			 * but there's enough code that assumes
			 * conn->session_info is never null that
			 * it's easier to hold onto the old pointer
			 * until we get a new sessionsetupX.
			 * As everything is hung off the
			 * conn pointer as a talloc context we're not
			 * leaking memory here. See bug #6315. JRA.
			 */
			if (conn->session_info == ent->session_info) {
				ent->session_info = NULL;
			} else {
				TALLOC_FREE(ent->session_info);
			}
			ent->read_only = False;
			ent->share_access = 0;
		}
	}
}

/****************************************************************************
 Clear a vuid out of the validity cache, and as the 'owner' of a connection.

 Called from invalidate_vuid()
****************************************************************************/

void conn_clear_vuid_caches(struct smbd_server_connection *sconn, uint64_t vuid)
{
	connection_struct *conn;

	for (conn=sconn->connections; conn;conn=conn->next) {
		if (conn->vuid == vuid) {
			conn->vuid = UID_FIELD_INVALID;
		}
		conn_clear_vuid_cache(conn, vuid);
	}
}

/****************************************************************************
 Free a conn structure - internal part.
****************************************************************************/

static void conn_free_internal(connection_struct *conn)
{
	vfs_handle_struct *handle = NULL, *thandle = NULL;
	struct trans_state *state = NULL;

	/* Free vfs_connection_struct */
	handle = conn->vfs_handles;
	while(handle) {
		thandle = handle->next;
		DLIST_REMOVE(conn->vfs_handles, handle);
		if (handle->free_data)
			handle->free_data(&handle->data);
		handle = thandle;
	}

	/* Free any pending transactions stored on this conn. */
	for (state = conn->pending_trans; state; state = state->next) {
		/* state->setup is a talloc child of state. */
		SAFE_FREE(state->param);
		SAFE_FREE(state->data);
	}

	free_namearray(conn->veto_list);
	free_namearray(conn->hide_list);
	free_namearray(conn->veto_oplock_list);
	free_namearray(conn->aio_write_behind_list);

	ZERO_STRUCTP(conn);
	talloc_destroy(conn);
}

/****************************************************************************
 Free a conn structure.
****************************************************************************/

void conn_free(connection_struct *conn)
{
	if (conn->sconn == NULL) {
		conn_free_internal(conn);
		return;
	}

	DLIST_REMOVE(conn->sconn->connections, conn);
	SMB_ASSERT(conn->sconn->num_connections > 0);
	conn->sconn->num_connections--;

	conn_free_internal(conn);
}

#ifdef QNAPNAS
int msg_size = sizeof(struct naslog_event_ex)+sizeof(struct naslog_conn);

static int _SendConnToLogEngine(int type, const char *event_user, const char *event_ip,
				const char *event_comp, int serv, int action, 
				const char *res)
{
	int msgid, ret, len;
	struct engine_entry e_entry={};

	msgid = msgget((key_t)1002, 0666 | IPC_CREAT);
	if (msgid < 0) {
		perror("_SendConnToLogEngine - msgget");
	}

	e_entry.command = W_CONNLOG;
	e_entry.a_connlog.type = type;
	len = strlen(event_user);
	strncpy(e_entry.a_connlog.conn_user, event_user, len);
	e_entry.a_connlog.conn_user[len] = 0x0;
	len = strlen(event_ip);
	strncpy(e_entry.a_connlog.conn_ip, event_ip, len);
	e_entry.a_connlog.conn_ip[len] = 0x0;
	len = strlen(event_comp);
	strncpy(e_entry.a_connlog.conn_comp, event_comp, len);
	e_entry.a_connlog.conn_comp[len] = 0x0;
	e_entry.a_connlog.conn_serv = serv;
	e_entry.a_connlog.conn_action = action;
	len = strlen(res);
	strncpy(e_entry.a_connlog.conn_res, res, len);
	e_entry.a_connlog.conn_res[len] = 0x0;

	ret = msgsnd(msgid, (void *)&e_entry, msg_size, IPC_NOWAIT);
	if (ret < 0) {
		perror("_SendConnToLogEngine - msgsnd");
	}

	return ret;
}

static int write_connlog_internal(int type, const char* user, char* ip, 
				  const char* comp, int serv, int action, 
				  const char* rec)
{
	_SendConnToLogEngine(type,
			     (user == NULL) ? "Unknown" : user, 
			     (ip == NULL) ? "0.0.0.0" : ip, 
			     (comp == NULL) ? "Unknown" : comp, 
			     serv, 
			     action, 
			     rec);

	return 0;
}

int write_connlog_common(connection_struct* conn, const char* fname, int type, int action)
{
	char rec[RES_LENGTH];
	char* share_name = NULL;
	const char* machine = get_remote_machine_name();
	const char* ptr;
	char* rhost;

	if (fname == NULL) return -1;

	/* Get remote host address. */
	rhost = tsocket_address_inet_addr_string(conn->sconn->remote_address, 
						 talloc_tos());

	/* find the last slash '/' and discard any character before the slash */
	if ((share_name = strrchr(conn->connectpath, '/')) == NULL) { return -1; }
	share_name++;
	if (!strcmp(fname, ".") || !strcmp(fname, "..")) return -1;

	if (!strncmp(fname, "./", 2))
		ptr=fname+2;
	else
		ptr=fname;
	snprintf(rec, RES_LENGTH, "%s/%s", share_name, ptr);

	write_connlog_internal(type,
			       conn->session_info->unix_info->unix_name,
			       rhost, 
			       machine, 
			       CONN_SERV_SAMBA,
			       action,
			       rec);

	return 0;
}

int write_connlog_rmdir(connection_struct* conn, const char* directory)
{
	char rec[RES_LENGTH];
	char* share_name = NULL;
	const char* machine = get_remote_machine_name();
	char* rhost;

	if (directory == NULL) return -1;

	/* Get remote host address. */
	rhost = tsocket_address_inet_addr_string(conn->sconn->remote_address, 
						 talloc_tos());

	/* find the last slash '/' and discard any character before the slash */
	if ((share_name = strrchr(conn->connectpath, '/')) == NULL) { return -1; }
	share_name++;

	snprintf(rec, RES_LENGTH, "%s/%s", share_name, directory);

	write_connlog_internal(CONN_TYPE_INFO,
			       conn->session_info->unix_info->unix_name,
			       rhost,
			       machine,
			       CONN_SERV_SAMBA,
			       CONN_ACTION_DEL,
			       rec);

	return 0;
}

int write_connlog_write(connection_struct* conn, const char* fname)
{
	return write_connlog_common(conn, fname, CONN_TYPE_INFO, CONN_ACTION_WRITE);
}

int write_connlog_unlink(connection_struct* conn, const char* fname)
{
	return write_connlog_common(conn, fname, CONN_TYPE_INFO, CONN_ACTION_DEL);
}

int write_connlog_read(connection_struct* conn, const char* fname)
{
	return write_connlog_common(conn, fname, CONN_TYPE_INFO, CONN_ACTION_READ);
}

int write_connlog_mv(connection_struct* conn, const char* oldname, const char* newname)
{
	char rec[RES_LENGTH];
	char* share_name = NULL;
	const char* oldfname = NULL; /* old filename */
	const char* newfname = NULL; /* new filename */
	const char* machine = get_remote_machine_name();
	int op_move = 0; /* 0 is rename, 1 is move */
	char* rhost;

	if (oldname == NULL) return -1;
	if (newname == NULL) return -1;

	/* Get remote host address. */
	rhost = tsocket_address_inet_addr_string(conn->sconn->remote_address, 
						 talloc_tos());

	/* find the last slash '/' and discard any character before the slash */
	if ((share_name = strrchr(conn->connectpath, '/')) == NULL) { return -1; }

	/* find the file name */
	if ((oldfname = strrchr(oldname, '/')) == NULL)
		oldfname = oldname;
	else
		oldfname++; /* discard the slash */

	if ((newfname = strrchr(newname, '/')) == NULL)
		newfname = newname;
	else
		newfname++;

	/*
	 * if the filename is the same, log as a move operation; otherwise
	 * log as a rename operation
	 */
	if (strcmp(oldfname, newfname) == 0)
		op_move = 1;

	share_name++;

	/* do not add directory if dir is root, aka "./" */
	if (strncmp(newname, "./", 2) == 0) { 
		newname+=2;
	}

	snprintf(rec, RES_LENGTH, "%s/%s -> %s/%s",
		 share_name, oldname,
		 share_name, newname);

	write_connlog_internal(CONN_TYPE_INFO,
			       conn->session_info->unix_info->unix_name,
			       rhost,
			       machine,
			       CONN_SERV_SAMBA,
			       op_move ? CONN_ACTION_MOVE : CONN_ACTION_RENAME,
			       rec);

	return 0;
}

int write_connlog_mkdir(connection_struct* conn, const char* fname)
{
	return write_connlog_common(conn, fname, CONN_TYPE_INFO, CONN_ACTION_MKDIR);
}

int write_connlog_add(connection_struct* conn, const char* fname)
{
	return write_connlog_common(conn, fname, CONN_TYPE_INFO, CONN_ACTION_ADD);
}

int write_connlog_delonclose(connection_struct* conn, const char* fname)
{
	return write_connlog_common(conn, fname, CONN_TYPE_INFO, CONN_ACTION_DEL);
}

int write_connlog_login(const struct auth_usersupplied_info* user_info, int type, int action)
{
	const char* machine = get_remote_machine_name();
	char* rhost;

	if (user_info == NULL) return -1;

	/* Get remote host address. */
	rhost = tsocket_address_inet_addr_string(user_info->remote_host, 
						 talloc_tos());

	if (rhost == NULL) return -1;

	write_connlog_internal(type,
			       strcmp(user_info->client.account_name, "") ? user_info->client.account_name : "guest",
			       rhost,
			       machine,
			       CONN_SERV_SAMBA,
			       action,
			       "---");

	return 0;
}

#endif /* QNAPNAS */

/* 
   Unix SMB/CIFS implementation.
   connection claim routines
   Copyright (C) Andrew Tridgell 1998

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
#include "dbwrap/dbwrap.h"
#include "auth.h"
#include "../lib/tsocket/tsocket.h"
#include "messages.h"
#include "lib/conn_tdb.h"

#ifdef QNAPNAS
#include "connection.h"
#include "passdb/lookup_sid.h"
#include "passdb.h"
#endif /* QNAPNAS */

struct count_stat {
	int curr_connections;
	const char *name;
	bool verify;
};

/****************************************************************************
 Count the entries belonging to a service in the connection db.
****************************************************************************/

static int count_fn(struct smbXsrv_tcon_global0 *tcon,
		    void *udp)
{
	struct count_stat *cs = (struct count_stat *)udp;

	if (cs->verify && !process_exists(tcon->server_id)) {
		return 0;
	}

	if (strequal(tcon->share_name, cs->name)) {
		cs->curr_connections++;
	}

	return 0;
}

/****************************************************************************
 Claim an entry in the connections database.
****************************************************************************/

int count_current_connections(const char *sharename, bool verify)
{
	struct count_stat cs;
	NTSTATUS status;

	cs.curr_connections = 0;
	cs.name = sharename;
	cs.verify = verify;

	/*
	 * This has a race condition, but locking the chain before hand is worse
	 * as it leads to deadlock.
	 */

	status = smbXsrv_tcon_global_traverse(count_fn, &cs);

	if (!NT_STATUS_IS_OK(status)) {
		DEBUG(0,("count_current_connections: traverse of "
			 "smbXsrv_tcon_global.tdb failed - %s\n",
			 nt_errstr(status)));
		return 0;
	}

	return cs.curr_connections;
}

bool connections_snum_used(struct smbd_server_connection *unused, int snum)
{
	int active;

	active = count_current_connections(lp_servicename(talloc_tos(), snum),
					   true);
	if (active > 0) {
		return true;
	}

	return false;
}

#ifdef QNAPNAS
void conn_reload(struct smbd_server_connection *sconn)
{
	connection_struct *conn = NULL;
	struct user_struct *vuser = NULL;
	char *found_username = NULL;
	struct security_token *token = NULL;
	NTSTATUS status;
	int i = 0;
	int j = 0;
	struct auth_serversupplied_info *server_info = NULL;
	struct samu *sampass = NULL;
	struct passwd *password = NULL;
	bool is_guest = False;
	TALLOC_CTX * mem_ctx = NULL;

	/* Check input. */
	if (sconn == NULL) {
		DEBUG(3, ("conn_reload: sconn is NULL.\n"));
		return;
	}

	for (conn = sconn->connections; conn; conn = conn->next, i++) {
		int snum = SNUM(conn);

		token = NULL;
		if (IS_IPC(conn)) {
			continue;
		}
		vuser = get_valid_user_struct(sconn, conn->vuid);
		if (vuser == NULL) {
			continue;
		}

		DEBUG(3, ("User(%s) connection reload\n", 
			  vuser->session_info->unix_info->unix_name));

		/*
		 * FIXME:
		 * a) Move memory allocate/release functions outside 
		 *    this for-loop scope, given efficiency ?
		 * b) Big and fat function -> several subroutines ?
		 * c) GDB step through or add print log function for 
		 *    debugging.
		 * d) Unit test later.
		 */
		/*
		 * We didn't get a PAC, we have to make up the user
		 * ourselves. Try to ask the pdb backend to provide
		 * SID consistency with ntlmssp session setup.
		 */
		/* 
		 * The stupid make_server_info_XX functions here
		 * don't take a talloc context. 
		 */
		/* FIXME: 
		 * The make_server_info_XX functions here 
		 * will take a talloc context. 
		 */

		sampass = samu_new(talloc_tos());
		if (sampass == NULL) {
			DEBUG(3, ("could not create sampass for %s\n",
				  vuser->session_info->unix_info->unix_name));
			goto conn_reload_exit;
		}

		if (pdb_getsampwnam(sampass, vuser->session_info->unix_info->unix_name)) {
			DEBUG(10, ("found user %s in passdb, calling "
				   "make_server_info_sam\n", 
				   vuser->session_info->unix_info->unix_name));
			status = make_server_info_sam(NULL, sampass, &server_info);
		} else {
			/*
			 * User not in passdb, make it up artificially
			 */
			DEBUG(10, ("didn't find user %s in passdb, calling "
				   "make_server_info_pw\n", 
				   vuser->session_info->unix_info->unix_name));

			/* Get password. */
			password = Get_Pwnam_alloc(talloc_tos(), 
					vuser->session_info->unix_info->unix_name);
			if (password) {
				/* Make server information. */
				status = make_server_info_pw(NULL,
						vuser->session_info->unix_info->unix_name, 
						password,
						&server_info);

				/* Release password. */
				TALLOC_FREE(password);
				password = NULL;
			} else {
				DEBUG(10, ("didn't find user %s in local passwd\n",
				   	  vuser->session_info->unix_info->unix_name));
				status = NT_STATUS_NO_SUCH_USER;
			}
		}
		/* Is guest? */
		if ((NT_STATUS_IS_OK(status)) && (server_info != NULL)) {
			is_guest = server_info->guest;
		} else {
			is_guest = True;
		}

		/* Release. */
		if (sampass) {
			TALLOC_FREE(sampass);
			sampass = NULL;
		}
		if (server_info) {
			TALLOC_FREE(server_info);
			server_info = NULL;
		}

		/* Check return status. */
		if (!NT_STATUS_IS_OK(status)) {
			DEBUG(3, ("User(%s) make_server_info_[sam|pw] failed: %s!\n",
				  vuser->session_info->unix_info->unix_name,
				  nt_errstr(status)));
			goto conn_reload_exit;
		}

		status = create_token_from_username(conn->session_info, 
				vuser->session_info->unix_info->unix_name, 
				is_guest,
				&vuser->session_info->unix_token->uid, 
				&vuser->session_info->unix_token->gid, 
				&found_username,
				&token);
		if (!NT_STATUS_IS_OK(status)) {
			DEBUG(3, ("could not create token for %s\n", 
					vuser->session_info->unix_info->unix_name));
			goto conn_reload_exit;
		}
		if (found_username) {
			TALLOC_FREE(found_username);
			found_username = NULL;
		}
		vuser->session_info->unix_token->ngroups = 0;
		if (vuser->session_info->unix_token->groups) {
			//TALLOC_FREE(vuser->session_info->unix_token->groups);
			vuser->session_info->unix_token->groups = NULL;
		}
		for (j = 1; j < token->num_sids; j++) {
			gid_t gid;
			struct dom_sid *sid = &token->sids[j];
			if (!sid_to_gid(sid, &gid)) {
				continue;
			}
			if (!add_gid_to_array_unique(conn->session_info, 
					gid, 
					&vuser->session_info->unix_token->groups, 
					&vuser->session_info->unix_token->ngroups)) {
				DEBUG(0, ("Failed to add gid %u, NT_STATUS_NO_MEMORY\n",
					  (unsigned int)gid));
				status = NT_STATUS_NO_MEMORY;
				goto conn_reload_exit;
			}
		}
		if (conn->session_info->security_token) {
			TALLOC_FREE(conn->session_info->security_token);
			conn->session_info->security_token = NULL;
		}
		conn->session_info->security_token = token;
		conn_clear_vuid_cache(conn, conn->vuid);
		if (!user_ok_token(conn->session_info->unix_info->unix_name, 
				   conn->session_info->info->domain_name, 
				   conn->session_info->security_token, 
				   snum)) {
			DEBUG(3, ("user_ok_token fail, connection close, vuser = [%s]\n", 
				  vuser->session_info->unix_info->unix_name));
			exit_server_cleanly("Connection close");
		}

	}

conn_reload_exit:
	return;
}
#endif /* QNAPNAS */

/*
 * rlm_eap_tls.h
 *
 * Version:     $Id: rlm_eap_tls.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 * Copyright 2003  Alan DeKok <aland@freeradius.org>
 * Copyright 2006  The FreeRADIUS server project
 */
#ifndef _RLM_EAP_TLS_H
#define _RLM_EAP_TLS_H

#include <freeradius-devel/ident.h>
RCSIDH(rlm_eap_tls_h, "$Id: rlm_eap_tls.h,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $")

#include "eap_tls.h"

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

/* configured values goes right here */
typedef struct eap_tls_conf {
	char		*private_key_password;
	char		*private_key_file;
	char		*certificate_file;
	char		*random_file;
	char		*ca_path;
	char		*ca_file;
	char		*dh_file;
	char		*rsa_file;
	char		*make_cert_command;
	int		rsa_key;
	int		dh_key;
	int		rsa_key_length;
	int		dh_key_length;
	int		verify_depth;
	int		file_type;
	int		include_length;

	/*
	 *	Always < 4096 (due to radius limit), 0 by default = 2048
	 */
	int		fragment_size;
	int		check_crl;
	char		*check_cert_cn;
	char		*cipher_list;
	char		*check_cert_issuer;

        int     	session_cache_enable;
        int     	session_timeout;
        int     	session_cache_size;
	char		*session_id_name;
	char		session_context_id[128];
	time_t		session_last_flushed;
} EAP_TLS_CONF;

/* This structure gets stored in arg */
typedef struct _eap_tls_t {
	EAP_TLS_CONF 	*conf;
	SSL_CTX		*ctx;
} eap_tls_t;


#endif /* _RLM_EAP_TLS_H */

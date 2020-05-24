/*
 * rlm_detail.c	accounting:    Write the "detail" files.
 *
 * Version:	$Id: rlm_detail.c,v 1.5 2010/03/30 10:29:59 richardchang Exp $
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
 * Copyright 2000,2006  The FreeRADIUS server project
 */

#include	<freeradius-devel/ident.h>
RCSID("$Id: rlm_detail.c,v 1.5 2010/03/30 10:29:59 richardchang Exp $")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>
#include	<freeradius-devel/rad_assert.h>
#include	<freeradius-devel/detail.h>

#include	<sys/stat.h>
#include	<ctype.h>
#include	<fcntl.h>

#define 	DIRLEN	8192

#ifdef QNAP
/* for system logs RichardChang@QNAP */
#include <qLogEngine.h>
#endif

struct detail_instance {
	/* detail file */
	char *detailfile;

	/* detail file permissions */
	int detailperm;

	/* directory permissions */
	int dirperm;

	/* last made directory */
	char *last_made_directory;

	/* timestamp & stuff */
	char *header;

	/* if we want file locking */
	int locking;

	/* log src/dst information */
	int log_srcdst;

	fr_hash_table_t *ht;
};

static const CONF_PARSER module_config[] = {
	{ "detailfile",    PW_TYPE_STRING_PTR,
	  offsetof(struct detail_instance,detailfile), NULL, "%A/%{Client-IP-Address}/detail" },
	{ "header",    PW_TYPE_STRING_PTR,
	  offsetof(struct detail_instance,header), NULL, "%t" },
	{ "detailperm",    PW_TYPE_INTEGER,
	  offsetof(struct detail_instance,detailperm), NULL, "0600" },
	{ "dirperm",       PW_TYPE_INTEGER,
	  offsetof(struct detail_instance,dirperm),    NULL, "0755" },
	{ "locking",       PW_TYPE_BOOLEAN,
	  offsetof(struct detail_instance,locking),    NULL, "no" },
	{ "log_packet_header",       PW_TYPE_BOOLEAN,
	  offsetof(struct detail_instance,log_srcdst),    NULL, "no" },
	{ NULL, -1, 0, NULL, NULL }
};


/*
 *	Clean up.
 */
static int detail_detach(void *instance)
{
        struct detail_instance *inst = instance;
	free((char*) inst->last_made_directory);
	if (inst->ht) fr_hash_table_free(inst->ht);

        free(inst);
	return 0;
}


static uint32_t detail_hash(const void *data)
{
	const DICT_ATTR *da = data;
	return fr_hash(&(da->attr), sizeof(da->attr));
}

static int detail_cmp(const void *a, const void *b)
{
	return ((const DICT_ATTR *)a)->attr - ((const DICT_ATTR *)b)->attr;
}


/*
 *	(Re-)read radiusd.conf into memory.
 */
static int detail_instantiate(CONF_SECTION *conf, void **instance)
{
	struct detail_instance *inst;
	CONF_SECTION	*cs;

	inst = rad_malloc(sizeof(*inst));
	if (!inst) {
		return -1;
	}
	memset(inst, 0, sizeof(*inst));

	if (cf_section_parse(conf, inst, module_config) < 0) {
		detail_detach(inst);
		return -1;
	}

	inst->last_made_directory = NULL;

	/*
	 *	Suppress certain attributes.
	 */
	cs = cf_section_sub_find(conf, "suppress");
	if (cs) {
		CONF_ITEM	*ci;

		inst->ht = fr_hash_table_create(detail_hash, detail_cmp,
						  NULL);

		for (ci = cf_item_find_next(cs, NULL);
		     ci != NULL;
		     ci = cf_item_find_next(cs, ci)) {
			const char	*attr;
			DICT_ATTR	*da;

			if (!cf_item_is_pair(ci)) continue;

			attr = cf_pair_attr(cf_itemtopair(ci));
			if (!attr) continue; /* pair-anoia */

			da = dict_attrbyname(attr);
			if (!da) {
				radlog(L_INFO, "rlm_detail: WARNING: No such attribute %s: Cannot suppress printing it.", attr);
				continue;
			}

			/*
			 *	For better distribution we should really
			 *	hash the attribute number or name.  But
			 *	since the suppression list will usually
			 *	be small, it doesn't matter.
			 */
			if (!fr_hash_table_insert(inst->ht, da)) {
				radlog(L_ERR, "rlm_detail: Failed trying to remember %s", attr);
				detail_detach(inst);
				return -1;
			}
		}
	}


	*instance = inst;
	return 0;
}

/*
 *	Do detail, compatible with old accounting
 */
static int do_detail(void *instance, REQUEST *request, RADIUS_PACKET *packet,
		     int compat)
{
	int		outfd;
	FILE		*outfp;
	char		timestamp[256];
	char		buffer[DIRLEN];
	char		*p;
	struct stat	st;
	int		locked;
	int		lock_count;
	struct timeval	tv;
	VALUE_PAIR	*pair;
	struct detail_instance *inst = instance;
#ifdef QNAP
	/* for system logs KenChen@QNAP */
	int myattr=-1; 
	char myuser[256];
	char myip[64], *comp = "---";
	char logstr[1024]; 
	VALUE_PAIR	*mypair; 
	int nas_address = 0;
	/***/
#endif
	rad_assert(request != NULL);

	/*
	 *	Nothing to log: don't do anything.
	 */
	if (!packet) {
		return RLM_MODULE_NOOP;
	}

	/*
	 *	Create a directory for this nas.
	 *
	 *	Generate the path for the detail file.  Use the
	 *	same format, but truncate at the last /.  Then
	 *	feed it through radius_xlat() to expand the
	 *	variables.
	 */
	radius_xlat(buffer, sizeof(buffer), inst->detailfile, request, NULL);
	RDEBUG2("%s expands to %s", inst->detailfile, buffer);
	
	/*
	 *	Grab the last directory delimiter.
	 */
	p = strrchr(buffer,'/');

	/*
	 *	There WAS a directory delimiter there, and
	 *	the file doesn't exist, so
	 *	we prolly must create it the dir(s)
	 */
	if ((p) && (stat(buffer, &st) < 0)) {
		*p = '\0';
		/*
		 *	NO previously cached directory name, so we've
		 *	got to create a new one.
		 *
		 *	OR the new directory name is different than the old,
		 *	so we've got to create a new one.
		 *
		 *	OR the cached directory has somehow gotten removed,
		 *	so we've got to create a new one.
		 */
		if ((inst->last_made_directory == NULL) ||
		    (strcmp(inst->last_made_directory, buffer) != 0)) {
			free((char *) inst->last_made_directory);
			inst->last_made_directory = strdup(buffer);
		}

		/*
		 *	stat the directory, and don't do anything if
		 *	it exists.  If it doesn't exist, create it.
		 *
		 *	This also catches the case where some idiot
		 *	deleted a directory that the server was using.
		 */
		if (rad_mkdir(inst->last_made_directory, inst->dirperm) < 0) {
			radlog_request(L_ERR, 0, request, "rlm_detail: Failed to create directory %s: %s", inst->last_made_directory, strerror(errno));
			return RLM_MODULE_FAIL;
		}

		*p = '/';
	} /* else there was no directory delimiter. */

	locked = 0;
	lock_count = 0;
	do {
		/*
		 *	Open & create the file, with the given
		 *	permissions.
		 */
		if ((outfd = open(buffer, O_WRONLY | O_APPEND | O_CREAT,
				  inst->detailperm)) < 0) {
			radlog_request(L_ERR, 0, request, "rlm_detail: Couldn't open file %s: %s",
			       buffer, strerror(errno));
			return RLM_MODULE_FAIL;
		}

		/*
		 *	If we fail to aquire the filelock in 80 tries
		 *	(approximately two seconds) we bail out.
		 */
		if (inst->locking) {
			lseek(outfd, 0L, SEEK_SET);
			if (rad_lockfd_nonblock(outfd, 0) < 0) {
				close(outfd);
				tv.tv_sec = 0;
				tv.tv_usec = 25000;
				select(0, NULL, NULL, NULL, &tv);
				lock_count++;
				continue;
			}

			/*
			 *	The file might have been deleted by
			 *	radrelay while we tried to acquire
			 *	the lock (race condition)
			 */
			if (fstat(outfd, &st) != 0) {
				radlog_request(L_ERR, 0, request, "rlm_detail: Couldn't stat file %s: %s",
				       buffer, strerror(errno));
				close(outfd);
				return RLM_MODULE_FAIL;
			}
			if (st.st_nlink == 0) {
				RDEBUG2("File %s removed by another program, retrying",
				      buffer);
				close(outfd);
				lock_count = 0;
				continue;
			}

			RDEBUG2("Acquired filelock, tried %d time(s)",
			      lock_count + 1);
			locked = 1;
		}
	} while (inst->locking && !locked && lock_count < 80);

	if (inst->locking && !locked) {
		close(outfd);
		radlog_request(L_ERR, 0, request, "rlm_detail: Failed to acquire filelock for %s, giving up",
		       buffer);
		return RLM_MODULE_FAIL;
	}

	/*
	 *	Convert the FD to FP.  The FD is no longer valid
	 *	after this operation.
	 */
	if ((outfp = fdopen(outfd, "a")) == NULL) {
		radlog_request(L_ERR, 0, request, "rlm_detail: Couldn't open file %s: %s",
		       buffer, strerror(errno));
		if (inst->locking) {
			lseek(outfd, 0L, SEEK_SET);
			rad_unlockfd(outfd, 0);
			RDEBUG2("Released filelock");
		}
		close(outfd);	/* automatically releases the lock */

		return RLM_MODULE_FAIL;
	}

	/*
	 *	Post a timestamp
	 */
	fseek(outfp, 0L, SEEK_END);
	radius_xlat(timestamp, sizeof(timestamp), inst->header, request, NULL);
	fprintf(outfp, "%s\n", timestamp);

	/*
	 *	Write the information to the file.
	 */
	if (!compat) {
		/*
		 *	Print out names, if they're OK.
		 *	Numbers, if not.
		 */
		if ((packet->code > 0) &&
		    (packet->code < FR_MAX_PACKET_CODE)) {
			fprintf(outfp, "\tPacket-Type = %s\n",
				fr_packet_codes[packet->code]);
		} else {
			fprintf(outfp, "\tPacket-Type = %d\n", packet->code);
		}
	}

	if (inst->log_srcdst) {
		VALUE_PAIR src_vp, dst_vp;

		memset(&src_vp, 0, sizeof(src_vp));
		memset(&dst_vp, 0, sizeof(dst_vp));
		src_vp.operator = dst_vp.operator = T_OP_EQ;

		switch (packet->src_ipaddr.af) {
		case AF_INET:
			src_vp.type = PW_TYPE_IPADDR;
			src_vp.attribute = PW_PACKET_SRC_IP_ADDRESS;
			src_vp.vp_ipaddr = packet->src_ipaddr.ipaddr.ip4addr.s_addr;
			dst_vp.type = PW_TYPE_IPADDR;
			dst_vp.attribute = PW_PACKET_DST_IP_ADDRESS;
			dst_vp.vp_ipaddr = packet->dst_ipaddr.ipaddr.ip4addr.s_addr;
			break;
		case AF_INET6:
			src_vp.type = PW_TYPE_IPV6ADDR;
			src_vp.attribute = PW_PACKET_SRC_IPV6_ADDRESS;
			memcpy(src_vp.vp_strvalue,
			       &packet->src_ipaddr.ipaddr.ip6addr,
			       sizeof(packet->src_ipaddr.ipaddr.ip6addr));
			dst_vp.type = PW_TYPE_IPV6ADDR;
			dst_vp.attribute = PW_PACKET_DST_IPV6_ADDRESS;
			memcpy(dst_vp.vp_strvalue,
			       &packet->dst_ipaddr.ipaddr.ip6addr,
			       sizeof(packet->dst_ipaddr.ipaddr.ip6addr));
			break;
		default:
			break;
		}

		fputs("\t", outfp);
		vp_print(outfp, &src_vp);
		fputs("\n", outfp);
		fputs("\t", outfp);
		vp_print(outfp, &dst_vp);
		fputs("\n", outfp);

		src_vp.attribute = PW_PACKET_SRC_PORT;
		src_vp.type = PW_TYPE_INTEGER;
		src_vp.vp_integer = packet->src_port;
		dst_vp.attribute = PW_PACKET_DST_PORT;
		dst_vp.type = PW_TYPE_INTEGER;
		dst_vp.vp_integer = packet->dst_port;

		fputs("\t", outfp);
		vp_print(outfp, &src_vp);
		fputs("\n", outfp);
		fputs("\t", outfp);
		vp_print(outfp, &dst_vp);
		fputs("\n", outfp);
	}
#ifdef QNAP
	memset(myuser, '\0', sizeof(myuser)); /* KenChen@QNAP */
	strcpy(myip, "---");
	myattr=-1;
	nas_address = 0;
#endif
	/* Write each attribute/value to the log file */
	for (pair = packet->vps; pair != NULL; pair = pair->next) {
		DICT_ATTR da;
		da.attr = pair->attribute;

		if (inst->ht &&
		    fr_hash_table_finddata(inst->ht, &da)) continue;

		/*
		 *	Don't print passwords in old format...
		 */
		if (compat && (pair->attribute == PW_USER_PASSWORD)) continue;
#ifdef QNAP
		/* KenChen@QNAP */
		if (pair->attribute == PW_ACCT_STATUS_TYPE){
			switch (pair->vp_integer) {
				case PW_STATUS_START:
				case PW_STATUS_STOP:
				case PW_STATUS_ACCOUNTING_ON:
				case PW_STATUS_ACCOUNTING_OFF:
					myattr = pair->vp_integer;
				break;
				default:
				break;
			}
		}
		if (pair->attribute == PW_USER_NAME && pair->vp_strvalue && strlen(pair->vp_strvalue)>0) {
			strncpy(myuser, pair->vp_strvalue, sizeof(myuser));
		}
		/***/
#endif
		/*
		 *	Print all of the attributes.
		 */
		fputs("\t", outfp);
		vp_print(outfp, pair);
		fputs("\n", outfp);
	}
#ifdef QNAP
	/* KenChen@QNAP */
	if (request->packet->src_ipaddr.ipaddr.ip4addr.s_addr) {
		nas_address = request->packet->src_ipaddr.ipaddr.ip4addr.s_addr;
		ip_ntoa(myip, nas_address);
	}
	if (myattr >= 0){
		if ((myattr == PW_STATUS_START) && myuser){
			SendConnToLogEngine(EVENT_TYPE_INFO, myuser, myip, comp, CONN_SERV_RADIUS, CONN_ACTION_LOGIN_SUCC, comp);
		}
		else if ((myattr == PW_STATUS_STOP) && myuser){
			SendConnToLogEngine(EVENT_TYPE_INFO, myuser, myip, comp, CONN_SERV_RADIUS, CONN_ACTION_LOGOUT, comp);
		}
		else if ((myattr == PW_STATUS_ACCOUNTING_ON) && myuser){
			SendConnToLogEngine(EVENT_TYPE_INFO, myuser, myip, comp, CONN_SERV_RADIUS, CONN_ACTION_LOGIN_SUCC, comp);
		}
		else if ((myattr == PW_STATUS_ACCOUNTING_OFF) && myuser){
			SendConnToLogEngine(EVENT_TYPE_INFO, myuser, myip, comp, CONN_SERV_RADIUS, CONN_ACTION_LOGOUT, comp);
		}
	}
	/***/
#endif
	/*
	 *	Add non-protocol attibutes.
	 */
	if (compat) {
		if (request->proxy) {
			char proxy_buffer[128];

			inet_ntop(request->proxy->dst_ipaddr.af,
				  &request->proxy->dst_ipaddr.ipaddr,
				  proxy_buffer, sizeof(proxy_buffer));
			fprintf(outfp, "\tFreeradius-Proxied-To = %s\n",
					proxy_buffer);
				RDEBUG("Freeradius-Proxied-To = %s",
				      proxy_buffer);
		}

		fprintf(outfp, "\tTimestamp = %ld\n",
			(unsigned long) request->timestamp);

		/*
		 *	We no longer permit Accounting-Request packets
		 *	with an authenticator of zero.
		 */
		fputs("\tRequest-Authenticator = Verified\n", outfp);
	}

	fputs("\n", outfp);

	if (inst->locking) {
		fflush(outfp);
		lseek(outfd, 0L, SEEK_SET);
		rad_unlockfd(outfd, 0);
		RDEBUG2("Released filelock");
	}

	fclose(outfp);

	/*
	 *	And everything is fine.
	 */
	return RLM_MODULE_OK;
}

/*
 *	Accounting - write the detail files.
 */
static int detail_accounting(void *instance, REQUEST *request)
{
	if (request->listener->type == RAD_LISTEN_DETAIL &&
	    strcmp(((struct detail_instance *)instance)->detailfile,
	           ((listen_detail_t *)request->listener->data)->filename) == 0) {
		RDEBUG("Suppressing writes to detail file as the request was just read from a detail file.");
		return RLM_MODULE_NOOP;
	}

	return do_detail(instance,request,request->packet, TRUE);
}

/*
 *	Incoming Access Request - write the detail files.
 */
static int detail_authorize(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->packet, FALSE);
}

/*
 *	Outgoing Access-Request Reply - write the detail files.
 */
static int detail_postauth(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->reply, FALSE);
}

#ifdef WITH_COA
/*
 *	Incoming CoA - write the detail files.
 */
static int detail_recv_coa(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->packet, FALSE);
}

/*
 *	Outgoing CoA - write the detail files.
 */
static int detail_send_coa(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->reply, FALSE);
}
#endif

/*
 *	Outgoing Access-Request to home server - write the detail files.
 */
static int detail_pre_proxy(void *instance, REQUEST *request)
{
	if (request->proxy &&
	    request->proxy->vps) {
		return do_detail(instance,request,request->proxy, FALSE);
	}

	return RLM_MODULE_NOOP;
}


/*
 *	Outgoing Access-Request Reply - write the detail files.
 */
static int detail_post_proxy(void *instance, REQUEST *request)
{
	if (request->proxy_reply &&
	    request->proxy_reply->vps) {
		return do_detail(instance,request,request->proxy_reply, FALSE);
	}

	/*
	 *	No reply: we must be doing Post-Proxy-Type = Fail.
	 *
	 *	Note that we just call the normal accounting function,
	 *	to minimize the amount of code, and to highlight that
	 *	it's doing normal accounting.
	 */
	if (!request->proxy_reply) {
		int rcode;

		rcode = detail_accounting(instance, request);
		if (rcode == RLM_MODULE_OK) {
			request->reply->code = PW_ACCOUNTING_RESPONSE;
		}
		return rcode;
	}

	return RLM_MODULE_NOOP;
}


/* globally exported name */
module_t rlm_detail = {
	RLM_MODULE_INIT,
	"detail",
	RLM_TYPE_THREAD_UNSAFE | RLM_TYPE_CHECK_CONFIG_SAFE | RLM_TYPE_HUP_SAFE,
	detail_instantiate,		/* instantiation */
	detail_detach,			/* detach */
	{
		NULL,			/* authentication */
		detail_authorize, 	/* authorization */
		NULL,			/* preaccounting */
		detail_accounting,	/* accounting */
		NULL,			/* checksimul */
		detail_pre_proxy,      	/* pre-proxy */
		detail_post_proxy,	/* post-proxy */
		detail_postauth		/* post-auth */
#ifdef WITH_COA
		, detail_recv_coa,
		detail_send_coa
#endif
	},
};


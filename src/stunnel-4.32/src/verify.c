/*
 *   stunnel       Universal SSL tunnel
 *   Copyright (C) 1998-2010 Michal Trojnara <Michal.Trojnara@mirt.net>
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the
 *   Free Software Foundation; either version 2 of the License, or (at your
 *   option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, see <http://www.gnu.org/licenses>.
 * 
 *   Linking stunnel statically or dynamically with other modules is making
 *   a combined work based on stunnel. Thus, the terms and conditions of
 *   the GNU General Public License cover the whole combination.
 * 
 *   In addition, as a special exception, the copyright holder of stunnel
 *   gives you permission to combine stunnel with free software programs or
 *   libraries that are released under the GNU LGPL and with code included
 *   in the standard release of OpenSSL under the OpenSSL License (or
 *   modified versions of such code, with unchanged license). You may copy
 *   and distribute such a system following the terms of the GNU GPL for
 *   stunnel and the licenses of the other code concerned.
 * 
 *   Note that people who make modified versions of stunnel are not obligated
 *   to grant this special exception for their modified versions; it is their
 *   choice whether to do so. The GNU General Public License gives permission
 *   to release a modified version without this exception; this exception
 *   also makes it possible to release a modified version which carries
 *   forward this exception.
 */

#include "common.h"
#include "prototypes.h"

/**************************************** prototypes */

/* verify initialization */
static int load_file_lookup(X509_STORE *, char *);
static int add_dir_lookup(X509_STORE *, char *);

/* verify callback */
static int verify_callback(int, X509_STORE_CTX *);
static int cert_check(CLI *c, X509_STORE_CTX *, char *, int);
static int crl_check(CLI *c, X509_STORE_CTX *, char *);
static int ocsp_check(CLI *c, X509_STORE_CTX *, char *);

/* utility functions */
static void log_time(const int, const char *, ASN1_TIME *);

/**************************************** verify initialization */

int verify_init(SERVICE_OPTIONS *section) {
    if(section->verify_level<0)
        return 1; /* no certificate verification */

    if(section->verify_level>1 && !section->ca_file && !section->ca_dir) {
        s_log(LOG_ERR,
            "Either CApath or CAfile has to be used for authentication");
        return 0;
    }

    section->revocation_store=X509_STORE_new();
    if(!section->revocation_store) {
        sslerror("X509_STORE_new");
        return 0;
    }

    if(section->ca_file) {
        if(!SSL_CTX_load_verify_locations(section->ctx,
                section->ca_file, NULL)) {
            s_log(LOG_ERR, "Error loading verify certificates from %s",
                section->ca_file);
            sslerror("SSL_CTX_load_verify_locations");
            return 0;
        }
        /* list of trusted CAs for the client to choose the right cert */
        SSL_CTX_set_client_CA_list(section->ctx,
            SSL_load_client_CA_file(section->ca_file));
        s_log(LOG_DEBUG, "Loaded verify certificates from %s",
            section->ca_file);
        if(!load_file_lookup(section->revocation_store, section->ca_file))
            return 0;
    }

    if(section->ca_dir) {
        if(!SSL_CTX_load_verify_locations(section->ctx,
                NULL, section->ca_dir)) {
            s_log(LOG_ERR, "Error setting verify directory to %s",
                section->ca_dir);
            sslerror("SSL_CTX_load_verify_locations");
            return 0;
        }
        s_log(LOG_DEBUG, "Verify directory set to %s", section->ca_dir);
        add_dir_lookup(section->revocation_store, section->ca_dir);
    }

    if(section->crl_file)
        if(!load_file_lookup(section->revocation_store, section->crl_file))
            return 0;

    if(section->crl_dir) {
        section->revocation_store->cache=0; /* don't cache CRLs */
        add_dir_lookup(section->revocation_store, section->crl_dir);
    }

    SSL_CTX_set_verify(section->ctx, section->verify_level==SSL_VERIFY_NONE ?
        SSL_VERIFY_PEER : section->verify_level, verify_callback);

    if(section->ca_dir && section->verify_use_only_my)
        s_log(LOG_NOTICE, "Peer certificate location %s", section->ca_dir);
    return 1; /* OK */
}

static int load_file_lookup(X509_STORE *store, char *name) {
    X509_LOOKUP *lookup;

    lookup=X509_STORE_add_lookup(store, X509_LOOKUP_file());
    if(!lookup) {
        sslerror("X509_STORE_add_lookup");
        return 0;
    }
    if(!X509_LOOKUP_load_file(lookup, name, X509_FILETYPE_PEM)) {
        s_log(LOG_ERR, "Failed to load %s revocation lookup file", name);
        sslerror("X509_LOOKUP_load_file");
        return 0;
    }
    s_log(LOG_DEBUG, "Loaded %s revocation lookup file", name);
    return 1; /* OK */
}

static int add_dir_lookup(X509_STORE *store, char *name) {
    X509_LOOKUP *lookup;

    lookup=X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
    if(!lookup) {
        sslerror("X509_STORE_add_lookup");
        return 0;
    }
    if(!X509_LOOKUP_add_dir(lookup, name, X509_FILETYPE_PEM)) {
        s_log(LOG_ERR, "Failed to add %s revocation lookup directory", name);
        sslerror("X509_LOOKUP_add_dir");
        return 0;
    }
    s_log(LOG_DEBUG, "Added %s revocation lookup directory", name);
    return 1; /* OK */
}

/**************************************** verify callback */

static int verify_callback(int preverify_ok, X509_STORE_CTX *callback_ctx) {
        /* our verify callback function */
    SSL *ssl;
    CLI *c;
    char subject_name[STRLEN];

    X509_NAME_oneline(X509_get_subject_name(callback_ctx->current_cert),
        subject_name, STRLEN);
    safestring(subject_name);

    /* retrieve the pointer to the SSL of the connection currently treated
     * and the application specific data stored into the SSL object */
    ssl=X509_STORE_CTX_get_ex_data(callback_ctx,
        SSL_get_ex_data_X509_STORE_CTX_idx());
    c=SSL_get_ex_data(ssl, cli_index);

    if(!cert_check(c, callback_ctx, subject_name, preverify_ok))
        return 0; /* reject connection */
    if(!crl_check(c, callback_ctx, subject_name))
        return 0; /* reject connection */
#if SSLEAY_VERSION_NUMBER >= 0x00907000L
    if(c->opt->option.ocsp && !ocsp_check(c, callback_ctx, subject_name))
        return 0; /* reject connection */
#endif /* OpenSSL-0.9.7 */

    /* errnum=X509_STORE_CTX_get_error(ctx); */
    s_log(LOG_NOTICE, "VERIFY OK: depth=%d, %s",
        callback_ctx->error_depth, subject_name);
    return 1; /* accept connection */
}

/**************************************** certificate checking */

static int cert_check(CLI *c, X509_STORE_CTX *callback_ctx,
        char *subject_name, int preverify_ok) {
    X509_OBJECT ret;

    if(c->opt->verify_level==SSL_VERIFY_NONE) {
        s_log(LOG_INFO, "VERIFY IGNORE: depth=%d, %s",
            callback_ctx->error_depth, subject_name);
        return 1; /* accept connection */
    }
    if(!preverify_ok) {
        /* remote site specified a certificate, but it's not correct */
        s_log(LOG_WARNING, "VERIFY ERROR: depth=%d, error=%s: %s",
            callback_ctx->error_depth,
            X509_verify_cert_error_string (callback_ctx->error),
                subject_name);
        return 0; /* reject connection */
    }
    if(c->opt->verify_use_only_my && callback_ctx->error_depth==0 &&
            X509_STORE_get_by_subject(callback_ctx, X509_LU_X509,
                X509_get_subject_name(callback_ctx->current_cert), &ret)!=1) {
        s_log(LOG_WARNING, "VERIFY ERROR ONLY MY: no cert for %s",
            subject_name);
        return 0; /* reject connection */
    }
    return 1; /* accept connection */
}

/**************************************** CRL checking */

/* based on BSD-style licensed code of mod_ssl */
static int crl_check(CLI *c, X509_STORE_CTX *callback_ctx,
        char *subject_name) {
    X509_STORE_CTX store_ctx;
    X509_OBJECT obj;
    X509_NAME *subject;
    X509_NAME *issuer;
    X509 *cert;
    X509_CRL *crl;
    X509_REVOKED *revoked;
    EVP_PKEY *pubkey;
    long serial;
    int i, n, rc;
    char *cp;
    ASN1_TIME *last_update=NULL, *next_update=NULL;

    /* determine certificate ingredients in advance */
    cert=X509_STORE_CTX_get_current_cert(callback_ctx);
    subject=X509_get_subject_name(cert);
    issuer=X509_get_issuer_name(cert);

    /* try to retrieve a CRL corresponding to the _subject_ of
     * the current certificate in order to verify it's integrity */
    memset((char *)&obj, 0, sizeof obj);
    X509_STORE_CTX_init(&store_ctx, c->opt->revocation_store, NULL, NULL);
    rc=X509_STORE_get_by_subject(&store_ctx, X509_LU_CRL, subject, &obj);
    X509_STORE_CTX_cleanup(&store_ctx);
    crl=obj.data.crl;
    if(rc>0 && crl) {
        cp=X509_NAME_oneline(subject, NULL, 0);
        s_log(LOG_INFO, "CRL: issuer: %s", cp);
        OPENSSL_free(cp);
        last_update=X509_CRL_get_lastUpdate(crl);
        next_update=X509_CRL_get_nextUpdate(crl);
        log_time(LOG_INFO, "CRL: last update", last_update);
        log_time(LOG_INFO, "CRL: next update", next_update);

        /* verify the signature on this CRL */
        pubkey=X509_get_pubkey(cert);
        if(X509_CRL_verify(crl, pubkey)<=0) {
            s_log(LOG_WARNING, "Invalid signature on CRL");
            X509_STORE_CTX_set_error(callback_ctx,
                X509_V_ERR_CRL_SIGNATURE_FAILURE);
            X509_OBJECT_free_contents(&obj);
            if(pubkey)
                EVP_PKEY_free(pubkey);
            return 0; /* reject connection */
        }
        if(pubkey)
            EVP_PKEY_free(pubkey);

        /* check date of CRL to make sure it's not expired */
        if(!next_update) {
            s_log(LOG_WARNING, "Found CRL has invalid nextUpdate field");
            X509_STORE_CTX_set_error(callback_ctx,
                X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD);
            X509_OBJECT_free_contents(&obj);
            return 0; /* reject connection */
        }
        if(X509_cmp_current_time(next_update)<0) {
            s_log(LOG_WARNING, "Found CRL is expired - "
                "revoking all certificates until you get updated CRL");
            X509_STORE_CTX_set_error(callback_ctx, X509_V_ERR_CRL_HAS_EXPIRED);
            X509_OBJECT_free_contents(&obj);
            return 0; /* reject connection */
        }
        X509_OBJECT_free_contents(&obj);
    }

    /* try to retrieve a CRL corresponding to the _issuer_ of
     * the current certificate in order to check for revocation */
    memset((char *)&obj, 0, sizeof obj);
    X509_STORE_CTX_init(&store_ctx, c->opt->revocation_store, NULL, NULL);
    rc=X509_STORE_get_by_subject(&store_ctx, X509_LU_CRL, issuer, &obj);
    X509_STORE_CTX_cleanup(&store_ctx);
    crl=obj.data.crl;
    if(rc>0 && crl) {
        /* check if the current certificate is revoked by this CRL */
#if SSLEAY_VERSION_NUMBER >= 0x00904000
        n=sk_X509_REVOKED_num(X509_CRL_get_REVOKED(crl));
#else
        n=sk_num(X509_CRL_get_REVOKED(crl));
#endif
        for(i=0; i<n; i++) {
#if SSLEAY_VERSION_NUMBER >= 0x00904000
            revoked=sk_X509_REVOKED_value(X509_CRL_get_REVOKED(crl), i);
#else
            revoked=(X509_REVOKED *)sk_value(X509_CRL_get_REVOKED(crl), i);
#endif
            if(ASN1_INTEGER_cmp(revoked->serialNumber,
                    X509_get_serialNumber(cert)) == 0) {
                serial=ASN1_INTEGER_get(revoked->serialNumber);
                cp=X509_NAME_oneline(issuer, NULL, 0);
                s_log(LOG_WARNING, "Certificate with serial %ld (0x%lX) "
                    "revoked per CRL from issuer %s", serial, serial, cp);
                OPENSSL_free(cp);
                X509_STORE_CTX_set_error(callback_ctx, X509_V_ERR_CERT_REVOKED);
                X509_OBJECT_free_contents(&obj);
                return 0; /* reject connection */
            }
        }
        X509_OBJECT_free_contents(&obj);
    }
    s_log(LOG_NOTICE, "CRL: verification passed");
    return 1; /* accept connection */
}

/**************************************** OCSP checking */

#if SSLEAY_VERSION_NUMBER >= 0x00907000L
static int ocsp_check(CLI *c, X509_STORE_CTX *callback_ctx,
        char *subject_name) {
    int error, retval=0;
    SOCKADDR_UNION addr;
    X509 *cert;
    X509 *issuer=NULL;
    OCSP_CERTID *certID;
    BIO *bio=NULL;
    OCSP_REQUEST *request=NULL;
    OCSP_RESPONSE *response=NULL;
    OCSP_BASICRESP *basicResponse=NULL;
    ASN1_GENERALIZEDTIME *revoked_at=NULL,
        *this_update=NULL, *next_update=NULL;
    int status, reason;

    /* TODO: check OCSP server specified in the certificate */
    s_log(LOG_DEBUG, "OCSP: starting verification");

    /* connect specified OCSP server (responder) */
    if((c->fd=
        socket(c->opt->ocsp_addr.addr[0].sa.sa_family, SOCK_STREAM, 0))<0) {
        sockerror("socket (auth_user)");
        return 0; /* reject connection */
    }
    if(alloc_fd(c->fd))
        goto cleanup;
    memcpy(&addr, &c->opt->ocsp_addr.addr[0], sizeof addr);
    if(connect_blocking(c, &addr, addr_len(addr)))
        goto cleanup;
    s_log(LOG_DEBUG, "OCSP: server connected");

    /* get current certificate ID */
    cert=X509_STORE_CTX_get_current_cert(callback_ctx); /* get current cert */
    if(X509_STORE_CTX_get1_issuer(&issuer, callback_ctx, cert)!=1) {
        sslerror("X509_STORE_CTX_get1_issuer");
        goto cleanup;
    }
    certID=OCSP_cert_to_id(0, cert, issuer);
    if(!certID) {
        sslerror("OCSP_cert_to_id");
        goto cleanup;
    }

    /* build request */
    request=OCSP_REQUEST_new();
    if(!request) {
        sslerror("OCSP_REQUEST_new");
        goto cleanup;
    }
    if(!OCSP_request_add0_id(request, certID)) {
        sslerror("OCSP_request_add0_id");
        goto cleanup;
    }
    OCSP_request_add1_nonce(request, 0, -1);

    /* send the request and get a response */
    /* FIXME: this code won't work with ucontext threading */
    /* (blocking sockets are used) */
    bio=BIO_new_fd(c->fd, BIO_NOCLOSE);
    setnonblock(c->fd, 0);
    response=OCSP_sendreq_bio(bio, c->opt->ocsp_path, request);
    setnonblock(c->fd, 1);
    if(!response) {
        sslerror("OCSP_sendreq_bio");
        goto cleanup;
    }
    error=OCSP_response_status(response);
    if(error!=OCSP_RESPONSE_STATUS_SUCCESSFUL) {
        s_log(LOG_WARNING, "OCSP: responder error: %d: %s",
            error, OCSP_response_status_str(error));
        goto cleanup;
    }
    s_log(LOG_DEBUG, "OCSP: response received");

    /* verify the response */
    basicResponse=OCSP_response_get1_basic(response);
    if(!basicResponse) {
        sslerror("OCSP_response_get1_basic");
        goto cleanup;
    }
    if(OCSP_check_nonce(request, basicResponse)<=0) {
        sslerror("OCSP_check_nonce");
        goto cleanup;
    }
    if(OCSP_basic_verify(basicResponse, NULL,
            c->opt->revocation_store, c->opt->ocsp_flags)<=0) {
        sslerror("OCSP_basic_verify");
        goto cleanup;
    }
    if(!OCSP_resp_find_status(basicResponse, certID, &status, &reason,
            &revoked_at, &this_update, &next_update)) {
        sslerror("OCSP_resp_find_status");
        goto cleanup;
    }
    s_log(LOG_NOTICE, "OCSP: status: %d: %s",
        status, OCSP_cert_status_str(status));
    log_time(LOG_INFO, "OCSP: this update", this_update);
    log_time(LOG_INFO, "OCSP: next update", next_update);
    /* check if the response is valid for at least one minute */
    if(!OCSP_check_validity(this_update, next_update, 60, -1)) {
        sslerror("OCSP_check_validity");
        goto cleanup;
    }
    if(status==V_OCSP_CERTSTATUS_REVOKED) {
        if(reason==-1)
            s_log(LOG_WARNING, "OCSP: certificate revoked");
        else
            s_log(LOG_WARNING, "OCSP: certificate revoked: %d: %s",
                reason, OCSP_crl_reason_str(reason));
        log_time(LOG_NOTICE, "OCSP: revoked at", revoked_at);
        goto cleanup;
    }

    /* success */
    s_log(LOG_NOTICE, "OCSP: verification passed");
    retval=1; /* accept connection */
cleanup:
    if(bio)
        BIO_free_all(bio);
    if(issuer)
        X509_free(issuer);
    if(request)
        OCSP_REQUEST_free(request);
    if(response)
        OCSP_RESPONSE_free(response);
    if(basicResponse)
        OCSP_BASICRESP_free(basicResponse);
    closesocket(c->fd);
    c->fd=-1; /* avoid double close on cleanup */
    return retval;
}
#endif /* OpenSSL-0.9.7 */

static void log_time(const int level, const char *txt, ASN1_TIME *t) {
    char *cp;
    BIO *bio;
    int n;

    if(!t)
        return;
    bio=BIO_new(BIO_s_mem());
    ASN1_TIME_print(bio, t);
    n=BIO_pending(bio);
    cp=malloc(n+1);
    n=BIO_read(bio, cp, n);
    cp[n]='\0';
    BIO_free(bio);
    s_log(level, "%s: %s", txt, cp);
    free(cp);
}

/* end of verify.c */

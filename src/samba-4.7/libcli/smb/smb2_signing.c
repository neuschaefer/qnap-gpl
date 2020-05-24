/*
   Unix SMB/CIFS implementation.
   SMB2 signing

   Copyright (C) Stefan Metzmacher 2009

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
#include "system/filesys.h"
#include "../libcli/smb/smb_common.h"
#include "../lib/crypto/crypto.h"
#include "lib/util/iov_buf.h"

#ifdef QNAPNAS_OPENSSL_ENC
#include "openssl/bio.h"
#include "openssl/evp.h"
#endif /* QNAPNAS_OPENSSL_ENC */

NTSTATUS smb2_signing_sign_pdu(DATA_BLOB signing_key,
			       enum protocol_types protocol,
			       struct iovec *vector,
			       int count)
{
	uint8_t *hdr;
	uint64_t session_id;
	uint8_t res[16];
	int i;

	if (count < 2) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	if (vector[0].iov_len != SMB2_HDR_BODY) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	hdr = (uint8_t *)vector[0].iov_base;

	session_id = BVAL(hdr, SMB2_HDR_SESSION_ID);
	if (session_id == 0) {
		/*
		 * do not sign messages with a zero session_id.
		 * See MS-SMB2 3.2.4.1.1
		 */
		return NT_STATUS_OK;
	}

	if (signing_key.length == 0) {
		DEBUG(2,("Wrong session key length %u for SMB2 signing\n",
			 (unsigned)signing_key.length));
		return NT_STATUS_ACCESS_DENIED;
	}

	memset(hdr + SMB2_HDR_SIGNATURE, 0, 16);

	SIVAL(hdr, SMB2_HDR_FLAGS, IVAL(hdr, SMB2_HDR_FLAGS) | SMB2_HDR_FLAG_SIGNED);

	if (protocol >= PROTOCOL_SMB2_24) {
		struct aes_cmac_128_context ctx;
		uint8_t key[AES_BLOCK_SIZE];

		ZERO_STRUCT(key);
		memcpy(key, signing_key.data, MIN(signing_key.length, 16));

		aes_cmac_128_init(&ctx, key);
		for (i=0; i < count; i++) {
			aes_cmac_128_update(&ctx,
					(const uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
		}
		aes_cmac_128_final(&ctx, res);
	} else {
		struct HMACSHA256Context m;
		uint8_t digest[SHA256_DIGEST_LENGTH];

		ZERO_STRUCT(m);
		hmac_sha256_init(signing_key.data, MIN(signing_key.length, 16), &m);
		for (i=0; i < count; i++) {
			hmac_sha256_update((const uint8_t *)vector[i].iov_base,
					   vector[i].iov_len, &m);
		}
		hmac_sha256_final(digest, &m);
		memcpy(res, digest, 16);
	}
	DEBUG(5,("signed SMB2 message\n"));

	memcpy(hdr + SMB2_HDR_SIGNATURE, res, 16);

	return NT_STATUS_OK;
}

NTSTATUS smb2_signing_check_pdu(DATA_BLOB signing_key,
				enum protocol_types protocol,
				const struct iovec *vector,
				int count)
{
	const uint8_t *hdr;
	const uint8_t *sig;
	uint64_t session_id;
	uint8_t res[16];
	static const uint8_t zero_sig[16] = { 0, };
	int i;

	if (count < 2) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	if (vector[0].iov_len != SMB2_HDR_BODY) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	hdr = (const uint8_t *)vector[0].iov_base;

	session_id = BVAL(hdr, SMB2_HDR_SESSION_ID);
	if (session_id == 0) {
		/*
		 * do not sign messages with a zero session_id.
		 * See MS-SMB2 3.2.4.1.1
		 */
		return NT_STATUS_OK;
	}

	if (signing_key.length == 0) {
		/* we don't have the session key yet */
		return NT_STATUS_OK;
	}

	sig = hdr+SMB2_HDR_SIGNATURE;

	if (protocol >= PROTOCOL_SMB2_24) {
		struct aes_cmac_128_context ctx;
		uint8_t key[AES_BLOCK_SIZE];

		ZERO_STRUCT(key);
		memcpy(key, signing_key.data, MIN(signing_key.length, 16));

		aes_cmac_128_init(&ctx, key);
		aes_cmac_128_update(&ctx, hdr, SMB2_HDR_SIGNATURE);
		aes_cmac_128_update(&ctx, zero_sig, 16);
		for (i=1; i < count; i++) {
			aes_cmac_128_update(&ctx,
					(const uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
		}
		aes_cmac_128_final(&ctx, res);
	} else {
		struct HMACSHA256Context m;
		uint8_t digest[SHA256_DIGEST_LENGTH];

		ZERO_STRUCT(m);
		hmac_sha256_init(signing_key.data, MIN(signing_key.length, 16), &m);
		hmac_sha256_update(hdr, SMB2_HDR_SIGNATURE, &m);
		hmac_sha256_update(zero_sig, 16, &m);
		for (i=1; i < count; i++) {
			hmac_sha256_update((const uint8_t *)vector[i].iov_base,
					   vector[i].iov_len, &m);
		}
		hmac_sha256_final(digest, &m);
		memcpy(res, digest, 16);
	}

	if (memcmp_const_time(res, sig, 16) != 0) {
		DEBUG(0,("Bad SMB2 signature for message\n"));
		dump_data(0, sig, 16);
		dump_data(0, res, 16);
		return NT_STATUS_ACCESS_DENIED;
	}

	return NT_STATUS_OK;
}

void smb2_key_derivation(const uint8_t *KI, size_t KI_len,
			 const uint8_t *Label, size_t Label_len,
			 const uint8_t *Context, size_t Context_len,
			 uint8_t KO[16])
{
	struct HMACSHA256Context ctx;
	uint8_t buf[4];
	static const uint8_t zero = 0;
	uint8_t digest[SHA256_DIGEST_LENGTH];
	uint32_t i = 1;
	uint32_t L = 128;

	/*
	 * a simplified version of
	 * "NIST Special Publication 800-108" section 5.1
	 * using hmac-sha256.
	 */
	hmac_sha256_init(KI, KI_len, &ctx);

	RSIVAL(buf, 0, i);
	hmac_sha256_update(buf, sizeof(buf), &ctx);
	hmac_sha256_update(Label, Label_len, &ctx);
	hmac_sha256_update(&zero, 1, &ctx);
	hmac_sha256_update(Context, Context_len, &ctx);
	RSIVAL(buf, 0, L);
	hmac_sha256_update(buf, sizeof(buf), &ctx);

	hmac_sha256_final(digest, &ctx);

	memcpy(KO, digest, 16);
}

NTSTATUS smb2_signing_encrypt_pdu(DATA_BLOB encryption_key,
				  uint16_t cipher_id,
				  struct iovec *vector,
				  int count)
{
	uint8_t *tf;
	uint8_t sig[16];
	int i;
	size_t a_total;
	ssize_t m_total;
	union {
		struct aes_ccm_128_context ccm;
		struct aes_gcm_128_context gcm;
	} c;
	uint8_t key[AES_BLOCK_SIZE];
#ifdef QNAPNAS_OPENSSL_ENC
	EVP_CIPHER_CTX *evp_ctx;
	uint8_t nonce[AES_CCM_128_NONCE_SIZE];
	int outlen;
	uint8_t *ccm_pt = NULL;
	uint8_t *outbuf = NULL;
	uint8_t *ptr = NULL;
#endif /* QNAPNAS_OPENSSL_ENC */

	if (count < 1) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	if (vector[0].iov_len != SMB2_TF_HDR_SIZE) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	tf = (uint8_t *)vector[0].iov_base;

	if (encryption_key.length == 0) {
		DEBUG(2,("Wrong encryption key length %u for SMB2 signing\n",
			 (unsigned)encryption_key.length));
		return NT_STATUS_ACCESS_DENIED;
	}

	a_total = SMB2_TF_HDR_SIZE - SMB2_TF_NONCE;

	m_total = iov_buflen(&vector[1], count-1);
	if (m_total == -1) {
		return NT_STATUS_BUFFER_TOO_SMALL;
	}

	SSVAL(tf, SMB2_TF_FLAGS, SMB2_TF_FLAGS_ENCRYPTED);
	SIVAL(tf, SMB2_TF_MSG_SIZE, m_total);

	ZERO_STRUCT(key);
	memcpy(key, encryption_key.data,
	       MIN(encryption_key.length, AES_BLOCK_SIZE));

	switch (cipher_id) {
	case SMB2_ENCRYPTION_AES128_CCM:
#ifdef QNAPNAS_OPENSSL_ENC
		memcpy(nonce, tf + SMB2_TF_NONCE, AES_CCM_128_NONCE_SIZE);

		evp_ctx = EVP_CIPHER_CTX_new();

		/* Set cipher type and mode */
		EVP_EncryptInit_ex(evp_ctx, EVP_aes_128_ccm(), NULL, NULL, NULL);

		/* Set nonce length if default 96 bits is not appropriate */
		EVP_CIPHER_CTX_ctrl(evp_ctx, EVP_CTRL_CCM_SET_IVLEN, AES_CCM_128_NONCE_SIZE, NULL);

		/* Set tag length */
		EVP_CIPHER_CTX_ctrl(evp_ctx, EVP_CTRL_CCM_SET_TAG, 16, NULL);

		/* Initialise key and IV */
		EVP_EncryptInit_ex(evp_ctx, NULL, NULL, key, nonce);

		/* Set plaintext length: only needed if AAD is used */
		EVP_EncryptUpdate(evp_ctx, NULL, &outlen, NULL, m_total);

		/* Zero or one call to specify any AAD */
		EVP_EncryptUpdate(evp_ctx, NULL, &outlen, tf + SMB2_TF_NONCE, a_total);

		/* 
		 * "smb2 max {read,trans,write}" can reach 8388608 (8MB),
		 * hence use heap (e.g. malloc) instead of stack (e.g. local array)
		 * to avoid stack overflow. 
		 * 
		 * FIXME: use calloc() instead of malloc()?
		 */
		ccm_pt = (uint8_t *) malloc(m_total * sizeof(uint8_t));
		outbuf = (uint8_t *) malloc(m_total * sizeof(uint8_t));
		ptr = ccm_pt;
		for (i=1; i < count; i++) {
			memcpy(ptr, vector[i].iov_base, vector[i].iov_len);
			ptr += vector[i].iov_len;
		}

		/* Encrypt plaintext: can only be called once */
		EVP_EncryptUpdate(evp_ctx, outbuf, &outlen, ccm_pt, m_total);

		/* Finalise: note get no output for CCM */
		EVP_EncryptFinal_ex(evp_ctx, outbuf, &outlen);

		ptr = outbuf;
		for (i=1; i < count; i++) {
			memcpy(vector[i].iov_base, ptr, vector[i].iov_len);
			ptr += vector[i].iov_len;
		}

		/* Get tag */
		EVP_CIPHER_CTX_ctrl(evp_ctx, EVP_CTRL_CCM_GET_TAG, 16, sig);

		EVP_CIPHER_CTX_free(evp_ctx);

		if (ccm_pt) {
			free(ccm_pt);
		}
		if (outbuf) {
			free(outbuf);
		}
#else /* QNAPNAS_OPENSSL_ENC */
		aes_ccm_128_init(&c.ccm, key,
				 tf + SMB2_TF_NONCE,
				 a_total, m_total);
		memset(tf + SMB2_TF_NONCE + AES_CCM_128_NONCE_SIZE, 0,
		       16 - AES_CCM_128_NONCE_SIZE);
		aes_ccm_128_update(&c.ccm, tf + SMB2_TF_NONCE, a_total);
		for (i=1; i < count; i++) {
			aes_ccm_128_update(&c.ccm,
					(const uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
			aes_ccm_128_crypt(&c.ccm,
					(uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
		}
		aes_ccm_128_digest(&c.ccm, sig);
#endif /* QNAPNAS_OPENSSL_ENC */
		break;

	case SMB2_ENCRYPTION_AES128_GCM:
		aes_gcm_128_init(&c.gcm, key, tf + SMB2_TF_NONCE);
		memset(tf + SMB2_TF_NONCE + AES_GCM_128_IV_SIZE, 0,
		       16 - AES_GCM_128_IV_SIZE);
		aes_gcm_128_updateA(&c.gcm, tf + SMB2_TF_NONCE, a_total);
		for (i=1; i < count; i++) {
			aes_gcm_128_crypt(&c.gcm,
					(uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
			aes_gcm_128_updateC(&c.gcm,
					(const uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
		}
		aes_gcm_128_digest(&c.gcm, sig);
		break;

	default:
		ZERO_STRUCT(key);
		return NT_STATUS_INVALID_PARAMETER;
	}
	ZERO_STRUCT(key);

	memcpy(tf + SMB2_TF_SIGNATURE, sig, 16);

	DEBUG(5,("encrypt SMB2 message\n"));

	return NT_STATUS_OK;
}

NTSTATUS smb2_signing_decrypt_pdu(DATA_BLOB decryption_key,
				  uint16_t cipher_id,
				  struct iovec *vector,
				  int count)
{
	uint8_t *tf;
	uint16_t flags;
	uint8_t *sig_ptr = NULL;
	uint8_t sig[16];
	int i;
	size_t a_total;
	ssize_t m_total;
	uint32_t msg_size = 0;
	union {
		struct aes_ccm_128_context ccm;
		struct aes_gcm_128_context gcm;
	} c;
	uint8_t key[AES_BLOCK_SIZE];
#ifdef QNAPNAS_OPENSSL_ENC
	EVP_CIPHER_CTX *evp_ctx;
	uint8_t nonce[AES_CCM_128_NONCE_SIZE];
	int outlen;
	int rv;
	uint8_t *ccm_ct = NULL;
	uint8_t *outbuf = NULL;
	uint8_t *ptr = NULL;
#endif /* QNAPNAS_OPENSSL_ENC */

	if (count < 1) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	if (vector[0].iov_len != SMB2_TF_HDR_SIZE) {
		return NT_STATUS_INVALID_PARAMETER;
	}

	tf = (uint8_t *)vector[0].iov_base;

	if (decryption_key.length == 0) {
		DEBUG(2,("Wrong decryption key length %u for SMB2 signing\n",
			 (unsigned)decryption_key.length));
		return NT_STATUS_ACCESS_DENIED;
	}

	a_total = SMB2_TF_HDR_SIZE - SMB2_TF_NONCE;

	m_total = iov_buflen(&vector[1], count-1);
	if (m_total == -1) {
		return NT_STATUS_BUFFER_TOO_SMALL;
	}

	flags = SVAL(tf, SMB2_TF_FLAGS);
	msg_size = IVAL(tf, SMB2_TF_MSG_SIZE);

	if (flags != SMB2_TF_FLAGS_ENCRYPTED) {
		return NT_STATUS_ACCESS_DENIED;
	}

	if (msg_size != m_total) {
		return NT_STATUS_INTERNAL_ERROR;
	}

	ZERO_STRUCT(key);
	memcpy(key, decryption_key.data,
	       MIN(decryption_key.length, AES_BLOCK_SIZE));

	switch (cipher_id) {
	case SMB2_ENCRYPTION_AES128_CCM:
#ifdef QNAPNAS_OPENSSL_ENC
		memcpy(nonce, tf + SMB2_TF_NONCE, AES_CCM_128_NONCE_SIZE);

		evp_ctx = EVP_CIPHER_CTX_new();

		/* Select cipher */
		EVP_DecryptInit_ex(evp_ctx, EVP_aes_128_ccm(), NULL, NULL, NULL);

		/* Set nonce length, omit for 96 bits */
		EVP_CIPHER_CTX_ctrl(evp_ctx, EVP_CTRL_CCM_SET_IVLEN, AES_CCM_128_NONCE_SIZE, NULL);

		/* Set expected tag value */
		EVP_CIPHER_CTX_ctrl(evp_ctx, EVP_CTRL_CCM_SET_TAG,16, tf + SMB2_TF_SIGNATURE);

		/* Specify key and IV */
		EVP_DecryptInit_ex(evp_ctx, NULL, NULL, key, nonce);

		/* 
		 * "smb2 max {read,trans,write}" can reach 8388608 (8MB),
		 * hence use heap (e.g. malloc) instead of stack (e.g. local array)
		 * to avoid stack overflow. 
		 * 
		 * FIXME: use calloc() instead of malloc()?
		 */
		ccm_ct = (uint8_t *) malloc(m_total * sizeof(uint8_t));
		outbuf = (uint8_t *) malloc(m_total * sizeof(uint8_t));
		ptr = ccm_ct;
		for (i=1; i < count; i++) {
			memcpy(ptr,vector[i].iov_base,vector[i].iov_len);
			ptr += vector[i].iov_len;
		}

		/* Set ciphertext length: only needed if we have AAD */
		/* 
		 * FIXME: Revise last parameter if ccm_ct is not (uint8_t *).
		 * e.g.	EVP_DecryptUpdate(evp_ctx, NULL, &outlen, NULL, sizeof(ccm_ct));
		 */
		EVP_DecryptUpdate(evp_ctx, NULL, &outlen, NULL, m_total);

		/* Zero or one call to specify any AAD */
		EVP_DecryptUpdate(evp_ctx, NULL, &outlen, tf + SMB2_TF_NONCE, a_total);

		/* Decrypt plaintext, verify tag: can only be called once */
		rv = EVP_DecryptUpdate(evp_ctx, outbuf, &outlen, ccm_ct, m_total);
		/* Output decrypted block: if tag verify failed we get nothing */
		if (rv != 1) {
			DEBUG(3,("Plaintext not available: tag verify failed.\n"));
		} else {
			ptr = outbuf;
			for (i=1; i < count; i++) {
				memcpy(vector[i].iov_base, ptr, vector[i].iov_len);
				ptr += vector[i].iov_len;
			}
		}

		EVP_CIPHER_CTX_free(evp_ctx);

		if (ccm_ct) {
			free(ccm_ct);
		}
		if (outbuf) {
			free(outbuf);
		}
#else /* QNAPNAS_OPENSSL_ENC */
		aes_ccm_128_init(&c.ccm, key,
				 tf + SMB2_TF_NONCE,
				 a_total, m_total);
		aes_ccm_128_update(&c.ccm, tf + SMB2_TF_NONCE, a_total);
		for (i=1; i < count; i++) {
			aes_ccm_128_crypt(&c.ccm,
					(uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
			aes_ccm_128_update(&c.ccm,
					( uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
		}
		aes_ccm_128_digest(&c.ccm, sig);
#endif /* QNAPNAS_OPENSSL_ENC */
		break;

	case SMB2_ENCRYPTION_AES128_GCM:
		aes_gcm_128_init(&c.gcm, key, tf + SMB2_TF_NONCE);
		aes_gcm_128_updateA(&c.gcm, tf + SMB2_TF_NONCE, a_total);
		for (i=1; i < count; i++) {
			aes_gcm_128_updateC(&c.gcm,
					(const uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
			aes_gcm_128_crypt(&c.gcm,
					(uint8_t *)vector[i].iov_base,
					vector[i].iov_len);
		}
		aes_gcm_128_digest(&c.gcm, sig);
		break;

	default:
		ZERO_STRUCT(key);
		return NT_STATUS_INVALID_PARAMETER;
	}
	ZERO_STRUCT(key);

#ifdef QNAPNAS_OPENSSL_ENC
	/*
	 * FIXME: Need to comapre signature?
	 */
        if (cipher_id != SMB2_ENCRYPTION_AES128_CCM) {
		sig_ptr = tf + SMB2_TF_SIGNATURE;
		if (memcmp(sig_ptr, sig, 16) != 0) {
			return NT_STATUS_ACCESS_DENIED;
		}
	}
#else
	sig_ptr = tf + SMB2_TF_SIGNATURE;
	if (memcmp(sig_ptr, sig, 16) != 0) {
		return NT_STATUS_ACCESS_DENIED;
	}
#endif /* QNAPNAS_OPENSSL_ENC */

	DEBUG(5,("decrypt SMB2 message\n"));

	return NT_STATUS_OK;
}

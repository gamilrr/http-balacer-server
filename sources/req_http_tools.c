#include "req_http_tools.h"
#include <openssl\evp.h>
//=================================================================================================
//=========================== HELPERS FUNCTION ====================================================
//=================================================================================================

//calculate the HMAC signature and store it in ReqHTTP.hmac.sing
int hmac_calc(ReqHTTP* req) {

	unsigned char* sign = NULL;
	const EVP_MD *md;

	sign = (unsigned char*)MALLOC(EVP_MAX_MD_SIZE);
	if (sign == NULL) {
		return 0;
	}

	switch (req->hmac.md) {
		/*case MD2:
		req->hmac.sign_len = 16;
		md = EVP_md2();
		break;
		*/
	case MD4:
		req->hmac.sign_len = 16;
		md = EVP_md4();
		break;

	case MD5:
		req->hmac.sign_len = 16;
		md = EVP_md5();
		break;

	case SHA1:
		req->hmac.sign_len = 20;
		md = EVP_sha1();
		break;

	case SHA224:
		req->hmac.sign_len = 28;
		md = EVP_sha224();
		break;

	case SHA256:
		req->hmac.sign_len = 32;
		md = EVP_sha256();
		break;

	/*case SHA348:
		req->hmac.sign_len = 48;
		md = sha348();
		break;*/

	/*case SHA512:
		req->hmac.sign_len = 64;
		md = EVP_sha512();
		break;*/

	case RIPEMD160:
		req->hmac.sign_len = 20;
		md = EVP_ripemd160();
		break;

	default:
		req->hmac.sign_len = 32;
		md = EVP_sha256();
		break;
	}
	
	req_crypto_hmac(req->hmac.hmac_pkey, req->hmac.pkey_len, md, req->buffer, req->len, sign, &(req->hmac.sign_len));

	FREE(req->hmac.sign);

	if (!copy_dymem(&(req->hmac.sign), sign, req->hmac.sign_len)) {
		FREE(sign);
		return 0;
	}

	FREE(sign);
	return 1;
}


static int check_hmac(ReqHTTP* req, unsigned char* resSign) {

	if (resSign == NULL) {
		return 0;
	}

	if (!hmac_calc(req)) {
		return 0;
	}

	if (MEMCMP(resSign, req->hmac.sign, req->hmac.sign_len) != 0) {
		return 0;
	}

	return 1;

}


//this function does a 128 CBC AES encryption, It uses standard block padding (aka PKCS padding)
int aesCbc128_encrypt_data(ReqHTTP* req) {

	unsigned char* encrypt_data = NULL;

	encrypt_data = (unsigned char*)MALLOC((req->len) + EVP_MAX_BLOCK_LENGTH);
	if (encrypt_data == NULL) {
		return 0;
	}


	if (!req_encrypt_aes128(req->aes.aes_key, req->aes.aes_iv, encrypt_data, &(req->enc_len), req->buffer, req->len)) {
		FREE(encrypt_data);
		return 0;
	}

	FREE(req->buffer);

	if (!copy_dymem(&(req->buffer), encrypt_data, req->enc_len)) {
		FREE(encrypt_data);
		return 0;
	}

	req->len = req->enc_len;

	FREE(encrypt_data);

	return 1;
}

//this function does a 128 CBC AES decryption, It uses standard block padding (aka PKCS padding)
int aesCbc128_decrypt_data(ReqHTTP* req) {

	unsigned char* decrypt_data = NULL;

	decrypt_data = (unsigned char*)MALLOC((req->len) + EVP_MAX_BLOCK_LENGTH);
	if (decrypt_data == NULL) {
		return 0;
	}


	if (!req_decrypt_aes128(req->aes.aes_key, req->aes.aes_iv, decrypt_data, &(req->pl_len), req->buffer, req->len)) {
		FREE(decrypt_data);
		return 0;
	}

	FREE(req->buffer);

	if (!copy_dymem(&(req->buffer), decrypt_data, req->pl_len)) {
		FREE(decrypt_data);
		return 0;
	}

	req->len = req->pl_len;

	FREE(decrypt_data);

	return 1;
}

REQ_RET accepted_data_check(ReqHTTP* req) {

	unsigned char* resSign;

	//check is the data is encrypted first
	if (req->aes.aes_key != NULL && req->enc_wrap == 1) {
		if (!aesCbc128_decrypt_data(req))
			return REQ_FAIL_HMAC;
	}


	if (req->hmac.hmac_pkey != NULL)
	if (req->appd == APPEND) {

		req->len = req->len - req->hmac.sign_len;
		if (!copy_dymem(&resSign, req->buffer + req->len, req->hmac.sign_len)) {
			return REQ_FAIL_DMEMORY;
		}
		
		//devide the buffer sent by server 
		req->buffer = (unsigned char*)REALLOC(req->buffer, req->len);
		if (req->buffer == NULL) {
			return REQ_FAIL_DMEMORY;
		}

		if (!check_hmac(req, resSign)) {
			FREE(resSign);			
			return 	REQ_FAIL_HMAC;
		}

		FREE(resSign);

	}else if (req->appd == NAPPEND) {
		if (!hmac_calc(req)) {
			return REQ_FAIL_CRYP;
		}
	}


	if (req->aes.aes_key != NULL && req->enc_wrap == 0) {
		if (!aesCbc128_decrypt_data(req))
			return REQ_FAIL_HMAC;
	}


	return REQ_SUCCESS;
}




//function to copy memory dynamically, must be freed the first parameter
int copy_dymem(unsigned char **dest, unsigned char *src, unsigned long len) {

	(*dest) = (unsigned char*)MALLOC(len);
	if ((*dest) == NULL) {
		return 0;
	}

	MEMCPY((*dest), src, len);

	return 1;
}

//callback funtion such is called when the process recieves the server response body
unsigned long write_callback_func(void *ptr, unsigned long size, unsigned long nmemb, void* s)
{
	
	unsigned long data_recv = size * nmemb;
	ReqChunk* b = (ReqChunk*)s;
	unsigned long new_len = b->len + data_recv;

	b->buffer = (unsigned char*)REALLOC(b->buffer, new_len);

	if (b->buffer == NULL) {
		return 0;
	}

	MEMCPY(b->buffer + b->len, ptr, data_recv);

	b->len = new_len;

	return data_recv;
}

//callback function such is called each time a data chunk will be sent to server 
unsigned long read_callback_func(void *ptr, unsigned long size, unsigned long nitems, void *stream)
{
	unsigned long data_recv = size * nitems;
	ReqChunk *rarg = (ReqChunk *)stream;
	unsigned long len = rarg->len - rarg->pos;

	if (len > data_recv)
		len = data_recv;

	MEMCPY(ptr, rarg->buffer + rarg->pos, len);
	rarg->pos += len;

	return len;
}


#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <string.h>
#include <stdio.h>
#include "crypto.h"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#define SITE_SECRET "http://hackmelbourne.org"
#define SITE_SECRET_LEN 24
EVP_MD_CTX *mdctx;
const EVP_MD *md;

/** Initializes the state of the OpenSSL engine.
  * Must be called before any other functions are called
  */
int init_crypto_state() {
	md = EVP_sha512();
	if (!md) {
		printf("Could not initalize crypto!\n");
		return -1;
	}
	return 0;
}

unsigned char * get_random_bytes(size_t len) {
	char timeseed[32];
    time_t now = time(NULL);
    size_t b = snprintf(timeseed,32,"%ld",now);
    RAND_add(&timeseed[0],b,2);

	unsigned char *randbytes = malloc(sizeof(unsigned char)*len);
    RAND_bytes(randbytes,len);

    RAND_cleanup();
    return randbytes;
}

char * getBase64String(char *inbytes, size_t inLen) {
	BIO *b64, *bmem;
	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	bmem = BIO_push(b64, bmem);
	BIO_write(bmem,inbytes,inLen);
	BIO_flush(bmem);
	BUF_MEM *bptr;
	BIO_get_mem_ptr(bmem,&bptr);
	size_t outputLen = bptr->length;
	char *data = malloc(outputLen * sizeof(char)+1);
	memcpy(data,bptr->data,outputLen);
	data[outputLen] = NULL;
	BIO_free_all(bmem);
	BIO_free_all(b64);
	return data;
}

char * decodeBase64String(char *inbytes, size_t inLen, char *storage, size_t maxOutLen) {
	BIO *bmem, *b64;
	//bmem = BIO_new(BIO_s_mem());
	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64,BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf(inbytes,inLen);
	bmem = BIO_push(b64,bmem);
	//BIO_write(bmem,inbytes,inLen);
	//BIO_flush(bmem);
	char *storptr = NULL;
	if (storage) {
		storptr = storage;
	} else {
		storptr = malloc(maxOutLen * sizeof(char) + 1);
	} 
	BIO_read(bmem,storptr,maxOutLen);
	//storptr[maxOutLen+1] = '\0';
	BIO_free_all(bmem);
	return storptr;
}
/** 
  * Return the SHA-512 based hash.
  */
char * hash_string(char *inbytes, size_t inLen, size_t *wroteBytes) {
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;
	mdctx = EVP_MD_CTX_create();
	
	EVP_DigestInit_ex(mdctx, md,NULL);
	EVP_DigestUpdate(mdctx,inbytes,inLen);
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	
	char *outputBuffer = (char *)malloc(sizeof(char) * 64);
	for(int i=0; i<SHA512_DIGEST_LENGTH; i++) {
		outputBuffer[i] = md_value[i];
	}
	if (wroteBytes != NULL) {
		*wroteBytes = 512;
	}
	EVP_MD_CTX_destroy(mdctx);
	return outputBuffer;	
}

/** Used to derive a key from a tag UID.
  * Currently not in use 
  */
char *get_derived_key(char *uid) {
	size_t hashBufLen=4+SITE_SECRET_LEN;
	char hashBuffer[hashBufLen];
	memset(hashBuffer,0,hashBufLen);
	// Copy the UID (first four bytes)
	//strncpy(&hashBuffer[0],uid,4);
	memcpy(&hashBuffer[0],uid,4);
	char *hashBufPtr = &hashBuffer[0]+4;
	// Copy the site secret
	strncpy(hashBufPtr, SITE_SECRET, SITE_SECRET_LEN);	
	// Get the hashed copy
	
	char *hashed = hash_string(&hashBuffer[0],4+SITE_SECRET_LEN, NULL);
	// Copy the first 3, and last 3 bytes of the hash
	char *key = (char *)malloc(sizeof(char) * 6);
	strncpy(key,hashed,3);
	strncpy(&key[3],&hashed[61],3);
	free(hashed);
	return key;
}

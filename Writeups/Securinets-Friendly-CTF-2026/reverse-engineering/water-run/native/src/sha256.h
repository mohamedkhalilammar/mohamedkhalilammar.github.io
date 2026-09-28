#ifndef CG_SHA256_H
#define CG_SHA256_H

#include <stddef.h>

#define CG_SHA256_DIGEST 32
#define CG_SHA256_BLOCK 64
#define CG_HMAC_MSG_MAX 256

void cg_sha256(const unsigned char *msg, size_t len, unsigned char out[CG_SHA256_DIGEST]);

int cg_hmac_sha256(const unsigned char *key, size_t key_len,
                   const unsigned char *msg, size_t msg_len,
                   unsigned char out[CG_SHA256_DIGEST]);

#endif

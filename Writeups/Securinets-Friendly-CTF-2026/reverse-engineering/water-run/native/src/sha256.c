#include <string.h>

#include "sha256.h"

static const unsigned int K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u,
    0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u, 0xe49b69c1u, 0xefbe4786u,
    0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
    0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu,
    0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au,
    0x5b9cca4fu, 0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void compress(unsigned int h[8], const unsigned char block[CG_SHA256_BLOCK])
{
    unsigned int w[64], a, b, c, d, e, f, g, hh, t1, t2;
    int i;

    for (i = 0; i < 16; i++)
        w[i] = ((unsigned int)block[i * 4] << 24) | ((unsigned int)block[i * 4 + 1] << 16) |
               ((unsigned int)block[i * 4 + 2] << 8) | (unsigned int)block[i * 4 + 3];

    for (i = 16; i < 64; i++) {
        unsigned int s0 = ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        unsigned int s1 = ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    a = h[0]; b = h[1]; c = h[2]; d = h[3];
    e = h[4]; f = h[5]; g = h[6]; hh = h[7];

    for (i = 0; i < 64; i++) {
        unsigned int S1 = ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25);
        unsigned int ch = (e & f) ^ ((~e) & g);
        unsigned int S0 = ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22);
        unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
        t1 = hh + S1 + ch + K[i] + w[i];
        t2 = S0 + maj;
        hh = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;

    memset(w, 0, sizeof(w));
}

void cg_sha256(const unsigned char *msg, size_t len, unsigned char out[CG_SHA256_DIGEST])
{
    unsigned int h[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };
    unsigned char tail[CG_SHA256_BLOCK * 2];
    unsigned long long bits = (unsigned long long)len * 8;
    size_t full = len / CG_SHA256_BLOCK;
    size_t rest = len % CG_SHA256_BLOCK;
    size_t tail_len, i;

    for (i = 0; i < full; i++)
        compress(h, msg + i * CG_SHA256_BLOCK);

    memset(tail, 0, sizeof(tail));
    if (rest)
        memcpy(tail, msg + full * CG_SHA256_BLOCK, rest);
    tail[rest] = 0x80;
    tail_len = (rest < 56) ? CG_SHA256_BLOCK : CG_SHA256_BLOCK * 2;

    for (i = 0; i < 8; i++)
        tail[tail_len - 1 - i] = (unsigned char)(bits >> (8 * i));

    for (i = 0; i < tail_len / CG_SHA256_BLOCK; i++)
        compress(h, tail + i * CG_SHA256_BLOCK);

    for (i = 0; i < 8; i++) {
        out[i * 4] = (unsigned char)(h[i] >> 24);
        out[i * 4 + 1] = (unsigned char)(h[i] >> 16);
        out[i * 4 + 2] = (unsigned char)(h[i] >> 8);
        out[i * 4 + 3] = (unsigned char)h[i];
    }

    memset(tail, 0, sizeof(tail));
    memset(h, 0, sizeof(h));
}

int cg_hmac_sha256(const unsigned char *key, size_t key_len,
                   const unsigned char *msg, size_t msg_len,
                   unsigned char out[CG_SHA256_DIGEST])
{
    unsigned char k[CG_SHA256_BLOCK];
    unsigned char pad[CG_SHA256_BLOCK + CG_SHA256_DIGEST];
    unsigned char scratch[CG_SHA256_BLOCK + CG_HMAC_MSG_MAX];
    size_t i;

    if (msg_len > CG_HMAC_MSG_MAX)
        return -1;

    memset(k, 0, sizeof(k));
    if (key_len > CG_SHA256_BLOCK)
        cg_sha256(key, key_len, k);
    else
        memcpy(k, key, key_len);

    for (i = 0; i < CG_SHA256_BLOCK; i++)
        scratch[i] = k[i] ^ 0x36;
    if (msg_len)
        memcpy(scratch + CG_SHA256_BLOCK, msg, msg_len);
    cg_sha256(scratch, CG_SHA256_BLOCK + msg_len, pad + CG_SHA256_BLOCK);

    for (i = 0; i < CG_SHA256_BLOCK; i++)
        pad[i] = k[i] ^ 0x5c;

    cg_sha256(pad, CG_SHA256_BLOCK + CG_SHA256_DIGEST, out);

    memset(k, 0, sizeof(k));
    memset(pad, 0, sizeof(pad));
    memset(scratch, 0, sizeof(scratch));
    return 0;
}

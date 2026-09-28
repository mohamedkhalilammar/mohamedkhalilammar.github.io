#include <jni.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const unsigned char REAL_LOOSELIPS_CT[] = {
    0x6f, 0x2f, 0xf5, 0x97, 0xc7, 0xde, 0xc5, 0x5b, 0xd1, 0xbf, 0xb2, 0x4b,
    0x32, 0xf7, 0x41, 0xed, 0x28, 0x60, 0x5f, 0x3d, 0xba, 0x11, 0xa5, 0xf3,
    0xc8, 0x03, 0x7d, 0x7a, 0x83, 0x4b, 0xef, 0xa5, 0x4d, 0x5f, 0x97, 0x62,
    0x04, 0xd0, 0xa7, 0x0c, 0x50, 0x79, 0xbc, 0x34, 0x6f, 0x1d, 0xb3, 0x6d,
    0x8f, 0xc2, 0x98, 0x23, 0x42, 0xfc, 0x85, 0x0b, 0x0f, 0x66, 0x88, 0x98,
    0xfb, 0xf5, 0x40, 0x12, 0x91, 0xa0, 0xc3, 0x2c, 0x2f, 0x6e, 0x6b, 0x07,
    0xe7, 0xc6, 0x80,
};

static const unsigned char REAL_EVIDENCELOCKER_CT[] = {
    0x06, 0xf6, 0x38, 0xe4, 0x25, 0x06, 0xb0, 0xa6, 0xf8, 0x3b, 0x83, 0x0e,
    0xeb, 0x0d, 0x9d, 0x5a, 0x4a, 0xfb, 0x78, 0xd1, 0x46, 0x2d, 0x80, 0xf0,
    0x6e, 0x57, 0x91, 0xd0, 0x54, 0x9b, 0x10, 0x5a, 0x54, 0xc0, 0xfd, 0xca,
    0x45, 0x91, 0xf4,
};

static const unsigned char REAL_FRONTDOOR_CT[] = {
    0x77, 0x68, 0x33, 0x80, 0x59, 0x27, 0x52, 0xcd, 0x81, 0x31, 0x81, 0x6b,
    0xfa, 0xfb, 0x16, 0xbe, 0x7e, 0x68, 0x3d, 0x80, 0xa9, 0x72, 0xf9, 0xc5,
    0x7d, 0xc3, 0x16, 0xb9, 0xc3, 0x31, 0x25, 0xd9, 0xcb, 0x41, 0xcb, 0xcf,
    0x1e, 0xa5,
};

static const unsigned char REAL_INTERNALTOOLS_CT[] = {
    0x76, 0xf6, 0xa7, 0x07, 0x4e, 0xe2, 0xbc, 0x5b, 0x79, 0x50, 0xfc, 0xef,
    0xb0, 0x75, 0x6e, 0x3e, 0x43, 0xc0, 0xd6, 0xde, 0x46, 0x61, 0xcc, 0xe0,
    0x93, 0x59, 0x25, 0x12, 0xfe, 0x8d, 0xd6, 0xb0, 0x2a, 0xce, 0x5b,
};

static const unsigned char REAL_FACEVALUE_CT[] = {
    0x00, 0xdd, 0x4c, 0x4f, 0xce, 0xbb, 0xf7, 0x12, 0xbe, 0x83, 0xd8, 0x10,
    0x1d, 0x82, 0xd6, 0xeb, 0xf4, 0x0d, 0x6f, 0xf6, 0x8e, 0x57, 0x2d, 0xac,
    0xf9, 0x39, 0x41, 0x77, 0x23, 0xb9, 0xea, 0x20, 0x58, 0x60, 0xc3, 0x1e,
    0x40, 0x42, 0xc4, 0xaf, 0x23, 0x29, 0x60, 0xa5,
};

static const unsigned char EL_STORAGE_BUDGET_CT[] = {
    0x47, 0x94,
};

static const unsigned char EL_UPLOAD_WIFI_CT[] = {
    0x99, 0x32, 0x28, 0x34,
};

static const unsigned char EL_THUMB_CACHE_CT[] = {
    0x86, 0x65, 0xdd,
};

static const unsigned char EL_SYNC_ACCOUNT_CT[] = {
    0xa7, 0x9b, 0x5d, 0x44, 0x71, 0x11, 0xa4, 0x4c, 0x8b, 0x6c, 0x6b, 0xa8,
    0xfc, 0xaf, 0xea, 0x81, 0x8a, 0x0f, 0x14, 0x76, 0xa5, 0xd7, 0x11, 0xd5,
    0x1c, 0x9a, 0x5a, 0x20, 0x82, 0x2d,
};

static const unsigned char EL_API_BASE_CT[] = {
    0x81, 0xb9, 0xff, 0x88, 0xe1, 0x61, 0x5c, 0x94, 0xf4, 0x7b, 0x55, 0x50,
    0xa4, 0xbc, 0x91, 0x3b, 0xbe, 0x60, 0x09, 0xd7, 0xa8, 0xdf, 0xf6, 0x2e,
    0x2f, 0xfc, 0xfe, 0x0a,
};

static const unsigned char LL_PRE_1_CT[] = {
    0xa8, 0xff, 0x6a, 0xb2, 0x7b, 0x20, 0x46, 0x22, 0x8e, 0x98, 0x59, 0xad,
    0x70, 0xb3, 0x4a, 0x30, 0xc0, 0x3c, 0x6b, 0x8b, 0x44, 0x21, 0xaf, 0x77,
    0xd8, 0x38, 0x6c, 0x40, 0x52, 0x1b, 0xc0, 0xdc,
};

static const unsigned char LL_PRE_2_CT[] = {
    0x58, 0x4c, 0xb5, 0xa1, 0xdf, 0x9f, 0xfb, 0xcc, 0xe6, 0x79, 0x99, 0xa0,
    0xe8, 0x62, 0xb1, 0x2d, 0x5d, 0x1c, 0xf9, 0xd4, 0xab, 0x85, 0x82, 0xd2,
    0x33,
};

static const unsigned char LL_PRE_3_CT[] = {
    0x0a, 0xc7, 0x09, 0x8c, 0x27, 0x9b, 0x0a, 0xf4, 0xe1, 0x0c, 0x8d, 0xba,
    0x44, 0xa7, 0xc7, 0xc9, 0x0f, 0x97, 0x8b, 0x59, 0x0d, 0x6d, 0xa4, 0xc1,
    0xba, 0x9d, 0xb7, 0x2f, 0x2a, 0x6e, 0x00, 0xc0, 0x56, 0x1c, 0xb4,
};

static const unsigned char LL_PRE_4_CT[] = {
    0x4c, 0x62, 0xb6, 0x02, 0x02, 0xb5, 0x81, 0x01, 0x9c, 0x42, 0x45, 0x05,
    0x0e, 0x77, 0xa7, 0x0c, 0x84, 0x38, 0xd4, 0x31, 0x3d, 0xe3, 0x46, 0x08,
    0x58, 0xca,
};

static const unsigned char LL_PRE_5_CT[] = {
    0xff, 0x08, 0x2a, 0x79, 0xab, 0xa6, 0x41, 0xc6, 0x34, 0xe8, 0xfb, 0xcc,
    0xa1, 0x63, 0x5e, 0x51, 0x74, 0x9a, 0x1b, 0xde, 0xf8, 0xeb, 0x67, 0xda,
    0x24, 0x08, 0xac, 0xc9, 0xa2, 0xaf,
};

static const unsigned char LL_EPI_1_CT[] = {
    0x90, 0xca, 0xd1, 0xf9, 0x33, 0xd2, 0xaf, 0xfc, 0x4a, 0x3b, 0x2d, 0xd8,
    0x17, 0x58, 0x10, 0x80, 0x4f, 0xcb, 0xfd, 0x5f, 0x6d, 0xd3, 0x1f, 0xa1,
    0xaa, 0x70,
};

static const unsigned char LL_EPI_2_CT[] = {
    0xe0, 0x10, 0x23, 0x5b, 0x9d, 0xcc, 0xc2, 0xfc, 0x52, 0xa6, 0x51, 0x73,
    0xbd, 0xcf, 0x59, 0xfa, 0xc4, 0x32, 0xee, 0x59, 0x1c, 0xe8, 0xda, 0x12,
    0x5b, 0xfc, 0xc9,
};

static const unsigned char LL_EPI_3_CT[] = {
    0xdf, 0x39, 0xfb, 0xae, 0x07, 0x45, 0x6a, 0xb1, 0x98, 0xf5, 0x8e, 0x58,
    0x6e, 0x3f, 0xe8, 0x28, 0x36, 0x2a, 0xb5, 0x86, 0x48, 0x39, 0xb9, 0x17,
    0x88,
};

static const unsigned char CHANNEL_CT[] = {
    0x23, 0x9c, 0x66, 0x4c, 0x9f, 0xef, 0xd1, 0x54, 0xaf, 0xb5, 0x81, 0x97,
    0x3e, 0x66, 0xcd, 0xae, 0x52, 0x24, 0x69, 0x34, 0x1c, 0x50, 0x49, 0x6a,
    0x68, 0xfe, 0x43, 0x6b, 0xad, 0x4c, 0x4c, 0x62,
};

static const unsigned char IT_BUILD_ID_CT[] = {
    0x34, 0xa1, 0xc4, 0x29, 0x56, 0xbb, 0x4a, 0x84, 0xb9, 0x4c, 0x81, 0x0c,
    0x23, 0x64,
};

static const unsigned char IT_ENV_CT[] = {
    0xb5, 0x9a, 0xa8, 0x59, 0x88, 0x67, 0x60,
};

static const unsigned char IT_DEVICE_LABEL_CT[] = {
    0x87, 0x24, 0xaa, 0xd9, 0xaf, 0x4f, 0x0d, 0x4c, 0xa3, 0x36, 0x82, 0x7b,
};

static const unsigned char IT_SESSION_OWNER_CT[] = {
    0x71, 0x3d, 0x9b, 0x0d, 0x23, 0xa1, 0x27, 0x2b, 0x9a, 0x08, 0x58, 0xcd,
    0xd1, 0xf8, 0x90, 0xf3, 0xd2, 0x41, 0x65, 0x55, 0xde, 0x43, 0xfb, 0x0c,
};

typedef struct {
    int slot;
    const unsigned char *ct;
    size_t len;
    uint32_t nonce;
} SecureSlot;

static const SecureSlot SLOTS[] = {
    { 0x4b, REAL_LOOSELIPS_CT, sizeof(REAL_LOOSELIPS_CT), 0x9e250d03u },
    { 0x18, REAL_EVIDENCELOCKER_CT, sizeof(REAL_EVIDENCELOCKER_CT), 0xecefe37bu },
    { 0x63, REAL_FRONTDOOR_CT, sizeof(REAL_FRONTDOOR_CT), 0x888417a5u },
    { 0x2f, REAL_INTERNALTOOLS_CT, sizeof(REAL_INTERNALTOOLS_CT), 0xb5bab1cdu },
    { 0x76, REAL_FACEVALUE_CT, sizeof(REAL_FACEVALUE_CT), 0x5da83cffu },
    { 0x07, EL_STORAGE_BUDGET_CT, sizeof(EL_STORAGE_BUDGET_CT), 0x922badb0u },
    { 0x1d, EL_UPLOAD_WIFI_CT, sizeof(EL_UPLOAD_WIFI_CT), 0x95f628f2u },
    { 0x39, EL_THUMB_CACHE_CT, sizeof(EL_THUMB_CACHE_CT), 0xbb5d75b8u },
    { 0x52, EL_SYNC_ACCOUNT_CT, sizeof(EL_SYNC_ACCOUNT_CT), 0x2a6a7b5fu },
    { 0x6a, EL_API_BASE_CT, sizeof(EL_API_BASE_CT), 0xc6737b8bu },
    { 0x03, LL_PRE_1_CT, sizeof(LL_PRE_1_CT), 0xd30a286eu },
    { 0x0f, LL_PRE_2_CT, sizeof(LL_PRE_2_CT), 0x5531ae6du },
    { 0x22, LL_PRE_3_CT, sizeof(LL_PRE_3_CT), 0x623a7a75u },
    { 0x36, LL_PRE_4_CT, sizeof(LL_PRE_4_CT), 0xa28718e5u },
    { 0x45, LL_PRE_5_CT, sizeof(LL_PRE_5_CT), 0xca2410fdu },
    { 0x58, LL_EPI_1_CT, sizeof(LL_EPI_1_CT), 0x5c1ed35fu },
    { 0x69, LL_EPI_2_CT, sizeof(LL_EPI_2_CT), 0xebf644bbu },
    { 0x74, LL_EPI_3_CT, sizeof(LL_EPI_3_CT), 0xfee29f53u },
    { 0x0b, IT_BUILD_ID_CT, sizeof(IT_BUILD_ID_CT), 0x4ec10fc6u },
    { 0x1a, IT_ENV_CT, sizeof(IT_ENV_CT), 0x643cb56du },
    { 0x41, IT_DEVICE_LABEL_CT, sizeof(IT_DEVICE_LABEL_CT), 0xfe03e76fu },
    { 0x5d, IT_SESSION_OWNER_CT, sizeof(IT_SESSION_OWNER_CT), 0xb2767375u },
};

#define SECURESTORE_MAX_LEN 96

#define CHANNEL_NONCE 0x7a3e11c4u
#define DIGEST_LEN 32
#define BLOCK_LEN 64
#define MESSAGE_MAX 1024

static const uint32_t ROUND_K[64] = {
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
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
};

#define ROR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void compress_block(uint32_t h[8], const unsigned char block[BLOCK_LEN]) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, hh, t1, t2, s0, s1;
    int i;

    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) | ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) | (uint32_t)block[i * 4 + 3];
    }
    for (i = 16; i < 64; i++) {
        s0 = ROR32(w[i - 15], 7) ^ ROR32(w[i - 15], 18) ^ (w[i - 15] >> 3);
        s1 = ROR32(w[i - 2], 17) ^ ROR32(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    a = h[0]; b = h[1]; c = h[2]; d = h[3];
    e = h[4]; f = h[5]; g = h[6]; hh = h[7];

    for (i = 0; i < 64; i++) {
        s1 = ROR32(e, 6) ^ ROR32(e, 11) ^ ROR32(e, 25);
        t1 = hh + s1 + ((e & f) ^ ((~e) & g)) + ROUND_K[i] + w[i];
        s0 = ROR32(a, 2) ^ ROR32(a, 13) ^ ROR32(a, 22);
        t2 = s0 + ((a & b) ^ (a & c) ^ (b & c));
        hh = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;

    memset(w, 0, sizeof(w));
}

static void digest_bytes(const unsigned char *msg, size_t len, unsigned char out[DIGEST_LEN]) {
    uint32_t h[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u,
    };
    unsigned char tail[BLOCK_LEN * 2];
    uint64_t bits = (uint64_t)len * 8u;
    size_t full = len / BLOCK_LEN;
    size_t rest = len % BLOCK_LEN;
    size_t tail_len, i;

    for (i = 0; i < full; i++) {
        compress_block(h, msg + i * BLOCK_LEN);
    }

    memset(tail, 0, sizeof(tail));
    if (rest > 0) {
        memcpy(tail, msg + full * BLOCK_LEN, rest);
    }
    tail[rest] = 0x80;
    tail_len = (rest < 56) ? BLOCK_LEN : BLOCK_LEN * 2;

    for (i = 0; i < 8; i++) {
        tail[tail_len - 1 - i] = (unsigned char)(bits >> (8 * i));
    }
    for (i = 0; i < tail_len / BLOCK_LEN; i++) {
        compress_block(h, tail + i * BLOCK_LEN);
    }
    for (i = 0; i < 8; i++) {
        out[i * 4] = (unsigned char)(h[i] >> 24);
        out[i * 4 + 1] = (unsigned char)(h[i] >> 16);
        out[i * 4 + 2] = (unsigned char)(h[i] >> 8);
        out[i * 4 + 3] = (unsigned char)h[i];
    }

    memset(tail, 0, sizeof(tail));
    memset(h, 0, sizeof(h));
}

static int keyed_digest(const unsigned char *key, size_t key_len,
                        const unsigned char *msg, size_t msg_len,
                        unsigned char out[DIGEST_LEN]) {
    unsigned char k[BLOCK_LEN];
    unsigned char outer[BLOCK_LEN + DIGEST_LEN];
    unsigned char inner[BLOCK_LEN + MESSAGE_MAX];
    size_t i;

    if (msg_len > MESSAGE_MAX) {
        return -1;
    }

    memset(k, 0, sizeof(k));
    if (key_len > BLOCK_LEN) {
        digest_bytes(key, key_len, k);
    } else {
        memcpy(k, key, key_len);
    }

    for (i = 0; i < BLOCK_LEN; i++) {
        inner[i] = k[i] ^ 0x36;
    }
    if (msg_len > 0) {
        memcpy(inner + BLOCK_LEN, msg, msg_len);
    }
    digest_bytes(inner, BLOCK_LEN + msg_len, outer + BLOCK_LEN);

    for (i = 0; i < BLOCK_LEN; i++) {
        outer[i] = k[i] ^ 0x5c;
    }
    digest_bytes(outer, BLOCK_LEN + DIGEST_LEN, out);

    memset(k, 0, sizeof(k));
    memset(outer, 0, sizeof(outer));
    memset(inner, 0, sizeof(inner));
    return 0;
}

static size_t read_package_name(char *out, size_t cap) {
    FILE *f = fopen("/proc/self/cmdline", "rb");
    if (f == NULL) {
        return 0;
    }
    size_t n = fread(out, 1, cap - 1, f);
    fclose(f);
    out[n] = '\0';
    return strlen(out);
}

static uint32_t derive_seed(const char *name, size_t len, uint32_t nonce) {
    uint32_t seed = 0x811C9DC5u;
    for (size_t i = 0; i < len; i++) {
        seed ^= (unsigned char)name[i];
        seed *= 0x01000193u;
    }
    if (nonce != 0u) {
        for (int i = 0; i < 4; i++) {
            seed ^= (nonce >> (i * 8)) & 0xFFu;
            seed *= 0x01000193u;
        }
    }
    return seed;
}

static uint8_t next_key_byte(uint32_t *x) {
    *x ^= *x << 13;
    *x ^= *x >> 17;
    *x ^= *x << 5;
    return (uint8_t)(*x & 0xFFu);
}

static jstring unseal(JNIEnv *env, const unsigned char *ct, size_t len, uint32_t nonce) {
    char package[128];
    size_t plen = read_package_name(package, sizeof(package));
    uint8_t plain[SECURESTORE_MAX_LEN + 1];
    if (plen == 0 || len > SECURESTORE_MAX_LEN) {
        return (*env)->NewStringUTF(env, "");
    }
    uint32_t state = derive_seed(package, plen, nonce);
    for (size_t i = 0; i < len; i++) {
        plain[i] = ct[i] ^ next_key_byte(&state);
    }
    plain[len] = '\0';
    return (*env)->NewStringUTF(env, (const char *)plain);
}

#define CHANNEL_NONCE 0x7a3e11c4u
#define SIG_KEY_LEN 32
#define SIG_MODE_BODY 0
#define SIG_MODE_TOKEN 1
#define SIG_MODE_TOKEN_AUDIT 2


static const char AUDIT_TAIL[] = { '/', 'a', 'u', 'd', 'i', 't' };

static int load_sig_key(unsigned char out[SIG_KEY_LEN]) {
    char package[128];
    unsigned char fold[DIGEST_LEN];
    size_t plen = read_package_name(package, sizeof(package));
    size_t i;

    if (plen == 0) {
        return -1;
    }

    digest_bytes((const unsigned char *)package, plen, fold);

    uint32_t state = derive_seed(package, plen, CHANNEL_NONCE);
    for (i = 0; i < SIG_KEY_LEN; i++) {
        out[i] = CHANNEL_CT[i] ^ next_key_byte(&state) ^ fold[i];
    }

    memset(fold, 0, sizeof(fold));
    memset(package, 0, sizeof(package));
    return 0;
}

static jstring native_sign(JNIEnv *env, jobject obj, jint mode, jstring data) {
    (void)obj;

    unsigned char key[SIG_KEY_LEN];
    unsigned char message[MESSAGE_MAX];
    unsigned char mac[DIGEST_LEN];
    char hex[(DIGEST_LEN * 2) + 1];
    size_t msg_len, i;

    if (data == NULL || load_sig_key(key) != 0) {
        return (*env)->NewStringUTF(env, "");
    }

    const char *text = (*env)->GetStringUTFChars(env, data, NULL);
    if (text == NULL) {
        memset(key, 0, sizeof(key));
        return (*env)->NewStringUTF(env, "");
    }

    msg_len = strlen(text);
    if (mode == SIG_MODE_TOKEN_AUDIT) {
        if (msg_len + sizeof(AUDIT_TAIL) > MESSAGE_MAX) {
            (*env)->ReleaseStringUTFChars(env, data, text);
            memset(key, 0, sizeof(key));
            return (*env)->NewStringUTF(env, "");
        }
        memcpy(message, text, msg_len);
        memcpy(message + msg_len, AUDIT_TAIL, sizeof(AUDIT_TAIL));
        msg_len += sizeof(AUDIT_TAIL);
    } else {
        if (msg_len > MESSAGE_MAX) {
            (*env)->ReleaseStringUTFChars(env, data, text);
            memset(key, 0, sizeof(key));
            return (*env)->NewStringUTF(env, "");
        }
        memcpy(message, text, msg_len);
    }
    (*env)->ReleaseStringUTFChars(env, data, text);

    if (keyed_digest(key, SIG_KEY_LEN, message, msg_len, mac) != 0) {
        memset(key, 0, sizeof(key));
        memset(message, 0, sizeof(message));
        return (*env)->NewStringUTF(env, "");
    }

    for (i = 0; i < DIGEST_LEN; i++) {
        snprintf(&hex[i * 2], 3, "%02x", mac[i]);
    }
    hex[DIGEST_LEN * 2] = '\0';

    memset(key, 0, sizeof(key));
    memset(message, 0, sizeof(message));
    memset(mac, 0, sizeof(mac));

    return (*env)->NewStringUTF(env, hex);
}

static const SecureSlot *find_slot(int slot) {
    size_t count = sizeof(SLOTS) / sizeof(SLOTS[0]);
    for (size_t i = 0; i < count; i++) {
        if (SLOTS[i].slot == slot) {
            return &SLOTS[i];
        }
    }
    return NULL;
}

static jstring native_read(JNIEnv *env, jobject obj, jint slot) {
    (void)obj;
    const SecureSlot *entry = find_slot((int)slot);
    if (entry == NULL) {
        return (*env)->NewStringUTF(env, "");
    }
    return unseal(env, entry->ct, entry->len, entry->nonce);
}

static const JNINativeMethod methods[] = {
    {"read", "(I)Ljava/lang/String;", (void *)native_read},
    {"sign", "(ILjava/lang/String;)Ljava/lang/String;", (void *)native_sign},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;

    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass cls = (*env)->FindClass(env, "tn/securinets/ctf/challenge/SecureStore");
    if (cls == NULL) {
        return JNI_ERR;
    }
    if ((*env)->RegisterNatives(env, cls, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

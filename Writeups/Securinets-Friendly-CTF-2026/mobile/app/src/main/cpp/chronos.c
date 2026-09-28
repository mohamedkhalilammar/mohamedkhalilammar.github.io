#include <jni.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((visibility("hidden")))
void sx_entry(uint8_t *out, const uint8_t *in, int len, uint32_t nonce, int mode);

static const unsigned char SHIFT_CT[] = {
    0xac, 0xf0, 0x37, 0x16, 0x36, 0x0c, 0x15, 0x61, 0x65, 0x49, 0xdc, 0xe6,
    0xa4, 0x51, 0xde, 0x8e, 0x94, 0x61, 0xbd, 0x4e, 0x9c, 0x3e, 0x17, 0xff,
    0x52, 0xef, 0xb7, 0x84, 0xb8, 0x9c, 0x1a, 0x3b, 0x66, 0xef, 0x82, 0xaa,
    0x57, 0x60, 0x8a, 0x9b, 0xd7, 0x13, 0x1d,
};

static const unsigned char ROSTER_CT[] = {
    0x3f, 0x29, 0x69, 0xe8, 0x54, 0x7d, 0xfc, 0x0c, 0x82, 0x08, 0x4f, 0x4e,
    0xed, 0xfb, 0x5b, 0x93, 0x67, 0xb8, 0xed, 0x57, 0x30, 0xff, 0xd3, 0x34,
    0x3f, 0x2e, 0x80, 0x22, 0xba, 0x45, 0xcf, 0xca,
};

#define CHRONOS_NONCE 0x1f0a5c73u
#define ROSTER_NONCE 0x3d9e2b16u
#define ACC_LEN 32
#define KEY_LEN 32
#define DIGEST_LEN 32
#define BLOCK_LEN 64
#define MESSAGE_MAX 512
#define SHIFT_MAX 128
#define TARGET_MAX 64

static const unsigned char STREAM_TAG[] = { 0x73, 0x68, 0x66, 0x74 };

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
    FILE *f;
    size_t n;

#ifdef SX_DESIGNER
    const char *forced = getenv("SX_DESIGNER_ID");
    if (forced != NULL && *forced != '\0') {
        n = strlen(forced);
        if (n > cap - 1) {
            n = cap - 1;
        }
        memcpy(out, forced, n);
        out[n] = '\0';
        return n;
    }
#endif

    f = fopen("/proc/self/cmdline", "rb");
    if (f == NULL) {
        return 0;
    }
    n = fread(out, 1, cap - 1, f);
    fclose(f);
    out[n] = '\0';
    return strlen(out);
}

static uint32_t derive_seed(const char *name, size_t len, uint32_t nonce) {
    uint32_t seed = 0x811C9DC5u;
    size_t i;
    int j;

    for (i = 0; i < len; i++) {
        seed ^= (unsigned char)name[i];
        seed *= 0x01000193u;
    }
    for (j = 0; j < 4; j++) {
        seed ^= (nonce >> (j * 8)) & 0xFFu;
        seed *= 0x01000193u;
    }
    return seed;
}

static uint8_t next_key_byte(uint32_t *x) {
    *x ^= *x << 13;
    *x ^= *x >> 17;
    *x ^= *x << 5;
    return (uint8_t)(*x & 0xFFu);
}

static int load_roster_key(unsigned char out[KEY_LEN]) {
    char package[128];
    unsigned char fold[DIGEST_LEN];
    size_t plen = read_package_name(package, sizeof(package));
    uint32_t state;
    size_t i;

    if (plen == 0 || sizeof(ROSTER_CT) < KEY_LEN) {
        return -1;
    }

    digest_bytes((const unsigned char *)package, plen, fold);
    state = derive_seed(package, plen, ROSTER_NONCE);
    for (i = 0; i < KEY_LEN; i++) {
        out[i] = ROSTER_CT[i] ^ next_key_byte(&state) ^ fold[i];
    }

    memset(fold, 0, sizeof(fold));
    memset(package, 0, sizeof(package));
    return 0;
}

static unsigned char acc[ACC_LEN];

static void expand(const unsigned char seed[ACC_LEN], unsigned char *out, size_t len) {
    unsigned char msg[sizeof(STREAM_TAG) + 1];
    unsigned char block[DIGEST_LEN];
    size_t done = 0;
    unsigned char counter = 0;

    memcpy(msg, STREAM_TAG, sizeof(STREAM_TAG));
    while (done < len) {
        size_t take = len - done;
        msg[sizeof(STREAM_TAG)] = counter;
        if (keyed_digest(seed, ACC_LEN, msg, sizeof(msg), block) != 0) {
            memset(out, 0, len);
            return;
        }
        if (take > DIGEST_LEN) {
            take = DIGEST_LEN;
        }
        memcpy(out + done, block, take);
        done += take;
        counter++;
    }

    memset(block, 0, sizeof(block));
}

__attribute__((visibility("hidden")))
void chronos_reset(void) {
    memset(acc, 0, sizeof(acc));
}

__attribute__((visibility("hidden")))
void chronos_fold(const char *target, size_t len, int slot) {
    unsigned char key[KEY_LEN];
    unsigned char message[TARGET_MAX + 1];
    unsigned char tag[DIGEST_LEN];
    unsigned char joined[ACC_LEN + DIGEST_LEN];

    if (target == NULL || load_roster_key(key) != 0) {
        return;
    }
    if (len > TARGET_MAX) {
        len = TARGET_MAX;
    }
    memcpy(message, target, len);
    message[len] = (unsigned char)(slot & 0xFF);

    if (keyed_digest(key, KEY_LEN, message, len + 1, tag) == 0) {
        memcpy(joined, acc, ACC_LEN);
        memcpy(joined + ACC_LEN, tag, DIGEST_LEN);
        digest_bytes(joined, sizeof(joined), acc);
    }

    memset(key, 0, sizeof(key));
    memset(message, 0, sizeof(message));
    memset(tag, 0, sizeof(tag));
    memset(joined, 0, sizeof(joined));
}

__attribute__((visibility("hidden")))
size_t chronos_open(char *out, size_t cap) {
    unsigned char stream[SHIFT_MAX];
    unsigned char sealed[SHIFT_MAX];
    unsigned char plain[SHIFT_MAX + 1];
    size_t len = sizeof(SHIFT_CT);
    size_t i;

    if (len > SHIFT_MAX || cap < len + 1) {
        if (cap > 0) {
            out[0] = '\0';
        }
        return 0;
    }

    expand(acc, stream, len);
    for (i = 0; i < len; i++) {
        sealed[i] = SHIFT_CT[i] ^ stream[i];
    }

    memset(plain, 0, sizeof(plain));
    sx_entry(plain, sealed, (int)len, CHRONOS_NONCE, 0);

    for (i = 0; i < len; i++) {
        if (plain[i] < 0x20 || plain[i] > 0x7e) {
            plain[i] = '.';
        }
    }
    plain[len] = '\0';
    memcpy(out, plain, len + 1);

    memset(stream, 0, sizeof(stream));
    memset(sealed, 0, sizeof(sealed));
    memset(plain, 0, sizeof(plain));
    return len;
}

static void native_reset(JNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    chronos_reset();
}

static void native_fold(JNIEnv *env, jobject obj, jstring target, jint index) {
    (void)obj;

    const char *text;

    if (target == NULL) {
        return;
    }
    text = (*env)->GetStringUTFChars(env, target, NULL);
    if (text == NULL) {
        return;
    }
    chronos_fold(text, strlen(text), (int)index);
    (*env)->ReleaseStringUTFChars(env, target, text);
}

static jstring native_open(JNIEnv *env, jobject obj) {
    (void)obj;

    char out[SHIFT_MAX + 1];

    memset(out, 0, sizeof(out));
    chronos_open(out, sizeof(out));
    return (*env)->NewStringUTF(env, out);
}

static const JNINativeMethod methods[] = {
    {"reset", "()V", (void *)native_reset},
    {"fold", "(Ljava/lang/String;I)V", (void *)native_fold},
    {"open", "()Ljava/lang/String;", (void *)native_open},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;

    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass cls = (*env)->FindClass(env, "tn/securinets/ctf/challenges/finalcountdown/Chronos");
    if (cls == NULL) {
        return JNI_ERR;
    }
    if ((*env)->RegisterNatives(env, cls, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

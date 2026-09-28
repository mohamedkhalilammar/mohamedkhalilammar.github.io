#include <jni.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

__attribute__((visibility("hidden")))
void sx_entry(uint8_t *out, const uint8_t *in, int len, uint32_t nonce, int mode);

static const unsigned char SEALED_NOTE[] = {
    0x77, 0x51, 0xcc, 0xfa, 0xbd, 0x6c, 0xa8, 0xc8, 0x01, 0x7f, 0xc9, 0xd6,
    0x7a, 0xa0, 0x1c, 0x12, 0x3a, 0x3d, 0xfe, 0x10, 0x8b, 0x76, 0x3b, 0xd4,
    0xe6, 0x35, 0x2b, 0xdc, 0x6a, 0x9b, 0xa8, 0x55, 0xdd, 0xe7, 0x16,
};

#define SIRR_MAX_LEN 128
#define SIRR_MIN_LEN 4

static void sx_live_unwrap(uint8_t *buf, int len, unsigned int liveNonce) {
    uint32_t x = liveNonce ? (uint32_t)liveNonce : 1u;
    for (int i = 0; i < len; i++) {
        x ^= (x << 13);
        x ^= x >> 17;
        x ^= (x << 5);
        buf[i] ^= (uint8_t)(x & 0xFF);
    }
}

__attribute__((visibility("default")))
const char *sirr_unseal(const unsigned char *blob, int len, unsigned int liveNonce) {
    static uint8_t plain[SIRR_MAX_LEN + 1];
    static uint8_t stage[SIRR_MAX_LEN];

    if (blob == NULL || len < SIRR_MIN_LEN || len > SIRR_MAX_LEN || liveNonce == 0u) {
        return "";
    }

    memcpy(stage, blob, (size_t)len);
    sx_live_unwrap(stage, len, liveNonce);

    memset(plain, 0, sizeof(plain));
    sx_entry(plain, stage, len, 0u, 0);
    plain[len] = '\0';

    return (const char *)plain;
}

#ifdef SX_DESIGNER
#define SIRR_DESIGNER_LIVE_NONCE 0x7c3d9a21u

__attribute__((visibility("default")))
const char *sirr_designer_open(void) {
    return sirr_unseal(SEALED_NOTE, (int)sizeof(SEALED_NOTE), SIRR_DESIGNER_LIVE_NONCE);
}

JNIEXPORT jstring JNICALL
Java_tn_securinets_ctf_challenges_nobodycalled_SirrCrypto_designerOpen(JNIEnv *env, jobject obj) {
    (void)obj;
    return (*env)->NewStringUTF(env, sirr_designer_open());
}
#endif

JNIEXPORT jstring JNICALL
Java_tn_securinets_ctf_challenges_nobodycalled_SirrCrypto_seal(JNIEnv *env, jobject obj, jstring note) {
    (void)obj;

    const char *text = (*env)->GetStringUTFChars(env, note, NULL);
    if (text == NULL) {
        return (*env)->NewStringUTF(env, "");
    }

    uint8_t body[SIRR_MAX_LEN];
    size_t len = strlen(text);
    if (len > SIRR_MAX_LEN) {
        len = SIRR_MAX_LEN;
    }
    memcpy(body, text, len);
    (*env)->ReleaseStringUTFChars(env, note, text);

    while (len < SIRR_MIN_LEN) {
        body[len++] = ' ';
    }

    uint32_t nonce = (uint32_t)(uintptr_t)&len ^ (uint32_t)clock();
    if (nonce == 0u) {
        nonce = 1u;
    }

    uint8_t sealed[SIRR_MAX_LEN + 1];
    memset(sealed, 0, sizeof(sealed));
    sx_entry(sealed, body, (int)len, nonce, 1);

    char hex[((SIRR_MAX_LEN + 4) * 2) + 1];
    snprintf(hex, 9, "%08x", nonce);
    for (size_t i = 0; i < len; i++) {
        snprintf(&hex[8 + (i * 2)], 3, "%02x", sealed[i]);
    }
    hex[8 + (len * 2)] = '\0';

    return (*env)->NewStringUTF(env, hex);
}

JNIEXPORT jstring JNICALL
Java_tn_securinets_ctf_challenges_nobodycalled_SirrCrypto_archivedBlobHex(JNIEnv *env, jobject obj) {
    (void)obj;

    char hex[(sizeof(SEALED_NOTE) * 2) + 1];
    for (size_t i = 0; i < sizeof(SEALED_NOTE); i++) {
        snprintf(&hex[i * 2], 3, "%02x", SEALED_NOTE[i]);
    }
    hex[sizeof(SEALED_NOTE) * 2] = '\0';

    return (*env)->NewStringUTF(env, hex);
}

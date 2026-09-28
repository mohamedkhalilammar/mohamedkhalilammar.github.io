#include <jni.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

__attribute__((visibility("hidden")))
void sx_entry(uint8_t *out, const uint8_t *in, int len, uint32_t nonce, int mode);

#define LICENCE_STEP 1u

static const unsigned char LICENCE_BODY[] = {
    0x03, 0x60, 0x89, 0xea, 0xcb, 0x03, 0xc7, 0x10, 0xa5, 0x28, 0x44, 0xe8,
    0x62, 0xae, 0x9c, 0xfa, 0x62, 0xa9, 0x0a, 0xfc, 0x99, 0xeb, 0x62, 0x20,
    0x4c, 0xbb, 0x2b, 0x06, 0x9a, 0x9c, 0xdf, 0xeb, 0x1d, 0x29, 0xfc,
};

static void legible(uint8_t *buf, int len) {
    for (int i = 0; i < len; i++) {
        if (buf[i] < 0x20u || buf[i] > 0x7Eu) {
            buf[i] = (uint8_t)(0x21u + (buf[i] % 0x5Du));
        }
    }
}

static jstring native_compute_flag(JNIEnv *env, jobject obj) {
    (void)obj;

    uint8_t body[sizeof(LICENCE_BODY) + 1];
    int len = (int)sizeof(LICENCE_BODY);

    memset(body, 0, sizeof(body));
    sx_entry(body, LICENCE_BODY, len, LICENCE_STEP, 0);
    legible(body, len);
    body[len] = '\0';

    return (*env)->NewStringUTF(env, (const char *)body);
}

static const JNINativeMethod methods[] = {
    {"nativeComputeFlag", "()Ljava/lang/String;", (void *)native_compute_flag},
};

static int register_methods(JNIEnv *env, const char *class_name, const JNINativeMethod *methods, int num_methods) {
    jclass cls = (*env)->FindClass(env, class_name);
    if (cls == NULL) {
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, cls, methods, num_methods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;

    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    if (!register_methods(env, "tn/securinets/ctf/challenges/license/LicenseCheck",
                          methods, sizeof(methods) / sizeof(methods[0]))) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

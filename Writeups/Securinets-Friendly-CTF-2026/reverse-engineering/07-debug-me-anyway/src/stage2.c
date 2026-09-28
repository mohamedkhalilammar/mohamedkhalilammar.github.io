#include "payload.h"
#include "stage2.h"

__attribute__((section("stage2d"), used)) static unsigned char CORE[] = SEALED_BYTES;

__attribute__((section("stage2"), used)) void payload_entry(char *out, unsigned int cap)
{
    unsigned int x = SEALED_SEED;
    unsigned int i;

    for (i = 0; i < sizeof CORE && i + 1u < cap; i++) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        out[i] = (char)(CORE[i] ^ (x & 0xffu));
    }
    out[i] = '\0';
}

#include <stdio.h>
#include <string.h>

#include "notice.h"
#include "payload.h"

#define LICENCE_PATH "/etc/meridian/service.lic"

static volatile unsigned int VAULT_SEED = 0xc1d70e39u;

static void __attribute__((noinline)) unwrap(unsigned int x, const unsigned char *in,
                                             unsigned int n, char *out)
{
    for (unsigned int i = 0; i < n; i++) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        out[i] = (char)(in[i] ^ (x & 0xffu));
    }
    out[n] = '\0';
}

static int __attribute__((noinline)) licensed(void)
{
    unsigned char seal[32];
    unsigned int h = 0x811c9dc5u;
    size_t n;
    FILE *f;

    f = fopen(LICENCE_PATH, "rb");
    if (f == NULL) {
        puts("no licence at " LICENCE_PATH);
        return 0;
    }

    n = fread(seal, 1, sizeof seal, f);
    fclose(f);

    if (n != 16) {
        puts("licence rejected: seal must be 16 bytes");
        return 0;
    }

    for (unsigned int i = 0; i < 16u; i++) {
        h ^= seal[i];
        h *= 0x01000193u;
    }

    if (h != 0x3b6f21a4u) {
        puts("licence rejected");
        return 0;
    }

    return 1;
}

int main(void)
{
    char note[192];


    if (!licensed()) {
        puts("locked -- no licence exists for this unit");
        return 1;
    }

    unwrap(VAULT_SEED, SEALED, SEALED_LEN, note);
    puts(note);
    return 0;
}

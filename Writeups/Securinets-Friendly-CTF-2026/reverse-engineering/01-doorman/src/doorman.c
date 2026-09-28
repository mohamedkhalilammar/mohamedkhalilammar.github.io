#include <stdio.h>
#include <string.h>

#include "notice.h"
#include "payload.h"

static volatile unsigned int GATE_SEED = 0x5f3a91c7u;

static unsigned int digest(const char *k)
{
    unsigned int h = 0x811c9dc5u;
    while (*k) {
        h ^= (unsigned char)*k++;
        h *= 0x01000193u;
    }
    return h ? h : 0x9e3779b9u;
}

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

int main(void)
{
    char badge[64];
    char entered[128];
    char posted[192];

    unwrap(GATE_SEED, BADGE, BADGE_LEN, badge);

    fputs("badge code: ", stdout);
    fflush(stdout);

    if (fgets(entered, sizeof entered, stdin) == NULL) {
        puts("no input");
        return 1;
    }
    entered[strcspn(entered, "\r\n")] = '\0';

    if (strcmp(entered, badge) != 0) {
        puts("denied");
        return 1;
    }

    unwrap(digest(badge), SEALED, SEALED_LEN, posted);
    puts(posted);
    return 0;
}

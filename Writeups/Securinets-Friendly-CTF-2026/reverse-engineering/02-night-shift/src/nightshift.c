#include <stdio.h>
#include <string.h>

#include "notice.h"
#include "payload.h"

static unsigned char spin(unsigned char v, unsigned int n)
{
    n &= 7u;
    return (unsigned char)((v << n) | (v >> ((8u - n) & 7u)));
}

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

static int __attribute__((noinline)) accepts(const char *key)
{
    unsigned int n = (unsigned int)strlen(key);
    if (n != ROSTER_LEN)
        return 0;

    for (unsigned int i = 0; i < n; i++) {
        unsigned char v = (unsigned char)key[i];
        v ^= (unsigned char)(0x5au + 7u * i);
        v = spin(v, i);
        if (v != ROSTER[i])
            return 0;
    }
    return 1;
}

int main(void)
{
    char entered[128];
    char posted[192];

    fputs("rotation key: ", stdout);
    fflush(stdout);

    if (fgets(entered, sizeof entered, stdin) == NULL) {
        puts("no input");
        return 1;
    }
    entered[strcspn(entered, "\r\n")] = '\0';

    if (!accepts(entered)) {
        puts("denied");
        return 1;
    }

    unwrap(digest(entered), SEALED, SEALED_LEN, posted);
    puts(posted);
    return 0;
}

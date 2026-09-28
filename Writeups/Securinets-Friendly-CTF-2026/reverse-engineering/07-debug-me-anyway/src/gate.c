#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <unistd.h>

#include "decoys.h"
#include "notice.h"
#include "report.h"
#include "stage2.h"

extern char __start_stage2[], __stop_stage2[];
extern char __start_stage2d[], __stop_stage2d[];

volatile unsigned int CORE_SUM = 0xdeadbeefu;
volatile unsigned int TEXT_OFF = 0xcafef00du;
volatile unsigned int TEXT_LEN = 0xfeedfaceu;
volatile unsigned int PICK_MASK = 0xabadcafeu;

typedef void (*builder)(char *, unsigned int);

static char HOLD[64];
volatile builder TABLE[5];

static int __attribute__((noinline)) leashed(void)
{
    return ptrace(PTRACE_TRACEME, 0, 0, 0) == -1;
}

static unsigned int mix(unsigned int h, const unsigned char *p, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++) {
        h ^= p[i];
        h *= 0x01000193u;
        h ^= h >> 13;
        h = (h << 5) | (h >> 27);
    }
    return h;
}

static unsigned int temper(unsigned int h)
{
    for (int r = 0; r < 64; r++) {
        h ^= h << 7;
        h *= 0x9e3779b1u;
        h ^= h >> 11;
    }
    return h ? h : 0x9e3779b9u;
}

static unsigned int selfsum(void)
{
    unsigned char buf[1024];
    unsigned int h = 0x811c9dc5u;
    unsigned long left = TEXT_LEN;
    FILE *f;

    if (left == 0 || left > (1u << 22))
        return 0;
    f = fopen("/proc/self/exe", "rb");
    if (f == NULL)
        return 0;
    if (fseek(f, (long)TEXT_OFF, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    while (left > 0) {
        unsigned long want = left < sizeof buf ? left : sizeof buf;
        size_t got = fread(buf, 1, want, f);

        if (got != want) {
            fclose(f);
            return 0;
        }
        h = mix(h, buf, got);
        left -= want;
    }
    fclose(f);
    return temper(h);
}

static void peel(unsigned char *p, unsigned long n, unsigned int x)
{
    for (unsigned long i = 0; i < n; i++) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        p[i] ^= (unsigned char)(x & 0xffu);
    }
}

static int unpack(unsigned int key)
{
    unsigned char *code = (unsigned char *)__start_stage2;
    unsigned long clen = (unsigned long)(__stop_stage2 - __start_stage2);
    unsigned char *data = (unsigned char *)__start_stage2d;
    unsigned long dlen = (unsigned long)(__stop_stage2d - __start_stage2d);
    unsigned long page = (unsigned long)sysconf(_SC_PAGESIZE);
    unsigned long lo = (unsigned long)code & ~(page - 1);
    unsigned long hi = ((unsigned long)code + clen + page - 1) & ~(page - 1);

    if (key == 0 || clen == 0 || dlen == 0)
        return 0;
    if (mprotect((void *)lo, hi - lo, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
        return 0;
    peel(code, clen, key);
    peel(data, dlen, key ^ 0x5bd1e995u);
    if (temper(mix(mix(0x811c9dc5u, code, clen), data, dlen)) != CORE_SUM) {
        mprotect((void *)lo, hi - lo, PROT_READ | PROT_EXEC);
        return 0;
    }
    return mprotect((void *)lo, hi - lo, PROT_READ | PROT_EXEC) == 0;
}

char *reveal(void)
{
    ((builder)(void *)__start_stage2)(HOLD, (unsigned int)sizeof HOLD);
    return HOLD;
}

int main(int argc, char **argv)
{
    unsigned int key;

    if (argc == 2 && strcmp(argv[1], "--notice") == 0) {
        fputs(AI_NOTICE, stdout);
        return 0;
    }

    puts("");
    puts("  GATE 2.4");
    report_print();
    puts("");

    if (leashed()) {
        puts("  ptrace(PTRACE_TRACEME) returned -1, which means a debugger already");
        puts("  has this process. refusing to continue.");
        puts("");
        return 1;
    }

    key = selfsum();
    if (!unpack(key)) {
        puts("  the builder table did not load.");
        puts("");
        return 1;
    }

    TABLE[0] = form_a;
    TABLE[1] = form_b;
    TABLE[2] = form_c;
    TABLE[3] = form_d;
    TABLE[4] = (builder)(void *)__start_stage2;

    printf("  builder %u of 5 is loaded at %p and is not invoked.\n",
           (key ^ PICK_MASK) % 5u, (void *)TABLE[(key ^ PICK_MASK) % 5u]);
    puts("  nothing in this run will print the flag.");
    puts("");
    fflush(stdout);

    (void)leashed();

#ifdef DESIGNER
    printf("  [d] %s\n\n", reveal());
#endif
    return 0;
}

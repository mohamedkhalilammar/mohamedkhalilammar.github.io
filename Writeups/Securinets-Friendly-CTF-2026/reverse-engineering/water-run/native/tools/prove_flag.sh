#!/usr/bin/env bash
# Recovers the real flag from the shipped-logic .so for each variant by driving
# the actual C library through its intended solve path -- score to the gate,
# an out-of-band write to lift/velocity above the wall-clear threshold (the
# same poke tests/test_challenge.c uses to stand in for a Cheat Engine edit),
# at the variant's own struct offset -- beginner keeps lift at score+8, advanced
# deliberately does not put velocity there, so the offset is passed in per run,
# artifact pickup, then cg_receipt(). This proves the compiled logic itself
# produces the shipped flag; it does not touch secrets/flags.json or
# keys/*.flag as ground truth for anything except the final diff.
set -euo pipefail
cd "$(dirname "$0")/.."
HERE=$(pwd)

cat > /tmp/cg-prove.c <<'C'
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void cg_ctx;
int main(int argc, char **argv) {
    void *h;
    if (argc < 3) { fprintf(stderr, "usage: cg-prove <lib> <float-offset>\n"); return 1; }
    h = dlopen(argv[1], RTLD_NOW);
    if (!h) { fprintf(stderr, "dlopen: %s\n", dlerror()); return 1; }
    cg_ctx *(*create)(unsigned int) = dlsym(h, "cg_create");
    int (*phase_a)(cg_ctx*) = dlsym(h, "cg_phase_a");
    int (*phase_b)(cg_ctx*) = dlsym(h, "cg_phase_b");
    int (*receipt)(cg_ctx*, char*, size_t) = dlsym(h, "cg_receipt");
    if (!create || !phase_a || !phase_b || !receipt) { fprintf(stderr, "missing symbol\n"); return 1; }
    cg_ctx *c = create(1);
    int *score = (int *)c;
    float *lift = (float *)((unsigned char *)c + atoi(argv[2]));
    *score = 8000;
    *lift = 16.0f;
    if (phase_a(c) != 0) { fprintf(stderr, "phase_a failed\n"); return 1; }
    *score += 37;
    if (phase_b(c) != 0) { fprintf(stderr, "phase_b failed\n"); return 1; }
    char out[96] = {0};
    int rc = receipt(c, out, sizeof(out));
    printf("%s\n", out);
    return rc == 0 ? 0 : 1;
}
C
gcc -o /tmp/cg-prove /tmp/cg-prove.c -ldl

for v in beginner advanced; do
    CG_VARIANT="$v" ./build.sh >/tmp/cg-build-$v.log 2>&1 \
        || { echo "$v: build FAILED, see /tmp/cg-build-$v.log" >&2; exit 1; }
    off=8
    [ "$v" = advanced ] && off=16
    got=$(/tmp/cg-prove "$HERE/build/libcgchallenge.so" "$off")
    want=$(cat "$HERE/keys/game$( [ "$v" = advanced ] && echo -advanced ).flag")
    if [ "$got" = "$want" ]; then
        echo "$v: OK  $got"
    else
        echo "$v: MISMATCH -- got '$got' want '$want'" >&2
        exit 1
    fi
done

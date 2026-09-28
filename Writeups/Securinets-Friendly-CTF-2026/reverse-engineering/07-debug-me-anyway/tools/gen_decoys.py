#!/usr/bin/env python3
"""Emit four working-but-wrong unsealers plus their sealed blobs.

Ours, not the player's -- build tooling, so CLAUDE.md's no-comments rule does
not apply here.

Point: poison the cheap static attack. Scanning the file for a 32-bit immediate
that turns some byte run into `Securinets{...}` is about twenty lines of Python
and it used to find the real flag in seconds. With these in the binary that same
attack returns four complete, plausible, WRONG flags and no way to rank them --
the real seed and the real blob both live inside the encrypted `stage2` section,
so they are not reachable by that attack at all.

The decoys are real code on a real code path: main builds a table of five
builders and indexes it with a value derived from the program's own .text
checksum, so working out which entry actually runs means reimplementing the
mixer -- which is the same work as decrypting stage2. They are never the one
that runs, so at run time exactly one flag-shaped string exists in memory and
`search -t bytes Securinets{` has a single hit.

Fake flags are deliberately plausible and deliberately not insulting -- a player
who submits one gets "incorrect" from CTFd, which is the feedback.
"""

import argparse
import pathlib
import secrets

MASK = 0xFFFFFFFF

FAKES = [
    "Securinets{ptr4c3_1s_just_4n_1f_st4t3m3nt}",
    "Securinets{r34d_th3_r3g1st3r_n0t_th3_c0d3}",
    "Securinets{4nt1_d3bug_1s_4_sp33d_bump_0nly}",
    "Securinets{th3_l34sh_w4s_n3v3r_t13d_t1ght}",
]


def keystream(seed: int, n: int):
    x = seed & MASK
    for _ in range(n):
        x ^= (x << 13) & MASK
        x ^= x >> 17
        x ^= (x << 5) & MASK
        yield x & 0xFF


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    names = ("form_a", "form_b", "form_c", "form_d")
    lines = ["#ifndef DECOYS_H", "#define DECOYS_H", ""]

    for name, fake in zip(names, FAKES):
        plain = fake.encode()
        seed = secrets.randbits(32) or 0x9E3779B9
        sealed = bytes(p ^ k for p, k in zip(plain, keystream(seed, len(plain))))
        if sealed == plain:
            return 1
        rows = "\n".join(
            "    " + " ".join(f"0x{b:02x}," for b in sealed[i:i + 12])
            for i in range(0, len(sealed), 12)
        )
        lines += [
            f"static const unsigned char {name.upper()}_D[] = {{",
            rows,
            "};",
            "",
            f"static void __attribute__((noinline)) {name}(char *out, unsigned int cap)",
            "{",
            f"    unsigned int x = 0x{seed:08x}u;",
            "    unsigned int i;",
            "",
            f"    for (i = 0; i < sizeof {name.upper()}_D && i + 1u < cap; i++) {{",
            "        x ^= x << 13;",
            "        x ^= x >> 17;",
            "        x ^= x << 5;",
            f"        out[i] = (char)({name.upper()}_D[i] ^ (x & 0xffu));",
            "    }",
            "    out[i] = '\\0';",
            "}",
            "",
        ]

    lines += ["#endif", ""]
    pathlib.Path(args.out).write_text("\n".join(lines))
    print(f"   4 decoy builders + blobs -> {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

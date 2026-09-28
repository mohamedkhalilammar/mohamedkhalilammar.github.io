#!/usr/bin/env python3
"""The cheap static attack, run against our own artifact so the build fails if it
ever works again.

Ours, not the player's -- build tooling, exempt from the no-comments rule.

This is the twenty-line attack that used to recover the flag from this challenge
in seconds and needed no decompiler: collect every 32-bit immediate in the
disassembly, treat each as an xorshift32 seed, and slide the resulting keystream
over the whole file looking for a window that decrypts to `Securinets{`.

It is exactly what a competent player -- or an LLM handed the binary -- reaches
for first. build.sh runs it and requires that it finds NO real flag and at least
two decoys, which is the difference between "sealed" and "actually hard".

Exit 0 = the real flag was NOT recovered (good). Exit 1 = it was (bad).
"""

import argparse
import pathlib
import re
import subprocess
import sys

MASK = 0xFFFFFFFF


def keystream(seed: int, n: int):
    x = seed & MASK
    out = bytearray()
    for _ in range(n):
        x ^= (x << 13) & MASK
        x ^= x >> 17
        x ^= (x << 5) & MASK
        out.append(x & 0xFF)
    return bytes(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("binary")
    ap.add_argument("--real", required=True, help="the real flag, which must NOT be found")
    ap.add_argument("--min-decoys", type=int, default=2)
    args = ap.parse_args()

    path = pathlib.Path(args.binary)
    blob = path.read_bytes()
    dis = subprocess.run(["objdump", "-d", str(path)], capture_output=True,
                         text=True, check=True).stdout
    seeds = {int(m, 16) for m in re.findall(r"\$0x([0-9a-f]{5,8})\b", dis)}

    needle = b"Securinets{"
    found = []
    for seed in seeds:
        ks = keystream(seed, 64)
        first = needle[0] ^ ks[0]
        start = 0
        while True:
            off = blob.find(bytes([first]), start)
            if off < 0 or off + 64 > len(blob):
                break
            start = off + 1
            window = blob[off:off + 64]
            cand = bytes(b ^ k for b, k in zip(window, ks))
            if cand.startswith(needle):
                text = cand.split(b"\x00")[0].decode("latin-1")
                if "}" in text:
                    found.append((seed, off, text[:text.index("}") + 1]))

    print(f"   {len(seeds)} candidate seeds in the disassembly")
    for seed, off, text in found:
        tag = "REAL" if text == args.real else "decoy"
        print(f"   [{tag}] seed 0x{seed:08x} @ 0x{off:x} -> {text}")

    real = [f for f in found if f[2] == args.real]
    decoys = [f for f in found if f[2] != args.real]

    if real:
        print("   FAIL: the cheap static attack recovered the real flag", file=sys.stderr)
        return 1
    if len(decoys) < args.min_decoys:
        print(f"   FAIL: only {len(decoys)} decoys were reachable, wanted "
              f"{args.min_decoys} -- the attack is not being poisoned",
              file=sys.stderr)
        return 1
    print(f"   ok -- the attack yields {len(decoys)} wrong flags and no real one")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

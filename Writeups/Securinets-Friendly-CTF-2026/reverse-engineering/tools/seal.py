#!/usr/bin/env python3
"""Seal a flag under a key and emit a C header.

Ours, not the player's -- this file is build tooling, so the no-comments rule
does not apply here (CLAUDE.md exempts tools/).

The flag never exists as a contiguous string in any shipped artifact. The
binary carries only the sealed bytes; the plaintext is reconstructed at run
time, in a stack buffer, from a key the player has to earn. `strings` on the
build output must return nothing flag-shaped -- build.sh checks that.

Primitive is FNV-1a 32 for key -> seed, then an xorshift32 keystream XORed
over the payload. Deliberately the same xorshift32 variant already used by
secrets/gen_sirr_blob.py and the game track, so there is one primitive in the
project rather than five, and it is short enough that a beginner can follow
the C by eye.

    seal.py --key KEY --flag-file src/flag.txt --symbol SEALED --out src/payload.h
"""

import argparse
import pathlib
import sys

MASK = 0xFFFFFFFF


def seed_from_key(key: bytes) -> int:
    h = 0x811C9DC5
    for b in key:
        h ^= b
        h = (h * 0x01000193) & MASK
    return h or 0x9E3779B9


def keystream(seed: int, n: int):
    x = seed & MASK
    for _ in range(n):
        x ^= (x << 13) & MASK
        x ^= x >> 17
        x ^= (x << 5) & MASK
        yield x & 0xFF


def seal(key: bytes, plaintext: bytes) -> bytes:
    return bytes(p ^ k for p, k in zip(plaintext, keystream(seed_from_key(key), len(plaintext))))


def as_c_array(symbol: str, data: bytes) -> str:
    rows = []
    for i in range(0, len(data), 12):
        rows.append("    " + " ".join(f"0x{b:02x}," for b in data[i:i + 12]))
    body = "\n".join(rows)
    return (
        f"static const unsigned char {symbol}[] = {{\n{body}\n}};\n"
        f"static const unsigned int {symbol}_LEN = {len(data)};\n"
    )


def main() -> int:
    ap = argparse.ArgumentParser()
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--flag-file", help="path to the gitignored plaintext flag")
    src.add_argument("--text", help="arbitrary plaintext (a password, not a flag)")
    how = ap.add_mutually_exclusive_group(required=True)
    how.add_argument("--key", help="derive the keystream seed from this string")
    how.add_argument("--seed", help="use this 32-bit seed directly (0x... or decimal)")
    ap.add_argument("--symbol", default="SEALED")
    ap.add_argument("--out", required=True)
    ap.add_argument("--append", action="store_true")
    args = ap.parse_args()

    if args.flag_file:
        path = pathlib.Path(args.flag_file)
        if not path.exists():
            sys.exit(f"seal: {path} does not exist -- create it (it is gitignored) before building")
        plain = path.read_text().strip().encode()
        if not plain:
            sys.exit(f"seal: {path} is empty")
        if not plain.startswith(b"Securinets{") or not plain.endswith(b"}"):
            sys.exit(f"seal: {path} does not hold a Securinets{{...}} flag")
    else:
        plain = args.text.encode()

    seed = seed_from_key(args.key.encode()) if args.key else int(args.seed, 0) & MASK
    sealed = bytes(p ^ k for p, k in zip(plain, keystream(seed, len(plain))))
    if sealed == plain:
        sys.exit("seal: sealed bytes equal the plaintext -- refusing to ship that")

    body = as_c_array(args.symbol, sealed)
    out = pathlib.Path(args.out)
    if args.append and out.exists():
        text = out.read_text().replace("\n#endif\n", "\n" + body + "\n#endif\n")
    else:
        text = "#ifndef PAYLOAD_H\n#define PAYLOAD_H\n\n" + body + "\n#endif\n"
    out.write_text(text)
    print(f"   sealed {len(plain)} bytes as {args.symbol} -> {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Compute the code-derived key that Dead Air seals its blobs under.

Build tooling, so the no-comments rule does not apply (CLAUDE.md exempts tools/).

The binary derives its keystream seeds at run time by hashing its OWN machine
code -- 64 bytes starting at `unwrap` -- instead of using a constant. This
script reproduces that hash from the ELF on disk so build.sh can seal the
payload under the same value.

Why: a constant seed means anyone can read the decompiled unwrap, copy the
literal, and recover the token and flag offline without ever touching the
network -- which is the entire lesson of this challenge. A seed that is a
property of the compiled image cannot be copied out of a decompiler listing;
it has to be extracted from the exact bytes at the exact offset, or the binary
has to be run. Running it is the intended path, so the tax lands squarely on
the shortcut.

This works because .text is identical in the file and in memory (no relocation
rewrites code here) and identical between the stripped and unstripped builds
(`strip` only drops the trailing symbol tables). build.sh asserts both.

    codekey.py --symbols build/relay.dbg --binary build/relay --symbol unwrap
"""

import argparse
import pathlib
import subprocess

MASK = 0xFFFFFFFF


def symbol_vaddr(path: pathlib.Path, name: str) -> int:
    out = subprocess.run(["nm", str(path)], capture_output=True, text=True, check=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] == name:
            return int(parts[0], 16)
    raise SystemExit(f"codekey: no symbol {name!r} in {path}")


def text_mapping(path: pathlib.Path) -> tuple:
    out = subprocess.run(["objdump", "-h", str(path)], capture_output=True, text=True, check=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 6 and parts[1] == ".text":
            return int(parts[3], 16), int(parts[5], 16)
    raise SystemExit(f"codekey: no .text section in {path}")


def code_bytes(symbols: pathlib.Path, binary: pathlib.Path, symbol: str, length: int) -> bytes:
    vaddr = symbol_vaddr(symbols, symbol)
    vma, file_off = text_mapping(binary)
    offset = vaddr - vma + file_off
    blob = binary.read_bytes()
    if offset + length > len(blob):
        raise SystemExit("codekey: computed range falls outside the file")
    return blob[offset:offset + length]


def fnv(data: bytes) -> int:
    h = 0x811C9DC5
    for b in data:
        h ^= b
        h = (h * 0x01000193) & MASK
    return h


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--symbols", required=True)
    ap.add_argument("--binary", required=True)
    ap.add_argument("--symbol", default="unwrap")
    ap.add_argument("--length", type=int, default=64)
    ap.add_argument("--print-bytes", action="store_true")
    args = ap.parse_args()

    data = code_bytes(pathlib.Path(args.symbols), pathlib.Path(args.binary),
                      args.symbol, args.length)
    if args.print_bytes:
        print(data.hex())
    else:
        print(f"0x{fnv(data):08x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

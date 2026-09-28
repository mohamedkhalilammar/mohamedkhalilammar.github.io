#!/usr/bin/env python3
"""Official solver for Broken Lock -- defeats the licence gate by patching.

The intended player move is to see that `licensed()` can never return 1 (its
seal check is a preimage problem, not a puzzle), find the function in a
disassembler, and make it return 1 anyway. This script does the same thing
non-interactively so build.sh can prove the challenge is solvable: it
overwrites the function's first six bytes with `mov eax, 1 ; ret`.

It locates the function by symbol, which a player cannot do on the stripped
binary they receive -- they find it by following the call from main. That is
the only difference between this and the manual solve.

    patch.py --symbols build/lock.dbg --binary build/lock --out /tmp/patched
"""

import argparse
import pathlib
import shutil
import subprocess

RET_TRUE = bytes((0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3))


def symbol_vaddr(path: pathlib.Path, name: str) -> int:
    out = subprocess.run(["nm", str(path)], capture_output=True, text=True, check=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] == name:
            return int(parts[0], 16)
    raise SystemExit(f"patch: no symbol {name!r} in {path}")


def text_mapping(path: pathlib.Path) -> tuple:
    out = subprocess.run(["objdump", "-h", str(path)], capture_output=True, text=True, check=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 6 and parts[1] == ".text":
            return int(parts[3], 16), int(parts[5], 16)
    raise SystemExit(f"patch: no .text section in {path}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--symbols", required=True)
    ap.add_argument("--binary", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--symbol", default="licensed")
    args = ap.parse_args()

    dbg, shipped, out = (pathlib.Path(p) for p in (args.symbols, args.binary, args.out))

    vaddr = symbol_vaddr(dbg, args.symbol)
    vma, file_off = text_mapping(shipped)
    offset = vaddr - vma + file_off

    shutil.copy(shipped, out)
    blob = bytearray(out.read_bytes())
    if offset + len(RET_TRUE) > len(blob):
        raise SystemExit("patch: computed offset falls outside the file")
    before = bytes(blob[offset:offset + len(RET_TRUE)])
    blob[offset:offset + len(RET_TRUE)] = RET_TRUE
    out.write_bytes(bytes(blob))
    out.chmod(0o755)

    print(f"   {args.symbol}() at vaddr 0x{vaddr:x} -> file offset 0x{offset:x}")
    print(f"   {before.hex()} -> {RET_TRUE.hex()}  (mov eax,1 ; ret)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

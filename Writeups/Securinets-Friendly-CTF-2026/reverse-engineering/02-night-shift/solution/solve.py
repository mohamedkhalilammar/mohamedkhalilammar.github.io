#!/usr/bin/env python3
"""Official solver for Night Shift -- recovers the rotation key from the binary.

This is the path a player is meant to walk: find the comparison table in the
binary, read the transform out of the disassembly, invert it. The only thing
this script does that a player would not is locate the table by parsing the
ELF instead of by eye in Ghidra.

    solve.py [path-to-binary]
"""

import argparse
import pathlib
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent.parent / "tools"))
from rota import forward, invert  # noqa: E402


def table_from_binary(path: pathlib.Path, length: int) -> bytes:
    raw = subprocess.run(
        ["objdump", "-s", "-j", ".rodata", str(path)],
        capture_output=True, text=True, check=True,
    ).stdout
    blob = bytearray()
    for line in raw.splitlines():
        parts = line.split()
        if len(parts) < 2 or not all(c in "0123456789abcdef" for c in parts[0]):
            continue
        for chunk in parts[1:]:
            if len(chunk) % 2 or not all(c in "0123456789abcdef" for c in chunk):
                break
            blob.extend(bytes.fromhex(chunk))
    for start in range(len(blob) - length + 1):
        candidate = bytes(blob[start:start + length])
        recovered = invert(candidate)
        if all(32 <= b < 127 for b in recovered) and forward(recovered) == candidate:
            return candidate
    raise SystemExit("solve: no plausible table found in .rodata")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", nargs="?", default="build/nightshift")
    ap.add_argument("--length", type=int, default=20)
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    here = pathlib.Path(__file__).resolve().parent.parent
    path = pathlib.Path(args.binary)
    if not path.is_absolute():
        path = here / path

    table = table_from_binary(path, args.length)
    key = invert(table).decode()

    if args.quiet:
        print(key)
    else:
        print(f"table  : {table.hex()}")
        print(f"key    : {key}")
        print(f"\nfeed it in:  printf '{key}\\n' | {args.binary}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

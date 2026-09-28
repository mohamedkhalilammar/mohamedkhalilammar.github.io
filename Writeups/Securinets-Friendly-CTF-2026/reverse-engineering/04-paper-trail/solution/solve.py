#!/usr/bin/env python3
"""Official solver for Paper Trail -- walks the player's route end to end.

The intended path is exactly three moves, and this script performs all three
so build.sh can prove they still work:

  1. recognise the PyInstaller bundle and extract it   (pyinstxtractor-ng)
  2. decompile the extracted bytecode back to source   (uncompyle6)
  3. read the passphrase out of the recovered source and reproduce the unseal

Step 3 is done by re-implementing the logic from the *decompiled* text rather
than by importing the original module, so a build that silently stopped
producing decompilable bytecode would fail here instead of passing on a
technicality.

Runs under the challenge's own 3.8 venv:  .venv/bin/python solution/solve.py
"""

import argparse
import os
import pathlib
import re
import subprocess
import sys
import tempfile


def extract(binary: pathlib.Path, workdir: pathlib.Path) -> pathlib.Path:
    subprocess.run(
        [sys.executable, "-m", "pyinstxtractor_ng", str(binary)],
        cwd=workdir, check=True, capture_output=True, text=True,
    )
    out = workdir / f"{binary.name}_extracted"
    if not out.is_dir():
        raise SystemExit("solve: pyinstxtractor produced no _extracted directory")
    return out


def decompile(pyc: pathlib.Path) -> str:
    proc = subprocess.run(
        [str(pathlib.Path(sys.executable).parent / "uncompyle6"), str(pyc)],
        capture_output=True, text=True,
    )
    if proc.returncode != 0:
        raise SystemExit(f"solve: uncompyle6 failed on {pyc.name}\n{proc.stderr.strip()}")
    return proc.stdout


def stream(seed: int, n: int):
    x = seed & 0xFFFFFFFF
    for _ in range(n):
        x ^= (x << 13) & 0xFFFFFFFF
        x ^= x >> 17
        x ^= (x << 5) & 0xFFFFFFFF
        yield x & 0xFF


def unwrap(seed: int, blob: bytes) -> bytes:
    return bytes(b ^ k for b, k in zip(blob, stream(seed, len(blob))))


def digest(key: bytes) -> int:
    h = 0x811C9DC5
    for b in key:
        h ^= b
        h = (h * 0x01000193) & 0xFFFFFFFF
    return h or 0x9E3779B9


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--binary", default="build/paper_trail")
    ap.add_argument("--expect")
    args = ap.parse_args()

    binary = pathlib.Path(args.binary).resolve()
    if not binary.exists():
        raise SystemExit(f"solve: {binary} does not exist -- build it first")

    with tempfile.TemporaryDirectory() as tmp:
        workdir = pathlib.Path(tmp)
        extracted = extract(binary, workdir)
        print(f"   extracted {len(list(extracted.rglob('*')))} entries from the bundle")

        payload_pyc = next(extracted.rglob("payload.pyc"), None)
        logic_pyc = next(extracted.rglob("paper_trail.pyc"), None)
        if payload_pyc is None or logic_pyc is None:
            raise SystemExit("solve: expected payload.pyc and paper_trail.pyc in the bundle")

        payload_src = decompile(payload_pyc)
        logic_src = decompile(logic_pyc)
        print("   both modules decompiled cleanly")

        hexes = re.findall(r'bytes\.fromhex\("([0-9a-f]+)"\)', payload_src)
        if len(hexes) != 2:
            raise SystemExit(f"solve: expected 2 blobs in payload.py, found {len(hexes)}")
        wrap, sealed = (bytes.fromhex(h) for h in hexes)

        seed_match = re.search(r"WRAP_SEED\s*=\s*(0[xX][0-9A-Fa-f]+|\d+)", logic_src)
        if not seed_match:
            raise SystemExit("solve: WRAP_SEED not visible in the decompiled source")
        wrap_seed = int(seed_match.group(1), 0)

        passphrase = unwrap(wrap_seed, wrap)
        flag = unwrap(digest(passphrase), sealed).decode()

    print(f"   passphrase: {passphrase.decode()}")
    print(f"   flag      : {flag}")

    if args.expect and flag != args.expect:
        raise SystemExit(f"   FAIL: recovered flag does not match src/flag.txt\n   got: {flag}")
    if args.expect:
        print("   ok -- extract, decompile, read, unseal: the whole route holds")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

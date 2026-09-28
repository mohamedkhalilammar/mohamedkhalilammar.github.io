#!/usr/bin/env python3
"""Prove the compiled library and the service compute the same token.

This is the stage straight-key's build.sh originally lacked and build-go.sh added:
a Python reimplementation agreeing with itself proves nothing. What matters is that
the *compiled binary* agrees with the service that will verify its output. A drift
here means every player's token is rejected -- a total challenge failure that no
unit test on either side alone would catch.

Usage: crosscheck.py <mint-binary> <key-file> [--runner wine]
"""
import argparse
import pathlib
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).parent))
from game_token import make_token, normalize_nonce

CASES = [
    (1, "3KQ7-9WTM-P2XA"),
    (1, "AAAA-AAAA-AAAA"),
    (2, "3KQ7-9WTM-P2XA"),
    (7, "Z"),
    (99, "0123456789-ABCDEFGHIJKLMNOPQRST"),
    (4294967295, "9ZZZ-0000-1111"),
]


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("mint")
    ap.add_argument("key_file")
    ap.add_argument("--runner", default=None, help="e.g. wine, for a Windows build")
    args = ap.parse_args()

    key = bytes.fromhex(pathlib.Path(args.key_file).read_text().strip())
    failures = 0

    for game_id, nonce in CASES:
        cmd = ([args.runner] if args.runner else []) + [args.mint, str(game_id), nonce]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode != 0:
            print(f"  FAIL  mint failed for {game_id}/{nonce}: {proc.stderr.strip()}")
            failures += 1
            continue

        from_c = proc.stdout.strip()
        from_py = make_token(key, game_id, normalize_nonce(nonce))
        if from_c != from_py:
            print(f"  FAIL  {game_id}/{nonce}: C gave {from_c}, service expects {from_py}")
            failures += 1

    print(f"crosscheck: {len(CASES)} cases, {failures} failures")
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Emit src/gamekey.h from a hex key file, XOR-masked at rest.

The mask is not security -- the key is in the shipped binary by necessity and
GAME-HACKING-TRACK.md records that ceiling deliberately. What it buys is that a
32-byte high-entropy run does not sit in .rdata as one contiguous, obviously-a-key
block, which is what a `binwalk`-style entropy sweep looks for.

Usage: gen_keyhdr.py <key-file> --out ../src/gamekey.h
"""
import argparse
import hashlib
import pathlib
import secrets
import sys

MASK_LEN = 16


def read_key(path: pathlib.Path) -> bytes:
    """Read a 32-byte key from a hex file, creating one if it does not exist."""
    if not path.exists():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(secrets.token_hex(32) + "\n")
        print(f"gen_keyhdr: created {path}", file=sys.stderr)

    raw = path.read_text().strip()
    try:
        key = bytes.fromhex(raw)
    except ValueError:
        raise SystemExit(f"gen_keyhdr: {path} is not hex")
    if len(key) != 32:
        raise SystemExit(f"gen_keyhdr: {path} must hold 32 bytes, found {len(key)}")
    return key


def c_array(data: bytes) -> str:
    lines = []
    for i in range(0, len(data), 12):
        chunk = ", ".join(f"0x{b:02x}" for b in data[i:i + 12])
        lines.append("    " + chunk + ",")
    return "\n".join(lines)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("key_file")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    key = read_key(pathlib.Path(args.key_file))

    # The mask is derived from the key so the header is reproducible: regenerating
    # from the same key file yields a byte-identical header, which keeps the build
    # diffable and keeps `git status` quiet on a no-op rebuild.
    mask = hashlib.sha256(b"cgv1-mask|" + key).digest()[:MASK_LEN]
    store = bytes(k ^ mask[i % MASK_LEN] for i, k in enumerate(key))

    header = f"""#ifndef CG_GAMEKEY_H
#define CG_GAMEKEY_H

#define CG_KEY_LEN {len(key)}
#define CG_KEY_MASK_LEN {MASK_LEN}

static const unsigned char CG_KEY_STORE[CG_KEY_LEN] = {{
{c_array(store)}
}};

static const unsigned char CG_KEY_MASK[CG_KEY_MASK_LEN] = {{
{c_array(mask)}
}};

#endif
"""
    out = pathlib.Path(args.out)
    out.write_text(header)
    print(f"gen_keyhdr: wrote {out} ({len(key)}-byte key, masked)")


if __name__ == "__main__":
    main()

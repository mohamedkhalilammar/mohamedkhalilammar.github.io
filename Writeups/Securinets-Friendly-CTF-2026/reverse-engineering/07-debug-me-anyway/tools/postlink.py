#!/usr/bin/env python3
"""Encrypt the `stage2` section of a linked ELF and patch the four constants the
loader needs.

Ours, not the player's -- build tooling, so the no-comments rule does not apply.

The mixer and keystream below MUST stay byte-for-byte equivalent to mix(),
temper() and unpack() in src/gate.c. Change one, change both; build.sh proves
they agree by running the artifact.

Key = temper(mix(.text bytes)). The runtime reads those same bytes back out of
/proc/self/exe -- NOT out of memory. That distinction is load-bearing: a
software breakpoint is a write to memory, so keying off memory would mean the
player's own first `break` changed the key. Keying off the file leaves
breakpoints free while still making an edit to the file fatal. See
notes/design.md; do not "simplify" it back.

All four patched constants live in .data, so patching them cannot disturb the
.text bytes the key is folded from.

PICK_MASK is chosen so that (key ^ PICK_MASK) % 5 == 4, which is the real
builder's slot in main's table. The other four slots are the decoys. Working out
statically which slot runs therefore requires reproducing the mixer exactly --
the same work as decrypting stage2.
"""

import argparse
import pathlib
import secrets
import struct
import subprocess
import sys

MASK = 0xFFFFFFFF
REAL_SLOT = 4
SLOTS = 5


def mix(h: int, data: bytes) -> int:
    for b in data:
        h ^= b
        h = (h * 0x01000193) & MASK
        h ^= h >> 13
        h = ((h << 5) | (h >> 27)) & MASK
    return h


def temper(h: int) -> int:
    for _ in range(64):
        h ^= (h << 7) & MASK
        h = (h * 0x9E3779B1) & MASK
        h ^= h >> 11
    return h or 0x9E3779B9


def keystream(seed: int, n: int):
    x = seed & MASK
    for _ in range(n):
        x ^= (x << 13) & MASK
        x ^= x >> 17
        x ^= (x << 5) & MASK
        yield x & 0xFF


def sections(path: pathlib.Path) -> dict:
    out = subprocess.run(["readelf", "-SW", str(path)], capture_output=True,
                         text=True, check=True).stdout
    found = {}
    for line in out.splitlines():
        line = line.strip()
        if not line.startswith("["):
            continue
        parts = line.split("]", 1)
        if len(parts) != 2:
            continue
        cols = parts[1].split()
        if len(cols) < 5:
            continue
        try:
            found[cols[0]] = (int(cols[2], 16), int(cols[3], 16), int(cols[4], 16))
        except ValueError:
            continue
    return found


def symbols(path: pathlib.Path) -> dict:
    out = subprocess.run(["nm", str(path)], capture_output=True, text=True,
                         check=True).stdout
    got = {}
    for line in out.splitlines():
        cols = line.split()
        if len(cols) == 3:
            got[cols[2]] = int(cols[0], 16)
    return got


def file_offset(addr: int, secs: dict) -> int:
    for name, (sa, so, ss) in secs.items():
        if name.startswith(".") and sa != 0 and sa <= addr < sa + ss:
            return so + (addr - sa)
    sys.exit(f"postlink: 0x{addr:x} is not inside any loaded section")


def patch(blob: bytearray, off: int, placeholder: int, value: int, what: str) -> None:
    if struct.unpack_from("<I", blob, off)[0] != placeholder:
        sys.exit(f"postlink: {what} does not hold its placeholder -- already sealed?")
    struct.pack_into("<I", blob, off, value)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("binary")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    path = pathlib.Path(args.binary)
    blob = bytearray(path.read_bytes())
    secs = sections(path)
    syms = symbols(path)

    for need in ("stage2", "stage2d", ".text", ".data"):
        if need not in secs:
            sys.exit(f"postlink: {path} has no '{need}' section")

    _ta, text_off, text_len = secs[".text"]
    _sa, s2_off, s2_len = secs["stage2"]
    _da, sd_off, sd_len = secs["stage2d"]
    if s2_len == 0 or sd_len == 0 or text_len == 0:
        sys.exit("postlink: stage2 or .text is empty -- did the compiler drop it?")
    if text_off <= s2_off < text_off + text_len:
        sys.exit("postlink: stage2 sits inside .text -- the key would cover the "
                 "ciphertext and the build could never reproduce it")

    key = temper(mix(0x811C9DC5, bytes(blob[text_off:text_off + text_len])))
    code = bytes(blob[s2_off:s2_off + s2_len])
    data = bytes(blob[sd_off:sd_off + sd_len])
    core_sum = temper(mix(mix(0x811C9DC5, code), data))

    sealed_code = bytes(p ^ k for p, k in zip(code, keystream(key, s2_len)))
    sealed_data = bytes(p ^ k for p, k in zip(data, keystream(key ^ 0x5BD1E995, sd_len)))
    if sealed_code == code or sealed_data == data:
        sys.exit("postlink: a sealed section equals its plaintext -- refusing")
    blob[s2_off:s2_off + s2_len] = sealed_code
    blob[sd_off:sd_off + sd_len] = sealed_data

    # Any mask satisfying the congruence works; pick a wide random one so the
    # constant does not read as "obviously zero / obviously a no-op" to someone
    # skimming the decompilation.
    pick_mask = None
    for _ in range(1 << 16):
        cand = secrets.randbits(32)
        if (key ^ cand) % SLOTS == REAL_SLOT:
            pick_mask = cand
            break
    if pick_mask is None:
        sys.exit("postlink: could not solve for PICK_MASK")

    for name, placeholder, value in (
        ("CORE_SUM", 0xDEADBEEF, core_sum),
        ("TEXT_OFF", 0xCAFEF00D, text_off),
        ("TEXT_LEN", 0xFEEDFACE, text_len),
        ("PICK_MASK", 0xABADCAFE, pick_mask),
    ):
        if name not in syms:
            sys.exit(f"postlink: symbol {name} not found -- build unstripped first")
        patch(blob, file_offset(syms[name], secs), placeholder, value, name)

    path.write_bytes(bytes(blob))
    if not args.quiet:
        print(f"   .text {text_len}B at 0x{text_off:x} -> key 0x{key:08x}")
        print(f"   stage2 {s2_len}B code + stage2d {sd_len}B blob sealed, sum 0x{core_sum:08x}")
        print(f"   PICK_MASK 0x{pick_mask:08x} -> slot {REAL_SLOT} of {SLOTS}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Post-link step: encrypt the duration table under a key derived from the compiled
binary's own .text, and patch it into the reserved slot.

This is why the key is nowhere in the file. The binary computes the same checksum over
its own mapped code at startup; the patcher computes it over the same bytes on disk.
Patching lands in .rdata, so .text -- and therefore the key -- is unchanged.

    ./patch_blob.py ../build/KeyboardSelfTest.exe TOKEN
"""
import argparse
import pathlib
import struct
import sys

from gen_payload import MSG_SEED, pack
from layout import slot_layout

MARKER = bytes(range(0xE1, 0xE9)) + bytes(range(0x5A, 0x62))
FNV_OFFSET, FNV_PRIME, MASK = 2166136261, 16777619, 0xFFFFFFFF


def text_section(data):
    """-> (raw offset, raw size) of .text."""
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    if data[e_lfanew:e_lfanew + 4] != b"PE\0\0":
        raise SystemExit("error: not a PE")
    n_sections = struct.unpack_from("<H", data, e_lfanew + 6)[0]
    opt_size = struct.unpack_from("<H", data, e_lfanew + 20)[0]
    table = e_lfanew + 24 + opt_size
    for i in range(n_sections):
        ent = table + i * 40
        if data[ent:ent + 5] == b".text":
            size, ptr = struct.unpack_from("<II", data, ent + 16)
            return ptr, size
    raise SystemExit("error: no .text section")


def code_checksum(data):
    ptr, size = text_section(data)
    h = FNV_OFFSET
    for b in data[ptr:ptr + size]:
        h = ((h ^ b) * FNV_PRIME) & MASK
    return h


def keystream(count, seed):
    x = seed & MASK
    for _ in range(count):
        x ^= (x << 13) & MASK
        x ^= x >> 17
        x ^= (x << 5) & MASK
        yield (x >> 16) & 0xFF


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("binary")
    p.add_argument("token")
    p.add_argument("--unit", type=int, default=150)
    p.add_argument("--slots", type=int, default=1,
                   help="1 = old single-blob C layout; >1 = the go multi-slot layout, "
                        "patching only the slots that layout.slot_layout(token) marks real")
    args = p.parse_args(argv)

    path = pathlib.Path(args.binary)
    data = bytearray(path.read_bytes())

    hits = [i for i in range(len(data) - len(MARKER))
            if data[i:i + len(MARKER)] == MARKER]
    if len(hits) != 1:
        raise SystemExit(f"error: marker found {len(hits)} times, expected exactly 1")
    region = hits[0] + len(MARKER)

    plain, count, durations = pack(args.token.strip().upper(), args.unit)
    master_key = code_checksum(data)

    if args.slots <= 1:
        blob = bytes(b ^ k for b, k in zip(plain, keystream(len(plain), master_key)))
        data[region:region + len(blob)] = blob
        path.write_bytes(bytes(data))
        print(f"patch_blob: .text key {master_key:#010x}, {len(blob)} bytes "
              f"at file offset {region:#x}")
        print(",".join(str(d) for d in durations))
        return 0

    # go multi-slot layout: only the real slots get patched. Decoy slots were
    # already written final (permanent random noise) by gen_payload.py and are
    # left alone -- their "plaintext" is never checked against anything.
    real_slots, salts = slot_layout(args.token)
    for slot in real_slots:
        key = master_key ^ salts[slot]
        blob = bytes(b ^ k for b, k in zip(plain, keystream(len(plain), key)))
        off = region + slot * len(plain)
        data[off:off + len(blob)] = blob
    path.write_bytes(bytes(data))

    print(f"patch_blob: .text key {master_key:#010x}, real slots {real_slots}, "
          f"{len(plain)} bytes each")
    print(",".join(str(d) for d in durations))
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Answer-key check against the SHIPPED binary, reimplementing exactly what it does at
startup: checksum .text, derive the key, decrypt the slot, expand the bits, decode the
Morse. If this returns the flag, the runtime path is sound and reproducible.

    ./verify_runtime.py ../build/KeyboardSelfTest.exe EXPECTEDTOKEN
"""
import argparse
import pathlib
import sys

from decode_log import decode, parse_log
from layout import slot_layout
from patch_blob import MARKER, code_checksum, keystream
from verify_build import to_log


def decode_slot(buf, bits, unit):
    durations = [unit * 3 if (buf[i >> 3] >> (7 - (i & 7))) & 1 else unit
                 for i in range(bits)]
    return decode(parse_log(to_log(durations, 1, 0)))


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("binary")
    p.add_argument("expected")
    p.add_argument("--bits", type=int, default=61)
    p.add_argument("--unit", type=int, default=150)
    p.add_argument("--slots", type=int, default=1)
    args = p.parse_args(argv)

    data = pathlib.Path(args.binary).read_bytes()
    region = data.find(MARKER) + len(MARKER)
    nbytes = (args.bits + 7) // 8
    key = code_checksum(data)
    print(f"key from .text  : {key:#010x}")

    if args.slots <= 1:
        buf = bytes(b ^ k for b, k in zip(data[region:region + nbytes],
                                          keystream(nbytes, key)))
        transmissions, unit = decode_slot(buf, args.bits, args.unit)
        print(f"unit recovered  : {unit:.0f} ms")
        if len(transmissions) != 1 or transmissions[0] != args.expected:
            print(f"FAIL: decoded {transmissions!r}", file=sys.stderr)
            return 1
        print(f"decoded         : {transmissions[0]}")
        print(f"flag            : Securinets{{{transmissions[0]}}}")
        return 0

    # go multi-slot layout: every real slot must independently decode to the
    # expected token, and at least one decoy slot must NOT (a sanity check that
    # decoys are actually distinct data, not accidental copies of the real one).
    real_slots, salts = slot_layout(args.expected)
    ok = True
    for slot in range(args.slots):
        slot_key = key ^ salts[slot]
        off = region + slot * nbytes
        buf = bytes(b ^ k for b, k in zip(data[off:off + nbytes],
                                          keystream(nbytes, slot_key)))
        transmissions, unit = decode_slot(buf, args.bits, args.unit)
        got = transmissions[0] if len(transmissions) == 1 else repr(transmissions)
        tag = "REAL" if slot in real_slots else "decoy"
        print(f"slot {slot} ({tag:5s}) unit={unit:5.0f}ms  -> {got}")
        if slot in real_slots and got != args.expected:
            print(f"FAIL: real slot {slot} decoded {got!r}, expected {args.expected!r}",
                  file=sys.stderr)
            ok = False
    if not ok:
        return 1
    print(f"flag            : Securinets{{{args.expected}}}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

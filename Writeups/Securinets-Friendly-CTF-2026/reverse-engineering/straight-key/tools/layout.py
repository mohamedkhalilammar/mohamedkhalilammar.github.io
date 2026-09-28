"""Shared slot layout for the multi-candidate blob (iteration 5, go build only).

Six ciphertext slots sit in the binary, one per transmission pass. Two of the six
carry the real message (independently keyed, so their ciphertext bytes differ even
though the plaintext is identical); the other four are permanent random noise chosen
once at generation time and never touched again.

gen_payload.py and patch_blob.py both need to agree on WHICH slots are real and what
each slot's key salt is, without a side-channel metadata file: both derive the layout
by hashing the token, which both scripts already receive as an argument. Same input,
same layout, no coordination file to go stale.

The runtime (src/go/transmit.go) never learns real_slots at all -- it decrypts and
transmits all six slots identically, uniformly, every run. Which one is real is a
build-time-only fact; nothing at runtime branches on it. See notes/design.md,
"Iteration 5", item 6.
"""
import hashlib
import random

NUM_SLOTS = 6
REAL_COUNT = 2


def slot_layout(token):
    """token -> (real_slots: sorted list of REAL_COUNT ints, salts: list of NUM_SLOTS ints)"""
    h = hashlib.sha256(token.strip().upper().encode()).digest()
    seed = int.from_bytes(h[:4], "big")
    rng = random.Random(seed)
    real_slots = sorted(rng.sample(range(NUM_SLOTS), REAL_COUNT))
    salts = [rng.getrandbits(32) for _ in range(NUM_SLOTS)]
    return real_slots, salts

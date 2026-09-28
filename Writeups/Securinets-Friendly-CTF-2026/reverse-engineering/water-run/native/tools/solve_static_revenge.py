#!/usr/bin/env python3
"""Recover Water Run: Revenge's flag from the distributed DLL, without loading it."""
import hashlib
import hmac
import sys
from pathlib import Path

blob = Path(sys.argv[1]).read_bytes()
expected = '5800f63582f8d603effaaf114e07eeac70745b0ecab23ce355d870957047f2c1'
if hashlib.sha256(blob).hexdigest() != expected:
    raise SystemExit('Different DLL build: recover its table offsets before using this solver.')

# Raw file offsets, not virtual addresses.
scattered = blob[0x5580:0x5580 + 45]
order = blob[0x55c0:0x55c0 + 45]
mask = blob[0x55f0:0x55f0 + 16]
store = blob[0x5600:0x5600 + 32]
tag = blob[0x5660:0x5660 + 16]
salt = blob[0x5670:0x5670 + 16]

master = bytes(value ^ mask[i % 16] for i, value in enumerate(store))
key = hmac.digest(master, b'vault|' + salt, 'sha256')
cipher = bytes(scattered[i] for i in order)
# Authenticate the reordered ciphertext with the same 10-byte suffix.
check = hmac.digest(key, cipher + b'|8000|A7C3', 'sha256')[:16]
if not hmac.compare_digest(check, tag):
    raise SystemExit('Tag mismatch: check the table offsets and byte order.')

stream = b''.join(
    hmac.digest(key, salt + counter.to_bytes(4, 'little'), 'sha256')
    for counter in range((len(cipher) + 31) // 32)
)
print(bytes(a ^ b for a, b in zip(cipher, stream)).decode('utf-8'))

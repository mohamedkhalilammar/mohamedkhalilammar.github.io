#!/usr/bin/env python3
"""Recover Water Run's flag from the distributed beginner DLL, without loading it."""
import hashlib
import hmac
import sys
from pathlib import Path

blob = Path(sys.argv[1]).read_bytes()
expected = '74b64f87fbdea2e9573d622010fa67a83f40a1ba9deab262e7d0ef639c5c7e98'
if hashlib.sha256(blob).hexdigest() != expected:
    raise SystemExit('Different DLL build: recover its table offsets before using this solver.')

# Raw file offsets, not virtual addresses.
scattered = blob[0x5380:0x5380 + 50]
order = blob[0x53c0:0x53c0 + 50]
mask = blob[0x5400:0x5400 + 16]
store = blob[0x5420:0x5420 + 32]
tag = blob[0x5490:0x5490 + 16]
salt = blob[0x54a0:0x54a0 + 16]

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

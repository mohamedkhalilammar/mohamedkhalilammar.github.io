import hashlib

prefix = b"MOJO-JOJO{GD8_Scr1pt1ng_1s_4_Pow3rful_Sk"
alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789{}_-!"

# Target hash bytes (extracted from movabs instructions)
# ba 88 1b d1 61 d8 dd 37 | 05 97 93 67 22 ab 12 ed
# 83 27 4c bf d4 69 78 6e | 4e c0 1b 1a 20 e9 d5 6f
# These are movabs into parts of the hash on stack.
# Let's try to find the correct ordering.

target_parts = [
    0x37ddd861d11b88ba,
    0xed12ab2267939705,
    0x6e7869d4bf4c2783,
    0x6fd5e9201a1bc04e
]

target_bytes = b"".join(p.to_bytes(8, 'little') for p in target_parts)

for c1 in alphabet:
    for c2 in alphabet:
        for c3 in alphabet:
            # We assume c4 is '}'
            candidate = prefix + c1.encode() + c2.encode() + c3.encode() + b"}"
            if len(candidate) == 44:
                h = hashlib.sha256(candidate).digest()
                if h == target_bytes:
                    print(f"FOUND: {candidate.decode()}")
                    exit()

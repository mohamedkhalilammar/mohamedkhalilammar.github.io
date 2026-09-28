#!/usr/bin/env python3
"""The Night Shift transform, in one place.

Build tooling, so the no-comments rule does not apply (CLAUDE.md exempts tools/).

This file is the single definition of the forward transform. build.sh uses it
to generate the comparison table the binary ships, and solution/solve.py uses
it to invert -- so the challenge and its official solver can never drift apart
without the build failing. The C in src/nightshift.c must mirror `forward`
exactly; if you change one, change both and let build.sh catch you.
"""


def rot(v: int, n: int) -> int:
    n &= 7
    return ((v << n) | (v >> ((8 - n) & 7))) & 0xFF


def unrot(v: int, n: int) -> int:
    return rot(v, (8 - (n & 7)) & 7)


def forward(plain: bytes) -> bytes:
    return bytes(rot(b ^ ((0x5A + 7 * i) & 0xFF), i) for i, b in enumerate(plain))


def invert(table: bytes) -> bytes:
    return bytes(unrot(t, i) ^ ((0x5A + 7 * i) & 0xFF) for i, t in enumerate(table))


if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        raise SystemExit("usage: rota.py <text>")
    t = forward(sys.argv[1].encode())
    assert invert(t) == sys.argv[1].encode(), "transform is not invertible"
    print(t.hex())

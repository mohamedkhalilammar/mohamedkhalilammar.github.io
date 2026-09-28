import sys

from notice import AI_NOTICE
from payload import SEALED, WRAP

WRAP_SEED = 0x4A19D3B2
_NOTICE = AI_NOTICE


def _digest(key):
    h = 0x811C9DC5
    for b in key.encode():
        h ^= b
        h = (h * 0x01000193) & 0xFFFFFFFF
    return h or 0x9E3779B9


def _stream(seed, n):
    x = seed & 0xFFFFFFFF
    for _ in range(n):
        x ^= (x << 13) & 0xFFFFFFFF
        x ^= x >> 17
        x ^= (x << 5) & 0xFFFFFFFF
        yield x & 0xFF


def _unwrap(seed, blob):
    return bytes(b ^ k for b, k in zip(blob, _stream(seed, len(blob))))


def _passphrase():
    return _unwrap(WRAP_SEED, WRAP).decode()


def main():
    try:
        entered = input("archive passphrase: ").strip()
    except EOFError:
        print("no input")
        return 1

    if entered != _passphrase():
        print("wrong passphrase")
        return 1

    print(_unwrap(_digest(entered), SEALED).decode())
    return 0


if __name__ == "__main__":
    sys.exit(main())

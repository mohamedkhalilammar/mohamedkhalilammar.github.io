#!/usr/bin/env python3
import argparse
import hashlib
import hmac
import pathlib


def arr(name: str, data: bytes) -> str:
    body = ", ".join(f"0x{x:02x}" for x in data)
    return f"static const unsigned char {name}[{len(data)}] = {{{body}}};"


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("flag_file")
    ap.add_argument("key_file")
    ap.add_argument("--out", required=True)
    ns = ap.parse_args()
    flag = pathlib.Path(ns.flag_file).read_text().strip().encode()
    master = bytes.fromhex(pathlib.Path(ns.key_file).read_text().strip())
    if not flag.startswith(b"Securinets{") or not flag.endswith(b"}"):
        raise SystemExit("game flag must use the Securinets{...} format")
    salt = hashlib.sha256(b"cg-offline-v2|" + master).digest()[:16]
    key = hmac.new(master, b"vault|" + salt, hashlib.sha256).digest()
    stream = bytearray()
    counter = 0
    while len(stream) < len(flag):
        stream.extend(hmac.new(key, salt + counter.to_bytes(4, "little"), hashlib.sha256).digest())
        counter += 1
    cipher = bytes(a ^ b for a, b in zip(flag, stream))
    order = sorted(range(len(flag)), key=lambda i: hashlib.sha256(key + i.to_bytes(2, "little")).digest())
    scattered = bytes(cipher[i] for i in order)
    inverse = bytes(order.index(i) for i in range(len(flag)))
    tag = hmac.new(key, cipher + b"|8000|A7C3", hashlib.sha256).digest()[:16]
    text = "\n".join([
        "#ifndef CG_VAULT_H",
        "#define CG_VAULT_H",
        f"#define CG_VAULT_LEN {len(flag)}",
        arr("CG_VAULT_S", salt),
        arr("CG_VAULT_C", scattered),
        arr("CG_VAULT_P", inverse),
        arr("CG_VAULT_T", tag),
        "#endif",
        "",
    ])
    pathlib.Path(ns.out).write_text(text)


if __name__ == "__main__":
    main()

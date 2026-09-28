"""Pull the two facts Dead Air hides out of a syscall trace.

The hostname and the port are both sealed, so `strings` shows neither. Both
are recoverable by watching the process instead of reading it, which is the
whole point of the challenge:

  host -- glibc puts the name in cleartext into the DNS query it emits, so it
          is sitting in the sendto/recvfrom buffer in DNS wire format
          (length-prefixed labels, NUL terminated).
  port -- once the name resolves, the connect() call carries it.

    read_trace.py --field host trace.txt
"""

import argparse
import re

ESCAPE = re.compile(rb'\\(x[0-9a-fA-F]{2}|[0-7]{1,3}|.)')
SIMPLE = {b'n': 10, b't': 9, b'r': 13, b'v': 11, b'f': 12, b'b': 8,
          b'a': 7, b'0': 0, b'\\': 92, b'"': 34, b"'": 39}
QUOTED = re.compile(rb'"((?:[^"\\]|\\.)*)"')
PORT = re.compile(rb'sin_port=htons\((\d+)\)')
LABEL = re.compile(rb'^[A-Za-z0-9-]+$')


def unescape(raw: bytes) -> bytes:
    def sub(m):
        body = m.group(1)
        if body[:1] == b'x':
            return bytes((int(body[1:], 16),))
        if body in SIMPLE:
            return bytes((SIMPLE[body],))
        if all(48 <= c <= 55 for c in body):
            return bytes((int(body, 8) & 0xFF,))
        return body
    return ESCAPE.sub(sub, raw)


def dns_names(blob: bytes):
    """Every length-prefixed label run in the buffer, as dotted names."""
    out = []
    i = 0
    while i < len(blob):
        labels = []
        j = i
        while j < len(blob):
            n = blob[j]
            if n == 0:
                break
            if not 1 <= n <= 63 or j + 1 + n > len(blob):
                labels = []
                break
            label = blob[j + 1:j + 1 + n]
            if not LABEL.match(label):
                labels = []
                break
            labels.append(label)
            j += 1 + n
        if len(labels) >= 2:
            out.append(b'.'.join(labels).decode())
            i = j + 1
        else:
            i += 1
    return out


def host_from(text: bytes):
    seen = {}
    for quoted in QUOTED.findall(text):
        for name in dns_names(unescape(quoted)):
            seen[name] = seen.get(name, 0) + 1
    if not seen:
        return None
    # A resolver retries with the search domain appended, so the shortest name
    # that turns up is the one the program actually asked for.
    return min(sorted(seen), key=lambda n: (n.count('.'), len(n)))


def port_from(text: bytes):
    ports = [int(p) for p in PORT.findall(text)]
    wanted = [p for p in ports if p not in (53, 0)]
    return wanted[0] if wanted else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("trace")
    ap.add_argument("--field", choices=("host", "port"), required=True)
    args = ap.parse_args()

    text = open(args.trace, "rb").read()
    value = host_from(text) if args.field == "host" else port_from(text)
    if value is None:
        raise SystemExit(f"read_trace: no {args.field} in the trace")
    print(value)


if __name__ == "__main__":
    main()

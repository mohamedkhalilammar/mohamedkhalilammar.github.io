#!/usr/bin/env python3
"""The depot relay the player has to stand up themselves.

This is the official solution to Dead Air, and build.sh uses it to prove the
challenge is solvable. A player does not need this script: the binary prints
the token it wants into its own HTTP request, so `nc -l -p 8080`, reading the
token off the screen, and typing it back is enough. This just automates that
so it can run unattended.

    relay.py --port 8080 [--wrong]

--wrong replies without the token, which is what the build uses to confirm the
binary actually checks the reply instead of accepting anything.
"""

import argparse
import re
import socket

TOKEN_HEADER = re.compile(rb"X-Relay-Expect:\s*(\S+)", re.IGNORECASE)


def serve_once(port: int, wrong: bool) -> str:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind(("127.0.0.1", port))
        srv.listen(1)
        srv.settimeout(30)
        conn, _ = srv.accept()
        with conn:
            conn.settimeout(10)
            request = b""
            while b"\r\n\r\n" not in request and len(request) < 8192:
                chunk = conn.recv(4096)
                if not chunk:
                    break
                request += chunk

            match = TOKEN_HEADER.search(request)
            if match is None:
                body = b"NO TOKEN IN REQUEST\n"
                token = ""
            else:
                token = match.group(1).decode()
                body = b"RELAY-ACK NOT-THE-TOKEN\n" if wrong else f"RELAY-ACK {token}\n".encode()

            conn.sendall(
                b"HTTP/1.1 200 OK\r\n"
                b"Content-Type: text/plain\r\n"
                + f"Content-Length: {len(body)}\r\n".encode()
                + b"Connection: close\r\n\r\n"
                + body
            )
    return token


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=8080)
    ap.add_argument("--wrong", action="store_true")
    args = ap.parse_args()
    token = serve_once(args.port, args.wrong)
    print(f"relay saw token: {token or '(none)'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

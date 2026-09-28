---
id: "friendly-ctf-relay"
title: "Relay"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The binary checks in with a server that does not exist, so you read the hostname out of its DNS query with strace, point that name at yourself, and answer its HTTP request with the token it sends you."
flag: "Securinets{4nsw3r_th3_c4ll_y0urs3lf_1e7d}"
tags:
  - "Reverse Engineering"
---

Run `relay` and it tries to phone home to a server that isn't there. It prints one line about a hostname not resolving, and exits. Your job is to make the check-in succeed.

This is the top of the ladder and the hardest of the five, mostly because you have to build part of the challenge yourself. There's no single trick. There are four small problems in a row, and the program tells you which one you're stuck on. So I'm going to go slowly.

## What you're looking at

A stripped 64-bit Linux ELF. You'll need `strace`, `nc` (netcat), `sudo` for one step, and two terminal windows.

```bash
chmod +x relay
./relay
echo $?
```

```text
the hostname does not resolve. find out what it is, and point it at your machine.
2
```

That `echo $?` is worth getting used to. Every program hands a number back to the shell when it exits, and `$?` holds the number from the last command. Zero means success, anything else means something went wrong. This binary uses a different number for each stage, so between the message and the exit code you always know how far you got:

| Exit code | Message | What it means |
|---|---|---|
| 2 | `the hostname does not resolve...` | the name lookup failed |
| 3 | `I'm speaking, but no one is hearing me !!` | the name resolved, but nothing accepted a connection |
| 4 | `You heard me but you didn't reply !!` | something accepted the connection but sent nothing back |
| 5 | `That's not what I wanted to hear !` | you replied, but not with what it wanted |
| 0 | the flag | done |

Keep this table open. You'll walk down it one row at a time.

## Finding the way in

### Stage 1: which hostname?

The message tells you to find out what the name is, so try `strings` first:

```bash
strings -a relay | less
```

```text
8080
GET /relay/checkin HTTP/1.1
Host: %s:%s
User-Agent: meridian-relay/4.2.1
X-Relay-Expect: %s
...
```

Useful. You can see it makes an HTTP request, you can see a port number, and you can see a header called `X-Relay-Expect`. But no hostname. It's stored scrambled, like the flags in every challenge in this track.

You might think: fine, I'll open it in Ghidra, find the unscrambling routine and the seed, and decode it myself. In the previous challenges that would have worked. Here it won't, and this is the part I hardened on purpose. You'll find seed constants (`0x2b84f16d` for the token, `0x6d02be47` for the hostname), but they aren't the real seeds. Before using them, the program XORs in a hash of its own machine code:

```c
static unsigned int code_key(void)
{
    const unsigned char *p = (const unsigned char *)(const void *)&unwrap;
    unsigned int h = 0x811c9dc5u;
    unsigned int n = CODE_SPAN;

    for (unsigned int i = 0; i < n; i++) {
        h ^= p[i];
        h *= 0x01000193u;
    }
    return h;
}
```

`CODE_SPAN` is 64. So the real seed depends on the exact 64 bytes of compiled code at the start of the unscrambling function. Copy the constant out of a decompiler and decode with it, and you get garbage. You could work all of that out and reproduce it, but it's a lot of effort compared to the alternative.

The alternative is to stop reading the program and watch it. To look up a name, a program has to hand that name to the system's resolver, and the resolver sends it out as a DNS query in plain text. It can't be scrambled at that point, because the DNS server has to read it. `strace` shows you every system call a program makes, including the bytes it sends:

```bash
strace -f -s 300 ./relay 2>&1 | less
```

`-f` follows any threads or child processes, and `-s 300` shows up to 300 bytes of each buffer instead of the default 32, which would cut the name off. Look through the calls that send data (`sendto`, `sendmmsg`, `sendmsg`, `write`). On my machine it was this one, trimmed:

```text
sendmmsg(3, [{msg_hdr={... msg_iov=[{iov_base="(\367\1\0\0\1\0\0\0\0\0\0\5relay\vmeridian-fs\2tn\0\0\1\0\1", iov_len=38}] ...
```

The name is in there, in DNS's own format. Each part of a hostname is written as a length byte followed by that many characters. `\5` then `relay` (5 characters). `\v` then `meridian-fs`: `\v` is just how `strace` prints byte 11, and `meridian-fs` is 11 characters long. `\2` then `tn`. Put the dots back: `relay.meridian-fs.tn`.

Two things can make this look different on your machine. Your resolver may retry with your network's search domain stuck on the end, so you might see the name more than once with extra bits attached. The shortest one is the real one. And some home routers answer DNS queries for names that don't exist. If yours does, your very first run may already say `no one is hearing me` and exit 3 instead of 2. The name is still in the trace either way.

### Stage 2: point the name at yourself

`/etc/hosts` is a file your system checks before it asks DNS. Add a line that says this name lives at `127.0.0.1`, which always means "this machine". It's a system file, so this is the one step that needs root:

```bash
echo '127.0.0.1 relay.meridian-fs.tn' | sudo tee -a /etc/hosts
./relay; echo $?
```

```text
I'm speaking, but no one is hearing me !!
3
```

Exit 3. The name resolves now, and the program tried to connect, but nothing was listening. Which port? You saw `8080` in `strings`, and `strace` confirms it:

```text
connect(3, {sa_family=AF_INET, sin_port=htons(8080), sin_addr=inet_addr("127.0.0.1")}, 16) = -1 EINPROGRESS (Operation now in progress)
```

`EINPROGRESS` looks like an error but isn't. The program opened the connection in non-blocking mode and is waiting for it to finish.

### Stage 3: listen

Ports below 1024 need root on Linux. 8080 doesn't, which is exactly why I picked it, so don't run your listener with `sudo`. Quickly check nothing else on your machine already owns that port, since dev servers love 8080:

```bash
ss -ltn | grep 8080
```

If that prints nothing, you're clear. In terminal 1, start a listener. With OpenBSD netcat (the default on Debian and Ubuntu):

```bash
nc -l 8080
```

With traditional netcat it's `nc -l -p 8080`. In terminal 2, run `./relay`. Terminal 1 now shows what the program sent:

```text
GET /relay/checkin HTTP/1.1
Host: relay.meridian-fs.tn:8080
User-Agent: meridian-relay/4.2.1
X-Relay-Expect: RELAY-7F3C-9A21-DEPOT
Connection: close
```

Read that `X-Relay-Expect` header like a sentence: the relay expects this. The program is announcing the reply it's waiting for. It's the token it will use to unscramble the flag, and it just sent it to you in the clear because it assumed the only thing listening would be its own server.

Terminal 2 is sitting there now, waiting for a reply. If you press Ctrl+C in terminal 1 without typing anything, or just wait 30 seconds, you get exit 4 (`You heard me but you didn't reply !!`). If you type something else and close the connection, you get exit 5 (`That's not what I wanted to hear !`). Both are worth seeing once, so you know the table is telling the truth.

### Stage 4: answer

netcat exits after each connection, so start `nc -l 8080` again in terminal 1, then run `./relay` in terminal 2. When the request shows up in terminal 1, type the token and press Enter:

```text
RELAY-7F3C-9A21-DEPOT
```

Then press Ctrl+C in terminal 1 to close the connection. The program reads until the other side hangs up (or 30 seconds pass), then checks whether your reply contains the token. Terminal 2 prints the flag:

```text
Securinets{4nsw3r_th3_c4ll_y0urs3lf_1e7d}
```

You don't need a proper HTTP response here, with a status line and headers. The program just searches everything you sent for the token, so a bare line typed into netcat is enough.

## The solve

The whole thing, start to finish:

```bash
strace -f -s 300 ./relay 2>&1 | grep -a 'send'
echo '127.0.0.1 relay.meridian-fs.tn' | sudo tee -a /etc/hosts

nc -l 8080
./relay
```

Run `nc -l 8080` in the first terminal and `./relay` in the second. Type `RELAY-7F3C-9A21-DEPOT` into the netcat window, press Enter, then Ctrl+C.

If you'd rather not type the token by hand, this is the listener I used to test the challenge. It reads the request, pulls the token out of the `X-Relay-Expect` header, and sends it back:

```python
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
```

Run `python3 relay.py` in one terminal and `./relay` in the other. `--wrong` deliberately answers without the token, which is how I checked that the binary really rejects a bad reply.

When you're done, take the line back out of your hosts file:

```bash
sudo sed -i '/relay.meridian-fs.tn/d' /etc/hosts
```

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/05-dead-air/`.

## Why it works

A program that talks to a network can't keep secret what it says on that network. It can scramble the hostname inside the file as carefully as it likes, but to connect it has to ask DNS for that name in plain text. It can scramble the token, but to check in it has to put the token in a request. Every secret here was protected on disk and handed over the moment the program used it.

That's the same lesson as Doorman, the first challenge on the ladder, just harder to reach. There it was a `strcmp` you could trace. Here it's a DNS query and an HTTP header, and you had to supply the server yourself before the program would say anything. The general version is worth keeping: when static analysis gets expensive, ask what the program has to *do* with the secret, and go watch it do that.

## Notes from building it

An earlier version left the hostname readable in `strings`. I only sealed it once I'd checked that `strace` recovers it from the DNS query, and the build re-checks that every time it runs, because a hidden name you can't find by watching would be a guessing game, not a challenge. For the same reason there's no anti-debugging in this binary at all: `strace` is the intended path, and blocking it would block the lesson. The self-hashing seeds forced a two-pass build, since I can't seal anything until the compiled code exists to hash, and the build refuses to ship if the code changes between the passes.

## Beyond the challenge

Imagine analyzing a suspicious client whose original command server is offline. DNS and connection attempts can reveal the destination and protocol even when the client produces no useful console output. A controlled replacement service in a lab can help you observe the next stage.

Treat outbound dependencies as part of program behavior. Reproduce only the responses you understand and observe what changes. This challenge uses a deliberately simple exchange; real clients may require authenticated or signed responses, so pointing a hostname at your listener does not guarantee the client will trust it.

## Keep learning

- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): trace the data passed to networking calls.
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): use breakpoints to inspect a request before it leaves the process.

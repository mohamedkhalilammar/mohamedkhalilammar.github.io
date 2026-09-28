---
id: "friendly-ctf-paper-trail"
title: "Paper Trail"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A 10 MB Linux binary that is really a PyInstaller bundle, so you extract it, decompile the Python bytecode, and read the passphrase and flag unsealing straight out of the source."
flag: "Securinets{unp4ck_th3_pyth0n_4nd_r34d_1t_2d8b}"
tags:
  - "Reverse Engineering"
---

The program asks for an archive passphrase and says `wrong passphrase` when you get it wrong. That part is ordinary. The odd part is the download: the file is about 10 MB, and the other binaries in this track are around 18 KB.

The challenge page says it straight out: it's 10 MB, it prints one line, and it isn't corrupt. Work out why a program this small is a file this big, and you'll know what to open it with. That really is most of the challenge.

## What you're looking at

```bash
ls -l paper_trail
file paper_trail
chmod +x paper_trail
./paper_trail
```

```text
-rwxr-xr-x 1 user user 10135944 ... paper_trail
paper_trail: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 3.2.0, ..., stripped
archive passphrase: hello
wrong passphrase
```

For the solve you'll want `pyinstxtractor-ng` and `uncompyle6`. I used a Python 3.8 virtual environment, matching the Python inside the bundle, and that's the setup everything below was tested on:

```bash
uv venv --python 3.8 .venv
uv pip install --python .venv/bin/python pyinstxtractor-ng uncompyle6
```

## Finding the way in

The reflex is to throw it into Ghidra. Do that and you'll find a real, working C program, and you'll spend an hour reading it, because it's the startup code for a Python interpreter. It unpacks things and launches Python. None of it has anything to do with the passphrase.

The size is the clue. A program that prints one line doesn't need 10 MB. Something else is packed in there with it, and `strings` will tell you what:

```bash
strings -a paper_trail | grep -iE '_MEIPASS|pyimod|PYZ|python3|pyinstaller'
```

```text
Could not load PyInstaller's embedded PKG archive from the executable (%s)
Could not side-load PyInstaller's PKG archive from external file (%s)
PYINSTALLER_SUPPRESS_SPLASH_SCREEN
...
Failed to get _MEIPASS as PyObject.
PYZ archive entry not found in the TOC!
...
mpyimod01_archive
mpyimod02_importers
mpyimod03_ctypes
blibpython3.8.so.1.0
```

The first line gives it away by name. That's PyInstaller, a tool that takes a Python program and bundles it together with an entire Python interpreter and every library it imports, so it runs on a machine without Python installed. `_MEIPASS` is the temporary folder it unpacks into, the `pyimod` entries are its own bootstrap modules, and `PYZ` is its archive format. `libpython3.8` tells you which Python version is inside, and that's the most useful line on the screen, because decompilers care a lot about Python versions.

So this isn't really a compiled binary. It's a Python program in a zip-like wrapper. Take the wrapper off:

```bash
.venv/bin/pyinstxtractor-ng paper_trail
```

```text
[+] Processing paper_trail
[+] Pyinstaller version: 2.1+
[+] Python version: 3.8
[+] Found 10 files in CArchive
[+] Beginning extraction...please standby
[+] Possible entry point: pyiboot01_bootstrap.pyc
[+] Possible entry point: paper_trail.pyc
[+] Found 76 files in PYZ archive
[+] Successfully extracted pyinstaller archive: paper_trail
```

The entry point is `paper_trail.pyc`, sitting at the top of `paper_trail_extracted/`. The modules it imports are one level deeper, in `paper_trail_extracted/PYZ.pyz_extracted/`, mixed in with a pile of standard library files like `base64.pyc` and `zipfile.pyc`. Two names in there don't belong to the standard library: `payload.pyc` and `notice.pyc`. The second is the competition notice every binary in this track carries. The first is what you want.

`.pyc` files are compiled Python bytecode, and bytecode keeps almost everything: function names, variable names, constants. `uncompyle6` turns it back into source. Start with the payload:

```bash
.venv/bin/uncompyle6 paper_trail_extracted/PYZ.pyz_extracted/payload.pyc
```

```python
WRAP = bytes.fromhex("487f80aabfbe7079acc80889d6966a6136b649bae1")
SEALED = bytes.fromhex("008e8d97a47e2716a8bb990fe2e49b44cbeb6190d931a3b60fc1957af26fe89cd8350fd089e4fb406abd901fc1f9")
```

Two blobs of scrambled bytes. Now the main program:

```bash
.venv/bin/uncompyle6 paper_trail_extracted/paper_trail.pyc
```

This is the fiddly part, so don't panic when it happens. `uncompyle6` prints a long wall of grammar rules and a `Parse error` before it gets to the source. It can't reconstruct `main()`, and it mangles one line. Scroll past the noise and you'll find this (trimmed):

```python
WRAP_SEED = 1243206578

def _digest(key):
    h = 2166136261
    for b in key.encode():
        h ^= b
        h = h * 16777619 & 4294967295
    else:
        return h or 2654435769


def _stream(seed, n):
    x = seed & 4294967295
    for _ in range(n):
        x ^= x << 13 & 4294967295
        x ^= x >> 17
        x ^= x << 5 & 4294967295
        yield x & 255


def _unwrap(seed, blob):
    return bytes((b ^ k for b, k in ))


def _passphrase():
    return _unwrap(WRAP_SEED, WRAP).decode()


def mainParse error at or near `SETUP_FINALLY' instruction at offset 0
```

That's still plenty. The constants come out in decimal, but `2166136261` is `0x811C9DC5` and `16777619` is `0x01000193`, the two FNV-1a constants, which you may recognise from the earlier challenges in this track. `_stream` is an xorshift keystream. The broken line in `_unwrap` is missing its `zip(...)`, but the shape tells you what it was: it XORs each byte of `blob` with the keystream, so it must be `zip(blob, _stream(seed, len(blob)))`.

And `main()`? You don't strictly need it, but if you want to be sure, `pydisasm` comes installed alongside `uncompyle6` and shows the raw bytecode:

```bash
.venv/bin/pydisasm paper_trail_extracted/paper_trail.pyc
```

Find the section for `main`. The names `main` uses are `input`, `_passphrase`, `_unwrap`, `_digest` and `SEALED`, and the strings are `archive passphrase: ` and `wrong passphrase`. Read in order: compare what you typed with `_passphrase()`, and if it matches, print `_unwrap(_digest(entered), SEALED)`.

So the passphrase is `WRAP` unscrambled with a fixed seed, and the flag is `SEALED` unscrambled with a seed made from the passphrase.

## The solve

Put the pieces back together:

```python
WRAP = bytes.fromhex("487f80aabfbe7079acc80889d6966a6136b649bae1")
SEALED = bytes.fromhex("008e8d97a47e2716a8bb990fe2e49b44cbeb6190d931a3b60fc1957af26fe89cd8350fd089e4fb406abd901fc1f9")
WRAP_SEED = 0x4A19D3B2


def digest(key):
    h = 0x811C9DC5
    for b in key.encode():
        h ^= b
        h = (h * 0x01000193) & 0xFFFFFFFF
    return h or 0x9E3779B9


def stream(seed, n):
    x = seed & 0xFFFFFFFF
    for _ in range(n):
        x ^= (x << 13) & 0xFFFFFFFF
        x ^= x >> 17
        x ^= (x << 5) & 0xFFFFFFFF
        yield x & 0xFF


def unwrap(seed, blob):
    return bytes(b ^ k for b, k in zip(blob, stream(seed, len(blob))))


passphrase = unwrap(WRAP_SEED, WRAP).decode()
print("passphrase:", passphrase)
print("flag:      ", unwrap(digest(passphrase), SEALED).decode())
```

```text
passphrase: meridian-archive-1994
flag:       Securinets{unp4ck_th3_pyth0n_4nd_r34d_1t_2d8b}
```

Or skip the second half and just give the program the passphrase:

```bash
printf 'meridian-archive-1994\n' | ./paper_trail
```

```text
archive passphrase: Securinets{unp4ck_th3_pyth0n_4nd_r34d_1t_2d8b}
```

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/04-paper-trail/`.

## Why it works

PyInstaller is a packaging tool, not a protection tool. It makes a Python program easy to hand to someone else, and it does that by shipping the bytecode more or less intact. Python bytecode is high-level: it keeps names, constants and structure, so getting it back to readable source is a normal, well-supported thing to do. Anything a PyInstaller app "hides" is one extraction and one decompile away.

The part that actually takes skill is recognising what you're holding. The same goes for .NET assemblies, Java JARs, Electron apps and Go binaries: each one has a telltale look, and each one has a tool that saves you hours. Before you reach for a disassembler, spend a minute asking what built the file. Here the answer was sitting in `strings` the whole time.

## Notes from building it

The bundle is pinned to Python 3.8 on purpose, and it's the most important decision in the challenge: built on a current Python, every decompiler a beginner reaches for would just report an unsupported version, and the intended solve wouldn't exist. Even on 3.8, `uncompyle6` still chokes on `main()`, as you saw, but everything you need survives it. I also built it without UPX, because a UPX-packed PyInstaller bundle tends to confuse the extractor, and the failure looks like a corrupt download rather than a puzzle.

## Beyond the challenge

An incident responder could receive a large executable that is mostly an interpreter and bundled application files. Recognizing a PyInstaller package changes the first step: inspect the archive and recovered bytecode before spending hours reversing the bootloader.

Identify the packaging layer before choosing a decompiler. Bundling Python into an executable makes distribution convenient; it does not guarantee that the program's logic or embedded secrets stay private. Bytecode tooling also depends on the Python version, so failed decompilation can be a compatibility problem rather than evidence that the code is unrecoverable.

## Keep learning

- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): use cross-references and data types to follow records through the program.
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): practice inspecting buffers at the code that consumes them.

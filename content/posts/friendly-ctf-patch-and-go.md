---
id: "friendly-ctf-patch-and-go"
title: "Patch&Go"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A licence check hashes a file against a constant you are not meant to match, so instead of passing it you patch the function to return 1, or set its return value in gdb."
flag: "Securinets{0n3_byt3_1s_4ll_1t_t00k_c93f}"
tags:
  - "Reverse Engineering"
---

This one doesn't even ask you for input. You run `lock`, it complains that there's no licence, and it exits.

The challenge page is blunt about it: there is no licence, and no input you can give will pass the check. So stop trying to pass the check. Change it. Every challenge before this one was about finding the right answer. This one is about noticing when there isn't one.

## What you're looking at

A stripped 64-bit Linux ELF.

```bash
chmod +x lock
./lock
```

```text
no licence at /etc/meridian/service.lic
locked -- no licence exists for this unit
```

You'll need `objdump`, `gdb`, and some way to edit bytes in a file. A hex editor works. So does `dd`, which is what I'll use below because it's already installed everywhere.

## Finding the way in

The obvious first thought is to give it a licence. If you did create that file, the program would get more specific with you. Here's what `strings` shows it can say:

```text
licence rejected
no licence at /etc/meridian/service.lic
licence rejected: seal must be 16 bytes
locked -- no licence exists for this unit
```

So it wants exactly 16 bytes, and then it checks them against something. Here's the function I wrote:

```c
static int licensed(void)
{
    unsigned char seal[32];
    unsigned int h = 0x811c9dc5u;
    size_t n;
    FILE *f;

    f = fopen(LICENCE_PATH, "rb");
    if (f == NULL) {
        puts("no licence at " LICENCE_PATH);
        return 0;
    }

    n = fread(seal, 1, sizeof seal, f);
    fclose(f);

    if (n != 16) {
        puts("licence rejected: seal must be 16 bytes");
        return 0;
    }

    for (unsigned int i = 0; i < 16u; i++) {
        h ^= seal[i];
        h *= 0x01000193u;
    }

    if (h != 0x3b6f21a4u) {
        puts("licence rejected");
        return 0;
    }

    return 1;
}
```

That loop is FNV-1a, a well-known hash. The 16 bytes get hashed down to one 32-bit number, and that number has to match a constant. Nobody handed you a licence, and you're not meant to invent one. Could you? FNV-1a isn't a cryptographic hash, so a determined person could go hunting for 16 bytes that land on that value, and you'd still need root to create a file under `/etc`. That's a lot of work to avoid changing one function.

Because that's the thing to notice. The whole program is one decision, "is it licensed, yes or no", and that decision is made on your machine, in code you can edit.

So find the decision. The binary is stripped, so start from the entry point. The first thing a program's startup code does is call `__libc_start_main` and pass it the address of `main`. You can see that address being loaded just before the call:

```bash
objdump -d -M intel lock | less
```

```text
    1098:	48 8d 3d 39 02 00 00 	lea    rdi,[rip+0x239]        # 12d8 <fopen@plt+0x268>
    109f:	ff 15 1b 3f 00 00    	call   QWORD PTR [rip+0x3f1b]        # 4fc0 <fopen@plt+0x3f50>
```

Ignore the `<fopen@plt+...>` labels. On a stripped binary `objdump` names every address relative to the last symbol it knows, and they mean nothing. The number that matters is `12d8`: that's `main`. Its first few lines:

```text
    12d8:	53                   	push   rbx
    12d9:	48 81 ec d0 00 00 00 	sub    rsp,0xd0
    ...
    12f3:	e8 f7 fe ff ff       	call   11ef <fopen@plt+0x17f>
    12f8:	85 c0                	test   eax,eax
    12fa:	74 43                	je     133f <fopen@plt+0x2cf>
```

It calls a function at `0x11ef`, checks whether the result is zero, and if it is, jumps to `0x133f`, which prints the `locked` message and returns 1. That function at `0x11ef` is `licensed()`. In Ghidra you'd see the same thing as an `if` around a call to `FUN_001011ef`.

A small thing that confuses people who have the source: if you go looking for `0x3b6f21a4` inside that function, you won't find it. The disassembly compares against `0xf528ee4c` instead. The compiler rearranged the final multiply to the other side of the comparison, which is allowed because multiplying by an odd number can always be undone. Same check, different constant.

## The solve

There are three reasonable ways to do this. Pick whichever makes more sense to you.

The first way is to make `licensed()` always say yes. Overwrite the start of the function with two instructions: `mov eax, 1` then `ret`. On x86-64, the return value of a function travels in `eax`, so this function now returns 1 immediately without looking at any file. Those two instructions are six bytes: `b8 01 00 00 00 c3`.

To patch a file you need a file offset, and `0x11ef` is an address. They happen to be the same number here, and you can check that rather than take my word for it:

```bash
objdump -h lock | grep text
```

```text
 11 .text         000002d7  0000000000001080  0000000000001080  00001080  2**6
```

The columns are size, virtual address, load address and file offset. `.text` sits at address `0x1080` and at file offset `0x1080`, so every address inside it equals its offset. Look at the bytes before you change them, then patch a copy:

```bash
xxd -s 0x11ef -l 8 lock
cp lock lock.patched
printf '\xb8\x01\x00\x00\x00\xc3' | dd of=lock.patched bs=1 seek=$((0x11ef)) conv=notrunc
./lock.patched
```

```text
000011ef: 4156 5348 83ec 3864                      AVSH..8d
Securinets{0n3_byt3_1s_4ll_1t_t00k_c93f}
```

`conv=notrunc` matters. Without it, `dd` cuts the file off right after your six bytes.

The second way is even smaller: flip the jump in `main`. The `je 133f` at `0x12fa` is encoded as `74 43`. Change `74` to `75` and it becomes `jne`, so the program now unlocks exactly when the licence check *fails*. That's one byte, which is where the flag's name comes from.

```bash
cp lock lock.flipped
printf '\x75' | dd of=lock.flipped bs=1 seek=$((0x12fa)) conv=notrunc
./lock.flipped
```

```text
no licence at /etc/meridian/service.lic
Securinets{0n3_byt3_1s_4ll_1t_t00k_c93f}
```

The third way doesn't touch the file at all. In `gdb`, stop right after `licensed()` returns and change `eax` before `main` looks at it. The binary is position-independent, and `gdb` loads it at `0x555555554000` by default, so `0x12f8` becomes `0x5555555552f8`. `starti` starts the program and pauses at its very first instruction, so the addresses exist before you set a breakpoint on them.

```bash
gdb ./lock
(gdb) starti
(gdb) break *0x5555555552f8
(gdb) continue
(gdb) set $eax = 1
(gdb) continue
```

```text
no licence at /etc/meridian/service.lic
Securinets{0n3_byt3_1s_4ll_1t_t00k_c93f}
```

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/03-broken-lock/`.

## Why it works

Any check that runs on a machine you control is a suggestion. The program's whole decision comes down to one value in one register, and you can change the instruction that produces it, the instruction that reads it, or the value itself while it's running. All three of the solves above are the same idea at different points in time.

What makes this worth a challenge is the recognition step. Beginners spend a long time trying to satisfy checks that were never meant to be satisfied. When the thing standing between you and the goal is a comparison against a hash you'll never match, the check itself is the obstacle, and the answer is to remove the obstacle. That's most of software cracking in one sentence, and it's also why licence checks that only live on the client don't really protect anything.

## Notes from building it

I split the gate from the payload on purpose: the check is trivial to patch, and the flag only gets unscrambled once the success path actually runs, so nobody has to reverse the unscrambling to win. The failure messages escalate (no file, wrong length, bad seal) so that "this can't be satisfied, patch it" is a conclusion you can reach instead of a guess. There's an honest hole I left open: the flag's unscrambling key is a plain constant in the binary, so someone who reverses that routine can skip patching entirely. Deriving the key from the code would have broken the moment you patched the code, which is the one thing this challenge asks you to do.

## Beyond the challenge

Consider a local application that enables a premium operation after one function returns true. Patching that return value changes the program's policy decision even if the original license calculation remains intact. This is useful to understand during authorized software protection reviews.

Map the check and its callers before editing anything. A bypass of one branch may leave later checks or decryption requirements unsatisfied. The lesson for a hosted product is to authorize valuable server operations on the server; the lesson for an analyst is to verify the behavior after a patch instead of treating the changed branch as the whole result.

## Keep learning

- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): study branches and the function graph before deciding what to patch.
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): practice stopping at a branch and inspecting the flags and registers.

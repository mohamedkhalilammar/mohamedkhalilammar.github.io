---
id: "friendly-ctf-doorman"
title: "Doorman"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The badge code is stored scrambled so strings finds nothing, but the program checks your input with strcmp, and ltrace prints both sides of that comparison."
flag: "Securinets{th3_d00r_0p3n3d_1ts3lf_a71c}"
tags:
  - "Reverse Engineering"
---

You get one file, `doorman`. Run it and it asks for a badge code. Type anything and it says `denied`. That's the whole program.

This is the first rung of the reverse-engineering ladder I built for Securinets Friendly CTF 2026, and it assumes you have never opened a binary before. The challenge page even tells you that running `strings` won't find the code. That isn't a taunt. It's a nudge toward a different kind of tool.

## What you're looking at

A 64-bit Linux ELF. If you're on Windows, WSL or any Linux VM will do.

```bash
file doorman
chmod +x doorman
./doorman
```

```text
doorman: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, ..., stripped
badge code: hello
denied
```

Two words in that `file` line are worth noticing. "Stripped" means the function names were removed, so a disassembler will show you names like `FUN_00101234` instead of whatever the author called things. "Dynamically linked" means the program borrows functions like `strcmp` and `puts` from the system's C library at run time instead of carrying its own copies. That second one is the whole challenge, as it turns out.

You'll want `ltrace` (`sudo apt install ltrace` on Debian or Ubuntu).

## Finding the way in

Start with `strings`. It prints every run of readable characters in a file, it takes a second, and it's always worth doing first.

```bash
strings doorman
```

```text
fflush
stdout
strcspn
fwrite
strcmp
libc.so.6
...
badge code: 
no input
denied
```

You'll also get a long block of competition notice text that every binary in this track carries. Skip past it. What you won't get is anything that looks like a badge code. The code is stored scrambled inside the file and only gets unscrambled into memory when the program starts, so on disk there's nothing readable to find.

Now look at the top of that list again. `strcmp` is the C library function that compares two strings and returns 0 if they match. It shows up in `strings` because the program imports it by name. So at some point the program takes what you typed and the real badge code and passes both to `strcmp`. At that moment the real code has to be sitting in memory as plain text, because `strcmp` has no idea it's supposed to be a secret.

That's exactly what `ltrace` shows you. It runs the program and prints every call it makes into a shared library, arguments included. It can do that because the program is dynamically linked: each call to `strcmp` goes through a small lookup step that `ltrace` can hook.

```bash
ltrace ./doorman
```

Type anything at the prompt. The end of the output looks like this:

```text
fgets("hello\n", 128, 0x7fac0379a8e0)            = 0x7ffc2ad51880
strcspn("hello\n", "\r\n")                       = 5
strcmp("hello", "SHIFT-4C2A-NIGHT")              = 21
puts("denied")                                   = 7
denied
+++ exited (status 1) +++
```

The `strcspn` line is the program trimming the newline off what you typed. The next line is the one you want. The first argument to `strcmp` is your input. The second is the badge code it wanted.

## The solve

```bash
printf 'x\n' | ltrace -e strcmp ./doorman
printf 'SHIFT-4C2A-NIGHT\n' | ./doorman
```

```text
badge code: Securinets{th3_d00r_0p3n3d_1ts3lf_a71c}
```

`-e strcmp` just tells `ltrace` to show that one function and skip the noise.

If `ltrace` gives you trouble on your distro, `gdb` gets you the same thing. On x86-64 Linux the first two arguments to a function travel in the `rdi` and `rsi` registers, so you stop at `strcmp` and read them:

```bash
gdb ./doorman
(gdb) break strcmp
(gdb) run
badge code: hello
(gdb) x/s $rdi
0x7fffffffdb00:	"hello"
(gdb) x/s $rsi
0x7fffffffdac0:	"SHIFT-4C2A-NIGHT"
```

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/01-doorman/`.

## Why it works

A program can hide data on disk as well as it likes. The moment it needs to use that data, it has to put it back together, and if the "use" is a call into a shared library, anybody watching those calls sees it. Scrambling the badge code beat `strings`. It did nothing against watching the program run.

The flag works the same way, one level down. It's also stored scrambled, and the key that unscrambles it is the badge code itself. So you can't skip the door: the flag only exists after the program has been given the right code.

The habit to keep from this is simple. After `strings`, the next thirty seconds on any crackme go to `ltrace` and `strace`. You'd be surprised how often a secret that's carefully hidden in the file gets passed straight to `strcmp`, `memcmp` or `open` a few instructions later.

## Notes from building it

The `strcmp` is deliberate, and I left myself a note not to "harden" it into a hand-written loop, because the traceable call is the entire lesson. Shift, the next challenge, is the one that takes it away. The binary also has to stay dynamically linked: a static build removes the lookup step `ltrace` hooks, and the easiest challenge in the track quietly becomes a disassembly exercise. You can also read the unscrambling loop in Ghidra and reimplement it, which is real reverse engineering and a perfectly good solve, so I didn't try to stop it.

## Beyond the challenge

Suppose you are reviewing an offline unlock utility that disguises its expected password in the file. It still has to compare the supplied value with something during execution. Observing that comparison can reveal information that a strings scan cannot.

The transferable skill is choosing an observation point where encoded data has become meaningful. Library tracing is useful when a program calls an observable comparison function; static linking, inlining or a custom check may require another approach. Hiding a local password's representation does not move the authentication decision outside the user's control.

## Keep learning

- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): practice string references and following the branch to a comparison.
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): use breakpoints and memory examination to check what a comparison receives.

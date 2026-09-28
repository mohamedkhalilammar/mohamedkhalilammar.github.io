---
id: "friendly-ctf-antidbg"
title: "AntiDbg"
category: "CTF Writeups"
date: "2026-09-27"
summary: "An anti-debug check refuses to run under a debugger, so you let it run, let it get its answer, then change that answer in a register before the program reads it."
flag: "Securinets{fl1p_th3_r3g1st3r_n0t_th3_byt3s_4c7f}"
tags:
  - "Reverse Engineering"
---

You get a Linux binary called `gate`. Run it on its own and it prints some boring electrical readings, tells you a "builder" is loaded but not invoked, and exits without ever showing you a flag. Run it under a debugger and it does something different: it refuses.

```
  ptrace(PTRACE_TRACEME) returned -1, which means a debugger already
  has this process. refusing to continue.
```

So the flag only comes out under a debugger, and the program won't run under one. That is the whole puzzle, and the way through it is smaller than it looks.

## What you're looking at

It's a stripped, 64-bit Linux ELF. You want a debugger, and **pwndbg** (a plugin for gdb) makes this much more pleasant, though plain gdb works fine too. There's also a file shipped alongside the binary that you should not ignore: `.gdb_history`. That's a gdb command history, and it is there on purpose. Open it and you're reading over the shoulder of someone who already solved this:

```
break ptrace
run
finish
set $rax=0
c
call (char *) reveal()
```

There's a lot of fumbling in that history (dozens of single-steps, a few typos of the `call` line), which is honest, but the shape of the intended solve is right there in the last handful of commands. The breadcrumb tells you three things: break on `ptrace`, do something to `$rax`, and call a function named `reveal`.

## Finding the way in

First, the thing that's blocking you. The refusal message names the technique in full: `ptrace(PTRACE_TRACEME)`. That's a Linux anti-debug classic. A process calls `ptrace` with `PTRACE_TRACEME` to say "let my parent trace me". A process can only have one tracer, so if a debugger already has it, that call fails and returns `-1`. The program checks for exactly that and quits.

Now, the obvious beginner instinct: the binary compares the result of `ptrace` against `-1`, so why not just patch that comparison out of the file with a hex editor? Find the `cmp`, flip a byte, done. This is the trap, and it's worth understanding why, because the flag name (`fl1p_th3_r3g1st3r_n0t_th3_byt3s`) is telling you not to.

Look at what happens after the check passes. The program unpacks an encrypted section using a key it computes from a checksum of its own code bytes, read back off disk. If you edit the file, even one byte in the code, the checksum changes, the key is wrong, and the section decrypts to garbage. The flag comes out mangled and you get no error explaining why. The static byte-patch defeats the check and breaks the payload in the same stroke.

The register write does not. A debugger changing a CPU register at run time doesn't touch the file, so the checksum still matches, the section unpacks correctly, and only the one value the check looks at is a lie. That's the distinction the challenge is built to teach: change the decision, not the code.

## The solve

Open it in gdb and follow the breadcrumb:

```
$ gdb ./gate
pwndbg> break ptrace
pwndbg> run
pwndbg> finish
pwndbg> set $rax = 0
pwndbg> continue
pwndbg> call (char *) reveal()
$1 = 0x405140 "Securinets{fl1p_th3_r3g1st3r_n0t_th3_byt3s_4c7f}"
```

Line by line, because every one earns its place:

- `break ptrace` breaks on the library call. This resolves even though the binary is stripped, because the dynamic symbol table has to survive for the linker, so you don't go hunting for addresses.
- `run` stops you inside `ptrace`, before it has done anything.
- `finish` lets the call return. gdb shows you the value: `-1`, "already traced".
- `set $rax = 0` is the lie. A function hands its result back in the `rax` register, and the program is about to test it. Setting it to `0` makes the check see success. This **must** come after `finish` — set it while still inside `ptrace` and the real `-1` lands on top of yours.
- `continue` lets it run. The check passes, it unpacks its payload, and then it calls `ptrace` a second time, so the one breakpoint you already set fires again, this time at a moment when the flag can be built.
- `call (char *) reveal()` calls a function the program never calls itself. `reveal` is dead code, left in for you to invoke. The `(char *)` cast is needed because the binary is stripped and gdb doesn't know the return type — leave it off and gdb tells you so: `'reveal' has unknown return type; cast the call to its declared return type`.

One thing that trips people: the order genuinely matters. `reveal()` jumps into that encrypted section, so if you call it at the first breakpoint, before you've flipped `$rax` and let it unpack, you jump into ciphertext and the process takes a SIGBUS. Defeat the check, let it unpack, then call the function.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/debug_me_anyway`.

## Why it works

Anti-debug checks are a decision made from data. `PTRACE_TRACEME` returns a number, the program tests it, the program branches. Anywhere there's a decision, there's a value feeding it, and if you're already inside a debugger you own every value the program can see. You don't have to stop it detecting you; you let it detect you, then rewrite the evidence in the register right before it looks.

The reason to prefer that over patching the file is the second half of the lesson. Real protections chain the check to something else, so defeating the check statically also breaks the thing you were after. Here it's a checksum keying a decryption, but the pattern is everywhere. A runtime register write is surgical: it changes one decision and leaves everything else, including the program's view of its own bytes, untouched. That's why "flip the register, not the bytes."

## Notes from building it

This is deliberately the easiest challenge in the RE ladder, and it fought me to keep it that way. Earlier versions had a 0xCC breakpoint trap, a buffer wipe, and a timer that punished slow stepping, and every one of them punished a beginner for experimenting, which is the opposite of what a first debugger challenge should do. I stripped all of it out. What's left is one register write and a binary that waits patiently. The static-analysis path is hardened (the cheap "guess the xorshift seed" attack that broke an early version is now closed, and there are four decoy flags in memory to waste an LLM's time), but none of that hardening touches the intended dynamic path, which stays five commands with no traps.

## Beyond the challenge

An analyst may find that a suspicious executable exits only when a debugger is attached. Tracing the detection result and the branch that consumes it can explain the difference between the instrumented and ordinary runs. The same checks can also appear in software protection systems, so their presence alone is not proof of malware.

Treat the changed return value as an experiment: record the original behavior, alter one decision, and see what becomes reachable. One bypass may expose further checks. Do not assume the first anti-debug condition is the only reason execution can diverge.

## Keep learning

- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): read about signals, breakpoints and changing execution.
- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): map the checks and their callers before changing a branch.

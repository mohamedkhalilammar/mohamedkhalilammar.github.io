# 07 "Debug Me Anyway" — design

Rung 07 of the RE ladder, and deliberately **the easiest challenge in it**. Two
jobs, both introductory:

1. First contact with **pwndbg** / gdb — breakpoints, `finish`, registers, `x/s`.
2. The idea that **an anti-debug check can be lied to**: let it run, let it get
   the right answer, then change the answer before the program reads it.

That is all it does. It is not trying to be hard, and it must not be made hard.

Flag `Securinets{fl1p_th3_r3g1st3r_n0t_th3_byt3s_4c7f}`, recorded in
`secrets/flags.json` as `re-07-debug-me-anyway`.

## The whole thing

`leashed()` calls `ptrace(PTRACE_TRACEME)`; under a debugger it returns -1 and
`main` refuses. `reveal()` unseals the flag into a stack buffer. `main` prints the
buffer's **address** and length, never its contents, then **waits in `fgets()`**.
So the flag is only ever readable from a debugger, the check is what stands in the
way, and once past it the player has unlimited real time.

Solve, in `solution/gdb-walkthrough.md`, replayed by `build.sh` on every build
under both pwndbg and plain gdb:

```
break ptrace / run / finish / set $rax = 0 / continue / call (char *) reveal()
```

**There is no solver script.** The command list players get is the only recorded
solve, so if it breaks, the build breaks.

## Deliberate: `break ptrace`, on a STRIPPED binary

The entry point is the library call, not our function. `break ptrace` resolves to
`<ptrace@plt>` before `run` even with the symbol table gone, because the dynamic
symbol table has to survive for the dynamic linker. `build.sh` asserts this every
build — if it ever stops resolving, the challenge has no way in.

That is why the artifact is stripped again after an interim version kept symbols
so players could `break leashed`. Breaking on the *library* call is the more
transferable skill and needs no symbols, so there is no reason to ship them.

`-no-pie` stays: if someone goes looking at addresses anyway, they are fixed and
identical on every machine.

## How the player gets a stop with the flag available — two rejected attempts first

This was got wrong twice before it was got right, and the reasons matter.

**Attempt 1: end in `fgets()` and tell the player `break fgets`.** Rejected as
guessy, correctly — nothing tells a player the program ends in `fgets`, so that
step is knowledge they can only be handed, not derive. (It also hid a real trap:
gcc rewrites `getchar()` into `getc()`, so a `getchar` build makes `break getchar`
resolve to a libc function that is never called and the breakpoint **silently
never fires**. That is why `fgets` was used at all.)

**Attempt 2: `continue`, then Ctrl+C.** Not guessy, but it cannot be replayed in
`gdb -batch`, so the documented path would have been the one path the build could
not verify.

**What it does now: the program calls `ptrace` a second time**, after unpacking,
and there is nothing to guess or interrupt — the single breakpoint the player
already set fires again. Repeated `PTRACE_TRACEME` is a real anti-debug idiom; the
second call's result is deliberately discarded, because undebugged the first call
has already made the parent the tracer and the second necessarily returns -1.

**And the flag is built by a function the program never calls.** `reveal()` is
dead code; the player invokes it themselves. It jumps into the encrypted `stage2`
section, so it only works *after* the check has been defeated and the section
unpacked — which is what keeps the register write load-bearing rather than
decorative. `build.sh` asserts that calling `reveal()` at the first breakpoint,
before the flip, yields nothing.

`strip --strip-all -K reveal` keeps exactly that one symbol. Players can
`info functions` and find it; the four decoy builders stay anonymous and
indistinguishable from the real one, which is the whole point of shipping them.

## The `getchar` rewrite — a trap for us, not the player

gcc rewrites `getchar()` into `getc(stdin)`. A binary built with `getchar()` has
no `getchar` in its PLT, so `break getchar` resolves to a libc function the
program never calls and the breakpoint **silently never fires** — the player sees
`continue` run to completion and the process exit. `fgets` survives as itself.
`build.sh` asserts `<fgets@plt>` is in the disassembly for exactly this reason.
The page and the walkthrough both call the `getchar` mistake out by name, because
it is the one a beginner will make.

Because the breakpoint is *at* `fgets`, it fires before anything is read, so the
verification harness needs no stdin games — `</dev/null` is fine.

## What was taken OUT, and do not put it back here

An earlier iteration of this rung had all of the following. Every one of them was
a dead end that punished a beginner for experimenting, and the user's call was to
strip it back to an introduction:

- **An encrypted payload keyed off the CHECK's own bytes**, which made hex-editing
  the check fail with a confusing message. The packing itself came back later to
  harden the static path, but keyed off `.text` as a whole instead, so editing the
  check is no longer punished — only the cheap static shortcut is.
- **The 0xCC trap** — a software breakpoint set inside the still-encrypted payload
  corrupted the decryption. A real lesson, and completely wrong for a first
  debugger challenge: the punishment for exploring is a refusal you cannot
  diagnose.
- **The wipe** — the buffer was zeroed immediately after decrypting, so the flag
  existed for a few instructions only and a late breakpoint got nothing.
- **Custom ELF sections** holding the check and the payload, so the two functions
  were recoverable only from section headers and `main` only from `_start`'s
  argument to `__libc_start_main`. (The artifact is stripped again now, but for a
  different reason: `break ptrace` needs no symbols, so shipping them is pointless.
  Nothing in the solve depends on finding our functions at all.)
- **Three checks** (`ptrace`, `/proc/self/status` TracerPid, and a timer on the
  derivation) and an 8-character key prompt, from the original design. The timer
  in particular punished breakpoints, which is the exact behaviour being taught.

If a harder anti-debug rung is wanted later, that machinery is the material for
it — in a **new** challenge, at a higher position on the ladder. This one stays
five commands with no traps.

## Also deliberate

- **The refusal names `PTRACE_TRACEME` in full.** A beginner who cannot google the
  technique cannot start. Sealing that string makes it a guessing game.
- **Multiple solves are all fine and the page says so:** `set $rip`, breaking on
  the `cmp` directly, or hex-editing the file. Nothing punishes any of them.
- **`continue` then Ctrl+C** is documented as an alternative to `break fgets`, for
  players who would rather interrupt than set a second breakpoint.
- **`report.c` stays** — real, boring arithmetic printed on startup, untouched by
  the check. Non-negotiable #4, and it is the only "noise" left.
- **The flag is never printed**, so running the binary can never leak it, and the
  memory read is a real instruction rather than flavour.

## Static resistance — the part that was measured, not assumed

The first version of this rung fell to a **twenty-line script with no decompiler**:
collect every 32-bit immediate in the disassembly, treat each as an xorshift32
seed, slide the keystream over the file, look for a window decrypting to
`Securinets{`. Three candidate seeds, one hit, seconds. That attack is now
`tools/static_attack.py` and **`build.sh` runs it every build and fails if it
recovers the real flag.**

Three things closed it:

1. **The real unsealer, its seed and its sealed bytes all ship encrypted.**
   `payload_entry()` lives in section `stage2` and the blob in `stage2d`; both are
   XOR-encrypted in the linked file by `tools/postlink.py`. None of the three is
   statically visible until stage2 is decrypted, so the cheap attack has nothing
   to find.
2. **The key is `temper(mix(.text))`** — folded over the program's own `.text`,
   read back out of `/proc/self/exe` at run time. Reproducing it statically means
   reimplementing both mixers byte-exactly, and a mistake gives garbage with no
   feedback.
3. **Four working decoy unsealers ship in the clear** (`tools/gen_decoys.py`),
   each yielding a plausible wrong `Securinets{...}`. The cheap attack now returns
   four complete, confident, wrong answers and no way to rank them. They sit in
   the same five-slot builder table as the real one in `main`, indexed by
   `(key ^ PICK_MASK) % 5` — so even working out *which slot runs* requires
   reproducing the mixer, which is the same work as decrypting stage2.

`PICK_MASK` is solved for at post-link so the real builder is always slot 4, and
is a wide random value rather than 0 so it does not read as a no-op.

**Exactly one flag-shaped string exists in memory at run time.** The decoys are
never the slot that runs, so `search -t bytes Securinets{` has a single hit and a
player following the page is never asked to choose. `build.sh` asserts the hit
count is 1 and that its address is the one the program printed.

**This does not make static extraction impossible, and nothing here claims it
does.** An offline binary that can produce the flag on any machine contains
everything needed to produce the flag. What changed is the price: from a
twenty-line script to identifying the packer, reproducing two mixers exactly,
decrypting two sections and then unsealing — with four convincing wrong answers
sitting in the way. The point was never a wall; it was making the debugger the
cheapest route, which it now clearly is.

**The dynamic path is untouched by all of this.** `break ptrace` / `finish` /
`set $rax = 0` / `break fgets` / `continue` / `x/s` still works, because the key
comes from the **file** and not from memory — so software breakpoints cannot
disturb it. The only residual trap is a breakpoint planted inside `stage2` before
it unpacks, which no documented step does.

## Known limits — stated, not papered over

- **A determined static solve still exists** — see the section above for exactly
  how much it now costs and why that is the honest ceiling for an offline binary.
- **The wait is also a window without a debugger.** While the process sits in
  `fgets`, anything with ptrace rights over it could read the buffer out of
  `/proc/<pid>/mem`. On distros with `yama ptrace_scope=1` a same-user non-parent
  is refused, but not everywhere. It is a harder route than using gdb and it is
  the same lesson, so it is not worth closing.
- **glibc 2.34+.** Verified running on Ubuntu 22.04/24.04, Debian 12 and Arch;
  **fails to start on Ubuntu 20.04 / Mint 20**. Stated on the challenge page.
  If 20.04 must be supported, rebuild on an older toolchain — **not** `-static`,
  which would break `break printf`/`break ptrace` once stripped.
- The solve was replayed on clean `ubuntu:22.04` with distro gdb and **no
  pwndbg**, so nothing depends on the plugin.

## Designer mode

`build/designer/gate` is the same source with `-DDESIGNER`, which prints the flag
after revealing it. For hinting and for checking the artifact without a debugger.
`build.sh` asserts it states the flag and that the shipped build does not. It has
the same `fgets` wait, so feed it an Enter (`printf '\n' | build/designer/gate`).

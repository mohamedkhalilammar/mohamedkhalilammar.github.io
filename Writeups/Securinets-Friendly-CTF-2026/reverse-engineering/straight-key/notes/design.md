# "Straight Key" — standalone single-flag challenge (malware/RE track)

Status: **built and Linux-verified 2026-09-07, fifth iteration.** A dynamic-analysis break
of iteration 4 (hooking the named emitter directly recovered the message with no
decryption at all) is fixed and re-verified — see "Iteration 5" near the bottom for what
changed, what attack tested each change, and the one residual gap that was found and
named rather than hidden. **Decided: the Go binary ships**, pending the VM test. The C
build is kept maintained as a fallback and was NOT touched this session — it still has
iteration 4's dynamic-analysis weakness; re-run iteration 5 against it first if it ever
becomes the one that ships.

Status of the original claim: **built and Linux-verified.** Compiles, runs under wine, and its
pattern decodes cold to the flag. Not device-verified — see the unverified assumptions
at the bottom.

The codename is the pun: a *straight key* is the name of the hand-operated Morse
transmitter, and it is also a keyboard key.

## Shape

Not a numbered case. Cases 01 and 02 are question banks graded in CTFd; this is one
artifact and one flag, submitted manually. It sits on the malware track because it is
Windows, clean-room built, and teaches a defender tool.

## The one-paragraph version

The player is given `KeyboardSelfTest.exe`. Running it appears to do nothing useful — a
stream of identical letters types itself into whatever window has focus, then it stops.
The letters carry no message. What carries the message is **how long each keypress is
held**: short holds and long holds, in Morse. Reading it requires writing a keyboard
monitor that timestamps key-down and key-up and reports durations. Nothing on screen
shows the data.

## Flag

```
Securinets{HELDNOTTYPED}
```

12 characters, 16.65 s per pass. The C build transmits once; the Go build transmits six
times with 3 s between, measured at 117 s end to end.

**Uppercase, no underscores — forced, not chosen.** Morse encodes A-Z and 0-9 and nothing
else. The wrapper cannot be transmitted, so the challenge page must state the submission
format or players burn attempts on casing.

**No leetspeak, deliberately.** The first token was `H0WL0NGN0TWH1CHB7E4`. It tested badly:
a solver decoded it correctly and then concluded *"the resulting phrase isn't grammatical,
there appears to be an additional transposition clue needed here"* — because `H0W`/`N0T`
read as a substitution cipher you are meant to keep unwrapping, and the hex tail `B7E4`
reads as ciphertext. A player who decodes perfectly would have distrusted their own answer
and hunted a stage 2 that does not exist. Plain English terminates cleanly. **Do not
reintroduce leetspeak or a hex tail here** — on this challenge they cost more than the
entropy is worth. The challenge page also states outright that there is no second layer.

## Channel spec

- **Key:** `K`, virtual key `0x4B`. Chosen over `F13` deliberately: `F13` is invisible in
  every window and was the elegant choice, but a beginner's first monitor commonly logs
  typed characters only and would report nothing at all, which reads as a broken challenge
  rather than as a clue. A letter key shows up in even a naive first attempt.
- **Injection:** `keybd_event` from `user32`. Synthesises input at driver level, so the
  press is visible to anything watching key state.
- **Unit:** 150 ms. Hold 1 unit = dot, 3 units = dash. Released 1 unit = gap inside a
  character, 3 units = gap between characters. Standard 1/3 ratios, no word gaps.
- **Repeats:** six passes separated by 3 s in the Go build; the C build sends one and was
  the outlier. Not decoration — the monitor has to be running before the sender, and every
  first-timer gets that order wrong once. With repeats they simply wait, and two minutes of
  unevenly arriving keys is the one free clue this challenge gives.

### Why the visible letters do not leak the answer

Looks like a hole, is not one. Injected input does **not** auto-repeat — auto-repeat comes
from keyboard hardware, not from Windows synthesising it from held state — so a 450 ms dash
and a 150 ms dot each produce exactly **one** `k`. The screen shows an unbroken run whose
length gives away the symbol count and nothing more. **If the injection method is ever
changed, re-check this first:** a method that auto-repeats would print the Morse in plain
sight and kill the challenge.

## Why it is a compiled binary and not a PowerShell script

It began as a `.ps1` with the durations in a plain array. That version was **tested against
a language model and it returned 18 of 19 characters from a single paste**, without running
anything, without writing a monitor. One slip — it read the table's final element as a rest
rather than a hold, turning `....-` into `....`. Luck, not a defence.

Compiling changes three things:

1. **It cannot be pasted into a chat window.** At a university CTF that is *the* shortcut.
2. **The durations stop being text.** `strings` returns nothing useful; extraction needs a
   hex viewer or a disassembler.
3. **It became testable here.** mingw and wine are both on the build box. The PowerShell had
   never been parsed by anything; the binary compiles, links, and its pattern is verified by
   actually running it. That closed the weakest assumption this challenge had.

What it does **not** change: the message is still in the artifact, and a competent reverser
extracts it and may then paste the numbers into a model. This raises the cost of the
shortcut; it does not close it. Nothing self-contained can.

**Cost paid, worth naming:** the PowerShell version could honestly say *"every line is here
to be read, satisfy yourself it is safe before running it."* A binary cannot. We now hand
students an opaque executable and rely on the track's VM-and-snapshot rule, as case 01
already does. Ship the C source in the debrief so the "audit what you run" lesson survives,
just delayed.

## What is in the shipped binary

The timeline is 61 alternating hold/rest elements, each either 1 unit or 3 — one bit each.
Bit-packed that is **8 bytes**, XOR-obfuscated with a keystream from a seeded xorshift and
expanded in memory at startup. `.rdata` carries 8 bytes of noise.

This matters because the plain table did not: as 32-bit integers, an alternating
`96 00 00 00` / `C2 01 00 00` pattern is obvious to anyone who opens a hex viewer. Packed
and obfuscated there is nothing to eyeball.

**It is obfuscation and is not claimed to be more.** The key is in the binary; whoever reads
the decrypt stub recovers the pattern. What changed versus the PowerShell XOR — which was
removed as worthless — is that the stub is now machine code rather than a readable loop
sitting beside the data, and no Morse table accompanies it announcing the method.

`build.sh` fails if the shipped binary contains the token, the word Morse, CTF branding, a
build-host path, or a plaintext duration table in either byte order.

## Briefing

Point at the keyboard, **not** the method. Do not ship a starter monitor — that hands over
the tool-building, which is the exercise. The page must carry: run in a VM and snapshot
first; run it with an empty Notepad focused; it runs about two minutes and repeats;
the submission format; and that the decoded string **is** the answer with no second layer.

## What this is not

The sample only injects keystrokes and sleeps. No persistence, no networking, no filesystem,
no registry, no process creation. It cannot read the keyboard, only write to it — a
transmitter, never a keylogger. **Do not ever add a receiving half.** The receiving half is
the player's to write.

## Designer mode

Required by `CLAUDE.md` for a timing-dependent module. Built as a **separate binary** from
the same source via `-DDESIGNER`, which prints the expanded pattern and injects nothing.
`build.sh` uses it for verification and deletes it; it never ships, so there is no designer
switch in the player's hands.

## Verification — what was actually run

- **The binary compiles clean** (`-Wall -Wextra`, stripped) and `file` reports PE32+.
- **The binary expands to the right pattern.** `designer.exe` is run under **wine** and its
  61 durations compared element-by-element against the generator. This tests the compiled
  decrypt-and-expand path, not the generator's opinion of it.
- **That pattern decodes cold to the flag.** Replayed as a capture and decoded with no
  knowledge of the token: unit recovered as 150 ms, every pass agreeing on
  `HELDNOTTYPED`.
- **Leak guards pass.** No token, no method name, no branding, no build-host path in ASCII
  or UTF-16LE, and no plaintext duration table.
- **Jitter tolerance measured** (on the timing scheme, 40 seeded runs per level): clean
  through +/-60 ms, collapses at +/-74 ms. Expected `Sleep` error is ~15 ms and the sender
  spins the last 16 ms of every wait, so roughly 4x margin. The failure mode at high jitter
  is the decoder's *unit estimate* sliding, not the 1:3 ratio closing.

Re-run `./build.sh` after any change; it performs all of the above.

### Not verified — needs Windows

- **A1.** That `keybd_event` injection is observable via `GetAsyncKeyState` polling with
  usable hold timing. This is the load-bearing assumption of the whole design; wine proves
  the binary runs and expands correctly, not that injection reaches a monitor. If it fails,
  the receiver must be a `WH_KEYBOARD_LL` hook — a much harder ask of a beginner, and the
  hinting level would need reconsidering. Run `solution/keyboard-monitor.ps1` against the
  sample first; it settles A1 in a minute.
- **A2.** That the `Sleep`-then-spin wait holds the ratio under real scheduler load.
  If it drifts, raise the unit to 200-250 ms — change the timing, never the token.
- **A3.** That injected `k`s do not auto-repeat in practice.
- **A4.** That the binary runs without SmartScreen or AV interference. An unsigned console
  PE that injects keystrokes is a plausible heuristic hit; this needs checking before it is
  handed to a room full of students.

## Files

| Path | What it is | Ships? |
|---|---|---|
| `build/KeyboardSelfTest.exe` | **the artifact players download** | yes |
| `build/KeyboardSelfTest-go.exe` | the Go artifact; not yet the shipped one | pending |
| `src/sender.c` | C source; `-DDESIGNER` builds the verification binary | in debrief |
| `src/go/main.go` | Go: stages, keystream, key derivation | in debrief |
| `src/go/transmit.go` | Go: the transmitting half, `!designer` only — emitter, per-slot decrypt, jitter | in debrief |
| `src/go/designer.go` | Go designer mode, `-tags designer`; prints every slot, never injects | no |
| `src/go/payload.go` | generated blob (6 slots + salts + dll/proc blobs) — do not edit by hand | no |
| `build-go.sh` | the Go pipeline; same stages as `build.sh` plus a compiled-expansion check | no |
| `src/token.txt` | the answer, in one place; the build reads it | no |
| `src/payload.h` | generated blob for the C build — do not edit by hand | no |
| `src/decoys.h` | generated decoy routines/tables for the C build only (iteration 3 scheme) | no |
| `build.sh` | generates, compiles, verifies under wine, guards leaks — C build | no |
| `solution/keyboard-monitor.ps1` | reference keyboard event monitor — the intended solve | no |
| `tools/layout.py` | slot layout for the go 6-slot scheme, shared by gen_payload.py/patch_blob.py | no |
| `tools/gen_payload.py` | packs and obfuscates the pattern (c: 1 slot; go: 6 slots) | no |
| `tools/gen_decoys.py` | C-build-only decoy routines/tables (iteration 3 scheme; go build no longer uses this) | no |
| `tools/morse.py` | shared alphabet | no |
| `tools/encode_morse.py` | designer encoder, synthetic capture generator | no |
| `tools/decode_log.py` | reference decoder | no |
| `tools/verify_build.py` | capture replay helper used by the build | no |
| `notes/superseded-powershell-sender.ps1.txt` | the PowerShell version, kept as history | no |

The flag is recorded here and in `src/token.txt`, not in `secrets/flags.json` — that file is
keyed by Android `ChallengeRegistry` id and this challenge has none. Same convention as
case 01.


## Hardening iteration 3 (2026-09-06) — the design the user specified

The second build (compiled C, 8-byte obfuscated blob, hardcoded xorshift seed) was still
one competent RE pass from solved: read the decrypt stub, lift 8 bytes, decode. The user
specified the fix and it is now built:

1. **No answer string anywhere**, and the *key* is not in the file either. The blob is
   decrypted under a key derived at run time from an **FNV-1a checksum of the binary's
   own `.text`**. To decrypt statically you must parse the PE, checksum the code exactly,
   and reimplement it. Built with `--disable-dynamicbase` so the mapped code is
   byte-identical to what the patcher checksummed on disk.
   **Verified: flipping one byte of `.text` makes it emit noise.** `build.sh` fails if
   that tamper check ever passes.
2. **261 plausible keyboard/HID diagnostic routines** and, more importantly, **12 decoy
   timing tables** of identical size and shape to the live one, each with its own
   decrypt-shaped expander. Extracting "the duration array" stops being a grep.
3. **The welcome line is assembled in memory**, never stored as text — `strings` shows
   nothing. It doubles as proof-of-life so nobody thinks the sample is broken.
4. **One transmission, then exit.** No repeats. The listener must already be running,
   which is what makes writing it non-optional. Re-runnable if the player misses it.

### Why this does not violate the doctrine's ban on obfuscation

`CLAUDE.md` rejects decoys and anti-decompiler tricks because they "slow a human roughly
as much as an agent". That holds for a static-analysis challenge. **It does not hold
here, because the intended solve never opens the binary at all** — the player runs it and
listens. Obfuscation costs the intended path nothing and costs the shortcut everything.
This is the one place in the project where that asymmetry exists; do not generalise it.

### What remains true, and should not be re-argued

The timings still *are* the message, so a determined reverser recovers them. What changed
is what a reader sees: not a flag, but "it presses a key with these durations", from which
they must infer that timing is meaningful, guess Morse, pick the live table out of
thirteen, and decode. That is several inferential steps rather than a read.

### Superseded — the Go proposal below was carried out; see Iteration 4

### Original proposal (kept for the reasoning): rewrite in Go

The remaining weakness is not the language, it is that `main()` is short and legible:
greet, run suite, expand, loop on `keybd_event`. The 261 decoys pad the file; they do not
hide `main`. A model that finds `main` reads the design in ten lines.

Go was proposed and is the right pick if this is taken further: Ghidra/IDA handle Go badly
(register calling convention, misidentified function boundaries), and a stripped binary is
~2 MB of runtime with our logic a few hundred bytes inside it. Go 1.26 and Rust 1.94 are
both installed; Rust would need `rustup target add x86_64-pc-windows-gnu`.
**Go's risks:** GC/scheduler jitter (sub-ms against a +/-60 ms tolerance, but measure it),
Go binaries embedding build paths and module metadata so the anti-leak grep matters more,
and the `.text` checksum needing rework to checksum via `os.Executable()` instead.
**Do it together with making the transmit path not one legible loop** — the language alone
is half the fix.


## Iteration 4 (2026-09-06) — the Go port, built and verified

`src/go/` now builds a second artifact with `./build-go.sh`. **It does not yet ship.**
`build.sh` and the C binary are untouched and still pass every check; which one goes to
students is decided after the VM test, not before.

### What the port changed, beyond the language

- **The transmit path is no longer one legible loop.** Work runs through a table of stage
  values (`announce, warmup, derive, unpack, deliver, cleanup`) rather than a straight line
  in `main`, and the transmit loop dispatches through a two-element parity table rather than
  branching on hold-versus-rest. The language alone was only half the fix, as the proposal
  said.
- **The key derivation reads the file, not the mapped image.** `os.Executable` → `debug/pe`
  → FNV-1a over `.text`. That sidesteps ASLR, so there is no `--disable-dynamicbase`
  equivalent to get wrong. `tools/patch_blob.py` and `tools/verify_runtime.py` work
  unchanged, exactly as the marker scheme was designed for.
- **Designer mode is a build tag, not a run-time flag.** `transmit.go` (`!designer`) and
  `designer.go` (`designer`) are alternatives, so the shipped image contains no designer
  switch for a player to find — the same property `-DDESIGNER` gave the C build.
- **Six passes, not one.** The run-once decision was reversed for the reasons recorded in
  `PROGRESS.md`; measured end-to-end at 117 s.

### The verification stage that did not exist before

`build.sh` proves the *patcher* agrees with itself: `verify_runtime.py` is a Python
reimplementation of the runtime path. `build-go.sh` adds a stage that closes that gap —
it builds the designer binary, patches it, runs it under wine, and diffs its printed
durations against the patcher's line byte for byte. **All 61 match.** That is the compiled
`derive`-and-`unpack` path agreeing with the patcher, not the patcher agreeing with itself.

Everything else holds: tamper check confirms one flipped `.text` byte yields noise, and the
artifact carries no token, no branding, no build-host path, no method name, and no plaintext
duration table.

### Timing, measured rather than assumed

The proposal listed GC and scheduler jitter as a risk to check. A full run under wine takes
**117.4 s against 114.9 s predicted** — a uniform **+6.8 ms per element**, which is bias
rather than jitter. The decoder estimates the unit from the signal, so a uniform stretch is
the one error it is immune to: replayed at that bias plus ±40 ms of added jitter, 40/40
seeded runs still decode to `HELDNOTTYPED`. `runtime.LockOSThread` and
`debug.SetGCPercent(-1)` are set before anything is timed. Re-measured after the switch to
garble: **117.3 s, +6.6 ms per element** — obfuscation costs nothing here. **This is wine on
Linux and is not a Windows measurement** — A2 stays open.

### Closed — Go keeps its function names, and `-s -w` does not strip them

Go's `pclntab` survives `-ldflags="-s -w"`, because the runtime needs it for tracebacks. An
ordinary `go build` therefore left the image naming `main.(*session).derive`, `.unpack`,
`.transmit`, `.hold`, `.rest` and `.strike`, plus all twelve `main.expandtbl*` decoys — a
reader who grepped `main.` got the design as a list. Against the C build, stripped with
`-s`, that was a **regression on exactly the axis Go was chosen for**.

**Fixed with `garble` (v0.17.0, `mvdan.cc/garble`).** `build-go.sh` now compiles through it,
and the same names come out as `main.(*bu5dDBPOc).heVEgPyD`. The whole pipeline was
re-verified under it — designer-diff, patch, `verify_runtime`, tamper check, leak grep — and
every stage passes.

A gate was added so this cannot silently return: the build extracts every `main.*` symbol
and fails if any of our own identifiers is still among them. **The gate was tested against a
plain `go build` and fires.** An ordinary `go build` can no longer produce a passing
artifact.

#### Two rules for anyone touching the Go build

1. **Never pass `garble -literals`.** It rewrites literal data into runtime-constructed
   form, which would destroy the 16-byte marker in `patStore` that `tools/patch_blob.py`
   locates in order to patch the blob; the patch step would fail with "marker found 0
   times". Name obfuscation is garble's default and is all this design wants.
2. **The `.text` checksum scheme survives garble, and that is proved rather than assumed** —
   it is what the designer-diff stage exists for. Re-run `./build-go.sh` after any change to
   the build and read that stage's output.
3. **Never delete a `//go:noinline` marker.** They are compiler directives, not comments, and
   the project's no-comments-in-source rule does not reach them. `stored()` carries one so
   the blobs stay addressed at run time rather than folded into code as immediates — inlined,
   they go out of reach of `tools/patch_blob.py`. `derive()`, `resolveSend()`, `deliver()` and
   `transmitSlot()` carry them so the run's shape is not flattened into one function. Removing
   any of them breaks the patcher or the design silently; nothing fails to compile.

`-seed=random` is used, so the obfuscated names differ on every build. That is deliberate for
an artifact players receive, and it means the binary is **not** byte-reproducible; the
verification stages re-prove correctness on each build instead.

### Decision: Go ships (pending the VM test)

The build choice is made, not deferred. `KeyboardSelfTest-go.exe` is the artifact that goes
to students once the VM test clears it. `build.sh`/`KeyboardSelfTest.exe` (C) stay maintained
as a **fallback**, not archived — kept green so a same-day artifact exists if Go fails the VM
test, rather than reopening the C path cold.

**What this decision costs, stated plainly:** choosing Go before A4 (SmartScreen/Defender) is
answered means A4 is no longer a bake-off between two candidates — it is pass/fail on the
harder case. A 2.1 MB garble-obfuscated binary that injects keystrokes is a materially
stronger AV heuristic hit than the 39 KB C build. If Defender flags it on the VM, the fix is
signing, an exclusion, or dialing back obfuscation — not a silent revert to C. That is a
deliberate choice, made with the cost known, not an oversight.

### Decided: no Linux build

Asked and closed the same session. Not a recompile — the two halves that make this
challenge beginner-friendly on Windows have no clean Linux equivalent:

- **Injection.** `keybd_event` has no cross-platform analog. X11 has `XTestFakeKeyEvent`
  (libXtst, unprivileged). Wayland has none — its security model blocks synthesizing input
  into another window on purpose; the workaround is `/dev/uinput`, which needs root or an
  `input`-group udev rule.
- **The monitor, which is the player's half.** `GetAsyncKeyState` is *why* this is
  approachable: any unprivileged process polls global key state instantly. X11 has the
  same shape (`XQueryKeymap`/XKB). **Wayland has nothing like it** — a normal process
  cannot query key state it doesn't have focus for. The real answer is raw
  `/dev/input/eventX`, again root or group membership.

So a Linux build is either X11-only (silently produces nothing on Wayland — default on
current Fedora, Ubuntu 22.04+, most rolling distros — with no clue why), or evdev-based
(works everywhere, but demands `sudo`/group setup on the player's own machine before the
puzzle starts, which is the friction `CLAUDE.md` explicitly says loses to beginner-friendly).
Also dilutes the point of being on this track at all — Windows PE was chosen specifically
to teach the isolated-VM-and-snapshot habit; a Linux ELF companion either skips that lesson
or needs its own isolated-VM story for one side challenge.

**Windows only. Players run it in their own lab — their own isolated VM, which we do not
provision or control (see `docs/MALWARE-TRACK.md`). Linux users need their own Windows VM
the same as anyone else; there is no shared/organizer-provided image to lean on.**

## Red-team finding (2026-09-06) — the Go/garble build is NOT resistant to static agentic RE

Verified, not theorized: the full flag was recovered from `build/KeyboardSelfTest-go.exe`
alone — no execution, no wine, no VM — with a ~40-line Python script implementing only
textbook PE parsing and FNV-1a. Result: `Securinets{HELDNOTTYPED}`.

**Why it falls:** `main.main` cannot be renamed by garble (the Go runtime calls it by that
exact name), so the entry point is free. Only 42 `main.*` symbols exist total (Go supplies
decoy-routine noise for free, so no junk functions were generated for this build, unlike
the C build's 261). The FNV-1a constants are textbook and recognizable on sight. The live
table is distinguishable from the 12 decoys by a structural tell (its keystream seed comes
from a function that opens a file and parses a PE header; the decoys' seeds are hardcoded
literals) — a shape difference, not just a data difference, which is exactly what an LLM
is good at spotting across near-identical blocks. Most importantly: **the key is a pure
function of bytes already static in the shipped file.** Nothing about recovering it requires
execution, timing, or environment — which is the actual gap against the project's own
pillar 3 (environment-coupled and stateful).

**What this does and does not mean.** The *weak* threat (paste the file/strings into a chat
and get the answer for free) is closed — iteration 1/2's demonstrated failure does not
recur. The *strong* threat (an agentic AI with PE-parsing + hash tooling and enough turns)
is not defended at all; it is a bounded, mechanical afternoon of work, not a wall.

**What would actually raise the bar, not attempted here:**
- Tie the key to something NOT fully present in the static file — real elapsed time, a
  device condition, session state — matching pillar 3 for real instead of only pillar 1.
- `-tiny` (strips Go's runtime type metadata, which garble's default mode leaves intact and
  which currently hands a reverser extra structural information for free).
- Recognize that rotating the token per year does not help: the *mechanism* (parse PE, hash
  own `.text`, XOR-decrypt one offset) is what gets reverse-engineered, and once written
  down it is a permanently reusable unlocking script for every future build of this same
  design, not just this one instance.

Not re-fixed this session — this is a finding for the next design pass, not an emergency
patch. The binary still meets every check `build-go.sh` runs; those checks were never
claiming to test this.

## Iteration 5 (2026-09-07) — the dynamic-analysis break, and the fix

A **separate** blind-analysis pass (not the static one above) attacked the iteration-4
Go binary dynamically and won trivially: hooking the emitter directly (Wine API tracing
of `keybd_event`/`SendInput`) returns vk code, up/down flag, and timestamp for every
call as literal integer arguments in the trace log. That is the exact hold-duration
signal, with zero decryption, zero decoding effort, and better precision than the
intended `GetAsyncKeyState`-polling receiver. Confirmed independently this session:
`strings` on the iteration-4 binary returns `keybd_event` and `user32.dll` in plain
ASCII — a static grep alone names the mechanism, before any dynamic step.

All six items below are one integrated redesign, not six independent patches — item 6
(decoys) is what makes items 3 and 4 possible, and item 1 is what makes the whole
exercise worth doing at all. **Go build only.** `sender.c`/`build.sh` (C) are untouched
and still the maintained fallback; scope was bounded to the artifact that ships.

### The redesign

Six ciphertext **slots** replace the single blob plus twelve inert decoy tables. One
slot transmits per pass (six passes, same runtime shape as iteration 4). Two slots —
chosen by `tools/layout.slot_layout(token)`, a hash of the token, no metadata file to
keep in sync — carry the real message under independently salted keys
(`checksum(.text) XOR slotSalt[i]`); the other four are permanent random noise fixed at
generation time. **The Go runtime never learns which slots are real.** `transmitSlot`
runs identically for all six; there is no branch, anywhere, on real-vs-decoy. That one
property is what closes item 6 for both static and dynamic inspection at once.

1. **Emitter.** `strike()` now calls `win32u.dll!NtUserSendInput` — the undocumented
   function `SendInput` itself forwards to on x64 Windows, confirmed present the same
   way in Wine (`objdump -p win32u.dll` lists the export). Resolved via
   `syscall.NewLazyDLL`/`NewProc` using DLL and proc-name strings that exist only as an
   XOR-obfuscated blob at rest and are decrypted into RAM transiently, right before the
   one call that needs them (`decryptStr`, same keystream construction as the welcome
   banner). No `keybd_event`, `SendInput`, or `user32.dll` string exists in the shipped
   file. The `INPUT` struct is hand-assembled into a 40-byte buffer at fixed x64 offsets
   rather than declared as a Go struct, so there is no type in the binary shaped like
   the documented `INPUT` layout.
   **Attack tried:** `WINEDEBUG=+relay` against the new binary, grepped for
   `keybd_event`/`SendInput` — **zero hits** (732 real calls, none named that). Attack
   re-run for the exact technique that broke iteration 4 — **closed.**
   **Attack tried, and this is the honest part:** Wine's relay trace, unlike a
   name-filtered tracer, hooks `KERNEL32.GetProcAddress` generically and decodes its
   second argument as a string — the log shows `GetProcAddress(...,"NtUserSendInput")`
   in the clear, and then one `win32u.NtUserSendInput(00000001,<ptr>,00000028)` call.
   **This item does not defeat a generic "hook GetProcAddress and log resolved names"
   technique** — a well-known, still-generic malware-triage move, not source-specific
   RE. What it does close is the specific, literal attack that broke iteration 4
   (watch the named emitter and read args off the log); what it does not close is one
   step more sophisticated. Worth naming rather than overselling: **raises the bar
   materially, does not remove the dynamic path entirely.** Also worth naming: even
   where the call IS logged, the args shown are a raw pointer and a size — recovering
   vk/edge/timing from that requires dereferencing the pointer (a memory read at the
   call site), which a bare `+relay` log does not do. The old `keybd_event` calls put
   vk and flags directly in the log line as integers; that specific convenience is gone
   even in the residual case.
2. **Plaintext lifetime.** The iteration-4 `unpack()` decrypted and expanded the full
   61-element message into a `[]int` once in `main()`, then kept it — unmodified — for
   the entire ~117s, six-pass run. A memory snapshot at any instant during that run
   contained the whole message as a flat array of exactly two values. `transmitSlot`
   now decrypts one ciphertext **byte** at a time (`curByte`, up to 8 symbols' worth),
   immediately before use, inside the transmit loop itself; nothing holds a whole slot's
   plaintext, let alone the message, at once. This is byte, not bit, granularity — the
   keystream is sequential, so byte-at-a-time is the finest granularity the cipher
   construction supports without pointless per-bit state juggling that buys nothing.
   **Attack tried:** a live `/proc/PID/mem` scan for a run of 61 consecutive
   150/450-valued little-endian int32s (the iteration-4 smoking gun), while the binary
   ran under wine. **Blocked by this sandbox's ptrace/proc-mem restrictions**
   (`PermissionError` even against a same-user process) — an environment limitation of
   this session, not a property of the design, so this claim rests on code review
   rather than a captured dump: trace `transmitSlot`'s scope yourself and confirm
   `curByte` is the only variable that ever holds decrypted bits, reassigned every 8
   iterations. **Re-run the memory-dump attack on real Windows before trusting this
   fully** — it is the one item in this pass that could not be empirically closed here.
3. **Timing.** Every hold/rest now gets a per-symbol jitter offset in `[-35, +35]` ms
   on top of the 150/450 base, drawn from a second xorshift stream independent of the
   decrypt stream. A raw duration list no longer contains suspiciously exact repeated
   millisecond values (a tell in itself: real held keys don't come out to the same
   millisecond six times running). **How a solver reasons about it:** unchanged from
   iteration 4 — `decode_log.py`'s decoder never assumed a fixed unit; it estimates one
   from the 10th percentile of observed holds and classifies by a >2x threshold, which
   is jitter-tolerant by construction. A player following the intended path (write a
   monitor, estimate the unit from what you actually captured) sees no difference.
   **Attack tried:** a Python simulation of the decoder against 200 seeded runs at
   +/-35ms per-symbol jitter — **200/200 decode correctly.** Also ran at +/-50/60/74ms
   to confirm the margin: clean through 50, degrading at 60 (179/200), collapsing at 74
   (83/200) — consistent with the jitter-tolerance figures iteration 4 already
   measured, and 35ms sits with real headroom under wine's separately-measured
   +6.6ms/element scheduling bias.
4. **Repeated transmissions.** The six passes are no longer verbatim. Each transmits a
   **different** slot; only two of six decode to the flag, the other four to
   Morse-shaped noise (confirmed in `verify_runtime.py`'s output below — real slots
   read `HELDNOTTYPED`, decoy slots read garbage like `EEETMETIZETETI<..-..>EITET`).
   Redundancy dropped from 6x to 2x, but it is now genuine cross-check redundancy
   (two independently-keyed ciphertexts agreeing on content) rather than a bare
   majority vote over identical copies. A raw capture of all six passes is no longer
   "the message, six times" — it is six candidate signals, most of which are noise.
5. **`GetTickCount`.** Iteration 4 called it and discarded the result — dead code, a
   red flag under RE (why call it if unused?). It now seeds the per-pass jitter stream
   (`tick() ^ slotSalt[pass]`, item 3). Freezing or patching the clock changes the
   jitter *pattern* but never the underlying bits, so decoding still succeeds either
   way — verified by the jitter simulation above, which covers the frozen-clock case
   too (jitter is pure per-symbol noise; nothing about correctness depends on it moving
   between passes). This is a genuine, if modest, environment-coupling: real elapsed
   runtime now visibly participates in what a capture looks like, addressing pillar 3
   in a small way. It is not claimed to solve pillar 3 the way the iteration-4 red-team
   finding's "not attempted here" list describes (the key itself is still fully static,
   present in the file) — that remains open, and is not this session's scope.
6. **Decoys.** The iteration-4 decoy tables (`decoys.go`, twelve tables, a
   `RunSelfTestSuite` dispatcher) fed a `sink` variable and never reached the emitter —
   exactly the property that made dynamic analysis blind to them. Retired entirely.
   The six-slot scheme above replaces it: every slot, real or decoy, is decrypted by
   the same code, keyed the same way (`checksum(.text) XOR slotSalt[i]`), and
   genuinely transmitted through `strike()`. There is no structural tell between a
   real slot and a decoy slot in either the source or the disassembly — the only
   difference is ciphertext content, which is indistinguishable without decrypting it.
   A dynamic capture of all six passes now contains real decoy signal, not silence.

### Full verification run (this session)

- `./build-go.sh` green end to end: designer-vs-patcher diff agrees for both real slots,
  tamper check confirms one flipped `.text` byte breaks every slot (not just the real
  ones), leak grep passes including the new emitter/DLL-name patterns, symbol grep shows
  31 renamed `main.*` symbols (down from iteration 4's count now that `decoys.go`'s
  routines are gone).
- `verify_runtime.py --slots 6` on the shipped binary: slots 1 and 5 (this build's real
  slots) both read `HELDNOTTYPED`; slots 0, 2, 3, 4 read distinguishable Morse-shaped
  garbage. `Securinets{HELDNOTTYPED}` confirmed from the actual compiled-and-patched
  file, not the generator's opinion of it.
- `WINEDEBUG=+relay` against the shipped binary: `keybd_event`/`SendInput` — zero hits.
  `GetProcAddress`/`NtUserSendInput` — visible (residual gap, documented under item 1).
- Jitter-decode simulation: 200/200 at the shipped +/-35ms.
- Memory-dump attack: blocked by sandbox ptrace restrictions, not attempted to
  completion — open item, see item 2.

### What this pass did not touch, and why

- **The C build.** `sender.c`/`build.sh` keep the iteration-3 design (single blob,
  twelve inert decoy tables, `keybd_event` directly) and remain the fallback if Go
  fails the still-pending VM test (A4, SmartScreen/Defender). It inherits the same
  dynamic-analysis weakness this session fixed in Go and was not re-verified against
  it — **if the C build ever becomes the one that ships, redo this pass against it
  first.**
- **The static/`.text`-checksum weakness from the iteration-4 red-team finding.** Still
  open, unrelated axis (static-only attack, no execution needed) from what this session
  fixed (a dynamic/API-monitoring attack, needs execution). Both can be true at once;
  neither fix implies the other.
- **A4 (SmartScreen/Defender) and A1/A2 (real-Windows verification of injection and
  timing under a real scheduler).** Still open, still needs a real Windows VM.

## Iteration 7 (2026-09-11) — packing: the shipped image is no longer readable

**The freeze was reopened deliberately and for this only.** `iteration6-decision-state.md`
says the binary is final. That still holds for the *mechanism* — nothing in `src/` changed,
the six slots, the `NtUserSendInput` emitter and the Morse concept are untouched, and none
of the five rejected hardening ideas were revisited. What changed is the **shape of the file
on disk**, which was the one axis iteration 6 never addressed and which the iteration-4
red-team finding left explicitly open: *"the Go/garble build is NOT resistant to static
agentic RE."*

garble renames identifiers. It does not stop anyone opening the binary in Ghidra and reading
`derive()`, the FNV-1a loop over `.text`, the xorshift keystream and the slot layout straight
off the decompilation. That is the shortcut this pass closes.

### What was surveyed, and why it was all rejected

| Candidate | Why not |
|---|---|
| **UPX** | Fatal, not merely weak. UPX rewrites the image into `UPX0`/`UPX1` and leaves **no section named `.text`** — `derive()` looks one up by name and bails with `errUnavailable`, so the sample never transmits. `upx -d` also undoes it in one command. |
| **Amber / donut / PEzor** (reflective PE loaders) | Built to defeat AV, not decompilers. They carry anti-analysis behaviour `CLAUDE.md` forbids, and reflectively loading a 2 MB Go image with its own TLS and exception directory is a large fragility bet for no additional static-analysis benefit over encrypting the sections in place. |
| **garble `-literals`** | Already banned in `build-go.sh` and still banned: it rewrites literal data into runtime-constructed form and would destroy the 16-byte marker `patch_blob.py` locates. |
| **Themida / VMProtect / Enigma** | Commercial. Out of scope by rule. |

What shipped instead is a bespoke in-place section packer, `tools/pack_pe.py` plus
`tools/stub.asm`. It keeps the PE structurally ordinary — same section names, valid imports,
ordinary load — and simply makes the bytes unreadable until the entry stub has run.

### The technique stack

1. **garble** — unchanged, still the first layer, still renaming every identifier.
2. **In-place encryption of `.text`, `.rdata`, `.pdata`, `.xdata`.** `.rdata` is the one that
   matters most: it carries the pclntab, the type metadata and every Go string. `.pdata` is
   the quiet one — it is the exception directory, and it hands IDA a complete function table
   even with no symbols at all. Encrypting it is why function recovery collapses.
3. **A 212-byte entry stub** in a new `.boot` section, written in NASM. No imports, no API
   calls, no strings. It recovers the image base from its own RIP rather than trusting
   `ImageBase`, walks a table of `(rva, len, seed)` triples, decrypts each region, and jumps
   to the original entry point.
4. **A key schedule, not a stored key.** Each region's state is
   `seed ^ rotl32(rva,7) ^ (len * 0x9E3779B1) ^ carry`, so lifting the stored 32 bits is not
   enough — the schedule has to be reproduced.
5. **A plaintext-fed carry chaining the regions together.** `carry` is folded from every byte
   of plaintext recovered so far. Region N's key therefore depends on region N−1 having been
   decrypted *correctly*. The cheap attack — lift the `.rdata` seed, decrypt `.rdata` alone,
   read the strings — does not work; you have to do `.text` first and get it right.
6. **ASLR off, relocations stripped.** Not cosmetic and not optional: the loader applies base
   relocations *before* the entry stub runs, which would corrupt the encrypted bytes at every
   relocation site. Clearing `DYNAMIC_BASE`/`HIGH_ENTROPY_VA`, zeroing the base-relocation
   directory and wiping `.reloc` is what makes in-place encryption viable at all.
7. **Header hygiene** — `TimeDateStamp` zeroed.

### Ordering is load-bearing: pack BEFORE patch

The sample keys itself off a checksum of its own `.text` **as it exists on disk**. Packing
changes those bytes, so it changes the key. `build-go.sh` therefore packs and only then runs
`patch_blob.py`, and the comment there says so.

Get this backwards and the failure is silent in the worst way: the binary still builds, still
runs, still prints its banner and still types for two minutes — it just transmits noise.
Nothing but `verify_runtime.py` would notice.

The same applies to the designer build, which is now packed before it is checked. Verifying
an unpacked designer would prove a fact about a binary we do not ship.

### The leak gates had to move, or they would have become theatre

Packing encrypts `.text` and `.rdata`, so every `strings` gate in `build-go.sh` passes
trivially on the shipped file **whether or not the string was ever compiled in**. Checking
only the packed artifact would let a real leak ship behind the encryption.

So the token/branding/emitter-name gates and the garble symbol gate now run on the **unpacked
intermediate**, and a separate new gate runs on the shipped artifact asserting the opposite:
zero `main.*` symbols, zero Go runtime strings, and ≥ 7.9 bits of entropy on the code
sections. One gate proves nothing leaked; the other proves the packing actually happened.

### Measured, not asserted

Same source, unpacked vs shipped:

| | unpacked | packed |
|---|---|---|
| functions recovered (rizin `aaa`) | **1518** | **1** |
| Go runtime strings | 2067 | **0** |
| `main.*` symbols | 32 | **0** |
| `.text` entropy | 6.26 | **8.000** |
| `.rdata` entropy | 5.65 | **8.000** |
| `.pdata` entropy | 5.11 | **7.989** |

Behaviourally, via `tools/capture_wine.py` — which relay-traces the real
`win32u.NtUserSendInput` calls, pairs them into hold durations and feeds them to the same
reference decoder the answer key uses — both builds decode to the same message from genuinely
emitted keystrokes. Jitter is seeded from `GetTickCount`, so the millisecond values differ
every run by design; **the invariant to compare on is the decoded message, never the raw
timings.**

### What this does NOT close

Stated plainly, because the challenge already has one honestly-recorded ceiling and this adds
a second.

- **The stub is 212 bytes and its key schedule is in the clear.** A competent analyst reads
  it, writes a static unpacker and recovers the *entire* original binary — garble-renamed but
  otherwise complete. Well under an hour's work. Packing raises the price of static analysis;
  it does not remove static analysis from the board. Anyone who claims this binary "cannot be
  disassembled" is wrong.
- **47 `kernel32` imports are still in plaintext and always will be.** `.idata` has to stay
  readable because the OS loader resolves imports before any code of ours runs.
  `WerSetFlags`, `TlsAlloc`, `WaitForMultipleObjects` and friends are a recognisable Go
  runtime import set. So "no recognisable Go runtime" is true of the strings, the pclntab and
  the function table — and **false** of the import table.
- **The payload marker is still findable.** The 16 bytes `E1..E8 5A..61` and the 48 bytes of
  slot ciphertext behind them sit in plaintext `.data` (currently file offset `0x1e62e0`), and
  against an otherwise-noise file they stand out more than they used to. This is deliberate:
  `.data` also contains the IAT, which the loader writes into before the stub runs, so
  encrypting it means carving the IAT range out — and it would break `patch_blob.py` and
  `verify_runtime.py`, which are the tools that make this build trustworthy. Destabilising
  verified tooling for a marker that reveals nothing decryptable was judged a bad trade. It
  points at the payload; it does not help anyone decrypt it.
- **Nothing about the runtime ceiling changed.** The plaintext is in memory the moment the
  stub finishes, and the decrypted durations still reach the emitter. Every dynamic attack
  that worked at iteration 5 works identically now. This pass is aimed at the static path
  only.
- **AV/SmartScreen risk went UP, and is untested.** Two ~8.0-entropy sections, RWX `.text`,
  ASLR off and an entry point in the last section is a textbook packer profile. A4 was already
  an open item needing a real Windows VM; it is now a bigger one. **Distribute in a
  password-protected zip and test the download path on real Windows before the event.**

### Rules for anyone touching this later

- **Never reorder pack and patch.** See above. The failure is silent.
- **`tools/stub.asm` and the `Chain` class in `tools/pack_pe.py` are one algorithm written
  twice.** If they drift the packed binary does not run at all — which is the failure mode we
  want, and `pack_pe.py` additionally round-trips every region through its own decryptor
  before writing. Do not "optimise" one side alone.
- **Do not encrypt `.idata`, `.reloc` or `.data`.** The first two are read by the loader
  before the stub exists; the third contains the IAT and the patch target.
- **Do not add anti-debug, anti-VM or unpacking detection to the stub.** The intended solve is
  to run this thing and watch the keyboard. `CLAUDE.md` is explicit: harden against the
  shortcut, never against the lesson.
- **`tools/winesafe.sh` exists for a reason.** The build used to run the real sample under a
  bare `wine`, and on a Wayland session that types two minutes of keystrokes into the
  developer's actual desktop. Every wine invocation in the build now goes through it, with
  both `DISPLAY` and `WAYLAND_DISPLAY` unset and the graphics drivers disabled.

## Iteration 8 (2026-09-24) — the shipped binary injects nothing

**Found from player reports: nobody sees a single `k`.** It is not a hinting problem, not a
monitor problem and not a Windows-only problem. The sample presses no keys at all, and has
not since the emitter moved to `win32u` in iteration 5.

### The finding

Relay-traced run of the shipped `build/KeyboardSelfTest-go.exe` (`0ecc40b2`):

```
372 calls to win32u.NtUserSendInput   ->   372 x retval=00000000  ("Invalid handle")
```

Every injection is rejected. The banner prints, the timing is perfect, the passes are
correctly spaced, and not one keystroke reaches the system.

**Cause: the process never loads `user32.dll`.** Its import table is `kernel32.dll` and
nothing else; `resolveSend()` loads `win32u.dll` alone and calls the syscall stub directly.
`NtUserSendInput` needs the calling process to have been initialised as a GUI client, which
is what loading `user32` does. Isolated with `tools/probe_emitter`, same process, same run,
only difference being whether `user32` is in the process:

| | result |
|---|---|
| `win32u.NtUserSendInput`, `user32` not loaded | **0** — Invalid handle |
| `win32u.NtUserSendInput`, `user32` loaded, never called | **1** |
| `user32.SendInput` | **1** |

Loading it is enough; no `user32` call has to be made. The earlier wine capture that appeared
to prove the emitter worked was reading a second process in the trace — wine's own loader
stub, which does pull in `user32`. The sample's own process (distinct thread id, distinct
module base) loads `win32u` only.

### Why every gate passed

`strike()` discards the return value of `SyscallN`, so the sample cannot tell an accepted
event from a rejected one, and neither can anything watching it. `capture_wine.py` then
timestamps *calls into* the emitter and never looks at what they returned — it measures
intent, not effect. So "it runs, and it still types the same message" was true of the calls
and false of the keyboard, and the answer key decoded a message that was never transmitted.

**A gate that observes the sender cannot verify a receiver.** The only honest behavioural
check reads the key state back, the way a player does.

### The fix, verified

Load `user32.dll` in `resolveSend()` before resolving the emitter — name kept xorshift-
encrypted like the other two, so the build's leak gates stay meaningful. Rebuilt from
`src/go` with that one change: **85/85 emitter calls returned 1.**

This costs nothing that iteration 5 bought. The sample still never *calls* `user32.SendInput`,
so hooking `user32.SendInput` still yields nothing, which was the whole point of moving to
`win32u`. It only stops lying to the kernel about being a GUI process.

### Three more things that make it unplayable, all separate from the above

1. **No grace period. First keystroke lands 146 ms after process start.** Double-clicking
   the exe creates a console window which *is* the foreground window, so the opening
   keystrokes go into the sample's own console. "Put an empty Notepad in front before you
   start it" cannot be satisfied — starting it is what takes the focus away. It needs a
   visible countdown before the first pass.

2. **Four of the six passes are noise, and the challenge page says they are repeats.**
   Real slots are `[1, 5]`; passes 1, 3, 4 and 5 (1-indexed) are the permanent decoys from
   iteration 5. Decoded, pass by pass, from a full 121 s capture:

   | pass | t | decodes to |
   |---|---|---|
   | 0 | 0.1–17.6 s | `T<.--.-><...-..>TNTNEE<--.--.>EEAE` |
   | 1 | 19.7–36.3 s | **HELDNOTTYPED** |
   | 2 | 39.3–55.9 s | `MEKYTMOYAEIBN` |
   | 3 | 58.9–75.6 s | `WDUEMEEQKHATIT` |
   | 4 | 78.6–95.2 s | `TOE<---->NEEMBETWEETP` |
   | 5 | 98.2–114.9 s | **HELDNOTTYPED** |

   A player who captures one pass has a 2-in-6 chance of a readable message, and the copy
   tells them to relax because "the next pass is coming". Decoys as *transmitted* content
   is a deliberate iteration-5 decision and the mechanism is sound; the player-facing
   promise that contradicts it is the bug. Either the copy says so plainly, or the real
   slots bracket the run.

3. **Failure is invisible and looks like success.** Both bail-outs (`derive`,
   `resolveSend`) print `self-test unavailable on this system` to **stderr** and
   `os.Exit(1)`. Double-clicked, the console closes before it can be read, which is exactly
   the "it finishes running so quickly" half of the report. The message belongs on stdout,
   and the process should not vanish on the way out.

Separately, `Friendly-CTF-2026/reverse-engineering/straight_key/challenge.yaml` carries none
of the operating instructions from `notes/challenge-page.md` — no Notepad, no two minutes,
no wait-for-the-next-pass. Players on the platform were never told any of it.

### Applied and rebuilt (same session)

Scope was the user's call: emitter fix, countdown, loud failure. **The decoy design and the
player-facing copy were deliberately left alone** — a single captured pass is still readable
2 times in 6, and the page still calls the passes repeats. That is a known, accepted gap, not
an oversight; do not "fix" it without asking.

What changed:

- `tools/gen_payload.py` emits `clientBlob`/`clientSeed` for `user32.dll`, xorshift-encrypted
  under `0x6C078965` exactly like the other two names. A plaintext `user32.dll` next to two
  encrypted ones would point straight at the emitter and undo iteration 5 item 1. Verified 0
  occurrences of `user32` in the unpacked image, and `user32` is now in `build-go.sh`'s leak
  gate alongside `win32u`.
- `resolveSend()` loads it before resolving `NtUserSendInput`. Load only — no call needed.
- `strike()` returns the syscall result. `hold()` counts rejections, `transmitSlot()` returns
  on the first one and `deliver()` fails with `errRejected`, so a blocked emitter now dies
  ~150 ms in instead of miming for two minutes.
- `standby()` prints a 10-second countdown before the first pass, so the player has time to
  focus a window. Fixes the 146 ms start.
- Errors go to **stdout**, and `linger()` holds the process 20 s before exit so a
  double-clicked console can be read. `deliver()` prints `Self-test complete.` on success, so
  finishing is distinguishable from dying.
- `capture_wine.py` now parses `Ret ... retval=` and **fails the build if any injection was
  rejected**. This is the gate that would have caught iteration 7 and did not exist.

Measured on the rebuilt artifact, `964280d4b3f23280ffde8b9f03fdc2ddfd4834476a088300d7e9fa7a70876f2b`
(md5 `381d3ca2`), full 135 s run:

| | 2026-09-11 build (`0ecc40b2`) | rebuilt (`964280d4`) |
|---|---|---|
| emitter calls | 372 | 372 |
| **accepted** | **0** | **372** |
| passes decoded | 6 (2 real) | 6 (2 real) |
| wall clock | 121 s | 135 s (10 s countdown + 125 s) |

Every other gate is unchanged and green: both real slots agree with the patcher, the tamper
check breaks every slot on one flipped code byte, `.text`/`.rdata` entropy 8.000, 0 `main.*`
symbols, 0 Go runtime strings. The artifact is copied to
`Friendly-CTF-2026/reverse-engineering/straight_key/attachments/KeyboardSelfTest.exe`.

**The C build at `build/KeyboardSelfTest.exe` was not touched and does not ship.** It uses
`keybd_event`, which lives in `user32` and therefore loads it implicitly, so it is probably
not affected by this particular defect — but that is an inference, not a measurement, and it
still carries iteration 4's dynamic-analysis weakness.

**Still open: A4.** AV and SmartScreen on real Windows are untested, and the packer profile
(RWX `.text`, entropy 8.000, relocations stripped, entry in `.boot`, kernel32-only imports)
is exactly what heuristics look for. A2, A3 and the real-device solve with
`solution/keyboard-monitor.ps1` also still need a Windows VM. **Wine now agrees the keystrokes
are accepted; it cannot tell you Defender will let the process live.**

### Rules this adds

- **Never trust a sender-side capture again.** Any change to the emitter is verified by
  reading key state back, not by counting calls. `tools/probe_emitter` is the minimum bar
  and `solution/keyboard-monitor.ps1` on real Windows is the real one.
- **Check the emitter's return value.** A rejected injection must fail loudly at build time,
  and must not be silently repeated 186 times at run time.

# Straight Key — CLOSED. Shipping at iteration 5 (2026-09-07)

**This challenge is finished.** The release artifact is `build/KeyboardSelfTest-go.exe`,
attached to players as `KeyboardSelfTest.exe`. No further hardening is planned and none is
outstanding — the hardening question was worked to a conclusion and closed, it was not
deferred. Player-facing copy is final in `challenge-page.md`.

This file records why the design landed where it did, so the reasoning survives without
anyone re-deriving it.

## Where things actually stand

Iteration 5 (six-item hardening: win32u emitter, per-byte plaintext lifetime, jittered
timing, non-verbatim repeats, real GetTickCount role, genuinely-transmitted decoys) is
**built, verified, and shipped** as `build/KeyboardSelfTest-go.exe`. That work is done
and is written up in `notes/design.md` under "Iteration 5". Nothing has been committed
to git this session (standing rule: only commit when asked).

## Blind-test result (the actual empirical check)

A genuinely fresh subagent, given ONLY the compiled exe + a stripped player brief (no
hints, no source, no repo access), solved it: recovered `Securinets{HELDNOTTYPED}`,
confirmed correct. Full accounts are in the conversation transcript, not re-copied
here in full, but the headline findings:

- **Item 1 (emitter hiding) held genuinely.** The agent's first two moves — hooking
  `user32.SendInput` and `user32.keybd_event`, which is what API Monitor's stock
  profiles and nearly every Frida-on-Windows snippet target — returned **zero events**
  across ~290 seconds. It only found `win32u.NtUserSendInput` after escalating to an
  unfiltered full-process relay trace against a pre-warmed wineserver; a broad
  `user32.*`/`win32u.*` filtered attempt actually deadlocked the sample. Real,
  confirmed friction against the specific "trivial API-trace" shortcut this whole
  iteration was built to close.
- **Item 6 (decoys) did NOT hold.** The agent decoded all 6 timing blocks and picked
  the real one by pure statistics — decoded bit-noise has a ~50/50 dot/dash run-length
  distribution, real Morse-encoded text doesn't. It never touched the checksum/salt
  derivation mechanism at all. This is a confirmed, not theoretical, gap: "understand
  and reconstruct the mechanism" was NOT required to win.

## Two proposed fixes for item 6, both rejected — and why

1. **"Make decoys decode to plausible-looking words instead of noise."** User rejected
   this correctly: the answer space is still 6 fixed strings against a cheap-retry
   CTFd grader, so "decode everything, submit everything" still wins regardless of how
   convincing the fakes look. This is exactly the class of thing `CLAUDE.md` already
   names and rejects ("these slow a human roughly as much as an agent, so they don't
   change relative difficulty"). Confirmed dead end, don't revisit without a genuinely
   different angle.
2. **"Use AES instead of the XOR keystream."** Also rejected, same reasoning: neither
   attack that actually worked (memory-dump of decrypted plaintext at emit time;
   statistical decode of the timing signal) touches the cipher primitive at all. The
   key is derived the same way either way (checksum of `.text`, computed identically
   at build and runtime), and the plaintext that reaches `NtUserSendInput` is identical
   regardless of what encrypted it at rest. Classic crypto-theater — would cost real
   engineering time for zero change in either demonstrated attack.

## The one structural idea on the table, NOT YET BUILT

**One randomly-selected slot transmitted per execution, instead of all 6 every run.**
Selection uses real OS entropy at startup (not derivable from the file), and the
chosen slot repeats a few times within that run for reliability — but a single run
never reveals more than one candidate.

Why this is structurally different from the two rejected ideas: it doesn't try to make
guessing harder per-guess, it makes **each guess cost a full run** — relaunch the
sample, sit through the ~117s transmission, decode, submit, and (if doing the malware
analysis "properly") reset the VM snapshot before the next attempt. That's minutes of
real wall-clock and environment churn per guess, which is the thing `CLAUDE.md`'s own
doctrine already flags as something "agents handle badly" (real elapsed time,
environment-coupled state). Genuine reverse engineering — recovering the
checksum-of-`.text` derivation and per-slot salts — stays O(1): compute which slot
index is real without needing to run anything repeatedly at all. That's the asymmetry
that was missing from both rejected ideas.

Known costs, not yet resolved:
- Needs a designer-mode override (env var or similar) to force a specific slot for
  testing/hinting — mandatory per `CLAUDE.md`'s standing rule for any session-seeded
  or timing-dependent module.
- Changes player experience materially: no more "one tidy run, 6 blocks, decode them
  all." A player may need multiple runs to even see a real candidate. Needs honest
  treatment in `notes/challenge-page.md` if built.
- Recommended to pair with a **CTFd-side attempt-limit/cooldown** on this specific
  challenge as a complementary, cheap, server-side lever — matches the project's own
  "verification is server-side" principle better than anything built into the binary.
- Also worth noting: this was raised, in conversation, as an idea to *evaluate*, not
  a committed plan. It has NOT been scoped in file-by-file detail the way iteration 5
  was before that build started.

## Also surfaced, informational, not yet acted on

- A blind-test run of `KeyboardSelfTest-go.exe` under Wine silently picked the Wayland
  driver despite `xvfb-run`, which would have sent real keystrokes to the actual
  desktop rather than the isolated VM. Worth a line in `challenge-page.md`'s
  "before you run it" section (need `WAYLAND_DISPLAY` unset + a dedicated Xvfb prefix)
  regardless of what happens with the slot-selection question above.
- Confirmed via direct binary inspection (not assertion) that the shipped exe contains
  zero occurrences of "morse", "Securinets", "CTF{", the literal token
  "HELDNOTTYPED", or any of keybd_event/SendInput/win32u/NtUserSendInput in plaintext;
  all `main.*` symbols are garble-renamed.

## DECIDED 2026-09-07: SHIPPING AT ITERATION 5

The challenge is **kept and shipped**, the concept is **not changing**, and the binary is
**final**. Iteration 5 is the release artifact.

The accepted trade, stated plainly so nobody reopens it by surprise: a capable agent with a
real Windows VM can solve this challenge. That was worked at length and is a property of the
format, not a defect in the build — a one-string offline flag can only ever require the
player to *obtain* the string, and obtaining it is signal decoding, which is what agents are
best at. What iteration 5 did buy is real and was confirmed by blind test: the trivial
headless API-trace shortcut is closed. That is the win, and it is enough.

Ideas raised and rejected in this session, so they are not re-proposed:

- **Turn it into a question bank** (like case 01). Rejected — user does not want this
  challenge to be a bank. Also carried a serious leak risk: any question naming the emitter
  would have converted the API-hook *shortcut* (which item 1 exists to close) into the
  mandatory path.
- **Split the flag** so the transmitted text is only one ingredient and a second value must
  come from the mechanism. Rejected — dilutes the concept the user actually wants.
- **Encode the message in OS keyboard auto-repeat** (message lives in what the OS delivers,
  not what the binary sends). Rejected on evidence, not taste: injected input via
  SendInput/NtUserSendInput does **not** generate typematic repeat — Windows produces repeat
  in the keyboard driver stack from a physically held key. And even if it did, the binary
  could only produce repeats by sending them itself, which puts the message straight back
  into the emitter output and defeats the entire point. Self-defeating; do not revisit.
- **One randomly-selected slot per execution** (the idea this file was originally written
  about). Not rejected on merit, but not built — it degrades the "one tidy run" player
  experience the user likes, and it was overtaken by the decision to ship.

### The one idea that was not refuted — recorded, not planned

Kept only so it is not re-invented from scratch. This is **not** outstanding work and the
challenge does not need it.

**Make the emitter shortcut lossy rather than blocked.** Alternate symbols across two
different injection paths. A player who actually receives keystrokes gets the complete
stream; an agent that hooks one API gets roughly half the symbols and a plausible-but-wrong
decode rather than an obvious failure — and the statistical slot-selection attack breaks too,
because it runs on a mutilated signal. This is the only proposed idea that attacks the
*demonstrated* attack without touching the concept.

Its cost, and the reason it was not built: both paths must surface to the **same kind of
receiver**, and there is no cheap pair that does. `keybd_event` and `SendInput` both funnel
into `NtUserSendInput`, so hooking that one point catches both and the pairing buys nothing.
Paths at different levels (e.g. `PostMessage(WM_KEYDOWN)`, invisible to a low-level keyboard
hook) would break the intended student solve. That leaves the exported `win32u.NtUserSendInput`
paired with a **direct syscall** to the same kernel routine — which means build-specific
syscall numbers and near-certain breakage under Wine, on an audience running assorted Windows
versions. Real engineering with real portability risk, for a challenge that already works.

### Why not cut the decoys

Considered and declined. Cutting them is cosmetic — it does not make the challenge harder,
it only removes a mechanism a solver bypasses without understanding. Not worth a rebuild and
re-verification cycle on an artifact that is currently verified and working.

## How the question was settled

The question that opened this file was: *"if we don't come to a solution to make this solved
as I intended, I'm thinking of dropping the challenge entirely."*

It was resolved as **keep and ship**. Of the three routes originally listed — build the
one-slot-per-run redesign, find a different structural angle, or accept the format has a
ceiling — the second was tried hardest and produced four candidate angles, all recorded
above, none of which survived. The conclusion is the third route, minus the drop: the format
has a ceiling, the ceiling is acceptable, and the challenge ships as it stands.

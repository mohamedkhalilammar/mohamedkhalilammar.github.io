# Dead Air — design notes

The capstone of the beginner ladder, and the only one where the player has to **build a piece of
the challenge themselves**. A binary checks in with a depot that no longer exists; the player
makes the depot exist.

## Mechanism

The client resolves `relay.meridian-fs.tn:8080`, and on this planet that name does not resolve.
The player points it at their own machine (`/etc/hosts`), stands up a listener, and sees the
binary's actual HTTP request — which carries `X-Relay-Expect: <token>`. **The binary tells the
server what it wants to hear, but only a server gets to hear it.** Echo the token back in the
response body and the check-in is acknowledged; the token is the key the flag is sealed under.

## Decisions that are load-bearing

- **The hostname is deliberately readable and `build.sh` asserts it stays that way.** It is the
  pointer to step one. Sealing it would convert a fair task into "guess where I hid the penny",
  which `docs/CTF-DESIGN-GUIDELINES.md` names explicitly as negative inspiration.
- **Port 8080, not 80.** Unprivileged, so no player needs `sudo` to listen, and nobody loses an
  hour to a permission error that has nothing to do with the challenge.
- **The token check is `strstr` over the whole reply, not a strict HTTP parse.** `nc -l -p 8080`,
  reading the token off the screen and typing it back, is a complete solve. A strict parser would
  force players into writing a real HTTP server and would punish them for a missing
  `Content-Length` — work that teaches nothing this challenge is about.
- **Every failure path names the next step**: cannot resolve → resolves but nothing is listening
  → connected but said nothing → replied without the token. That sequence is the challenge's
  breadcrumb trail, and it is why a beginner can finish this without a hint.
- **30-second receive timeout.** Long enough to type a token by hand, short enough that a
  forgotten listener does not hang forever.
- **Verification runs against the shipped binary, not a test build.** `unshare -r -m` gives an
  unprivileged mount namespace, so `build.sh` bind-mounts its own `/etc/hosts` for the duration
  of one process. The thing tested and the thing downloaded are the same bytes.
- **Designer mode (`RELAY_HOST`) is compiled only into `build/relay-designer`, which must never
  ship.** `build.sh` asserts the override string is absent from the shipping build. It exists
  because CLAUDE.md requires a known-state mode for every environment-coupled module, and for
  testing where user namespaces are unavailable.

## Hardened against static extraction (2026-09-21)

The first version narrated its own solve and kept the hostname in `strings`. Both were pulled on
the user's call: **players use assistants, so a binary that explains itself is a binary that is
already solved.** What replaced the narration is *observable state*, not silence —

- **One failure line, `relay: check-in failed.`**, for every failure. `build.sh` asserts the old
  guidance strings ("cannot resolve", "nothing answers", "did not carry", "decommissioned",
  "Checking in with") never come back.
- **Distinct exit codes** carry the progress the messages used to: 2 did not resolve, 3 nothing
  listening, 4 connected but silent, 5 replied without the token. A player who checks `$?` gets
  real feedback; one who does not, gets nothing. That is the trade: signal stays, hand-holding goes.
- **The hostname is sealed too.** `strings` no longer gives step one; a syscall trace does, because
  glibc puts the name in cleartext into the DNS query it emits. `build.sh` runs `strace` every
  build and fails if the name is not recoverable that way — sealing it is only fair while that
  holds.

**The seeds are derived from the binary's own machine code, not from constants.** `code_key()`
hashes 64 bytes at `unwrap` and XORs the result into both the hostname and token seeds. This is
the part that makes static recovery expensive: there is no literal to lift out of a decompiler
listing. An attacker has to notice the function is hashing itself, extract exactly those bytes at
exactly that offset, and reproduce FNV over them — or just run the binary, which is the intended
path. `build.sh` asserts the naive attack (copy the sealed blob and the visible `TOKEN_SEED`, XOR
them) yields garbage.

That forces a **two-pass build**: seal under a placeholder, compile, hash the compiled `.text`,
re-seal under that hash, recompile. Pass 2 changes only `const` array contents in `.rodata`, so
`.text` must come out byte-identical — **and the build asserts it does**, because if it ever
drifts the shipped binary computes a seed the build did not seal under and the challenge is
silently unsolvable. The designer variant has different code and therefore its own hash, so it
gets its own full two-pass build.

**What was deliberately NOT added: anti-debug.** `strace` is now required to find the hostname, so
any ptrace detection would destroy the intended path. Same rule as the game track — harden the
shortcut, never the lesson.

### Gotcha that cost real time

A quoted `#include "payload.h"` searches the *including file's own directory first*. A leftover
`src/payload.h` from the pre-hardening build silently shadowed the freshly sealed header in
`build/gen-ship/`, so the binary shipped sealed under the old constant seed and failed to resolve
anything. `build.sh` now refuses to run if `src/payload.h` exists.

## Accepted residual

Static recovery is now expensive but not impossible: an attacker who emulates the binary, or who
correctly reproduces the self-hash, still gets the token and with it the flag offline. That is
inherent — the binary must be able to compute its own token with no network, so anything it can
compute, a faithful emulator can compute. What the hardening buys is that **reading the
decompiler output and reimplementing it is no longer enough**, which is precisely the shortcut the
session-32 red team used against the Android track. Same class of accepted residual as Straight
Key's one-string offline flag, but priced much higher.

## Ship note

Players must be told the binary makes an outbound connection to a name that does not resolve, and
that this is intended — otherwise the first assumption will be "the challenge server is down",
which is the single most expensive false belief a networked task can create.

## 2026-09-24 — the silence is reversed, on purpose

**Every failure stage now tells the player what happened and what to do about it.** This
undoes the one exception `re-track/README.md` carved out for this challenge, and it was the
user's call, not a drift. The reasoning that produced the silence was that the exit status
already carried the progress signal (2 resolve, 3 connect, 4 no reply, 5 wrong token) and
that noticing it was part of the exercise. In practice nothing ever told a player the exit
status was a channel at all -- the description only gestured at "the difference between the
ways it can fail" -- so on the first rung of a beginner ladder it read as no feedback rather
than as subtle feedback.

The exit codes are unchanged and `build.sh` still asserts 2 and 3; they are simply no longer
the player-facing channel, and the description no longer mentions them.

**The line that must not be crossed is naming the two sealed facts.** No message contains the
hostname or the token. Stage 2 says a host could not be resolved and to go find out which
name it wants -- it does not say `relay.meridian-fs.tn`, so `strace` is still the first move
and the lesson survives. Stage 3 does name port 8080, which is a deliberate concession: the
port is the least interesting secret in the challenge and being stuck on it teaches nothing.
Stage 5 names the `X-Relay-Expect` header, which by then the player has already received.

`build.sh`'s "it says nothing it does not have to" gate is replaced by one that asserts the
guidance is *present*, so a later edit cannot quietly restore the silence. The leak check
above it already proves the hostname and token do not survive in the binary, which is the
constraint that actually matters.

**Also corrected here:** the description claimed nothing it needs is privileged. That is true
of the listener on 8080 and false of the `/etc/hosts` edit. `HOSTALIASES` was tested as the
unprivileged alternative and does not work -- `getaddrinfo` ignores it, the run still exits 2.
The description now says plainly that listening needs no root and pointing a hostname does.

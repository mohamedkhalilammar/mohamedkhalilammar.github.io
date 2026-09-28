# Iteration 5 hardening — in-progress session state

Scope: Go build only (`src/go/`, `build-go.sh`). C build (`sender.c`, `build.sh`)
intentionally untouched, stays the fallback, per existing project convention.

Concrete pre-fix finding confirmed this session (not theoretical): `strings` on the
CURRENTLY SHIPPED `build/KeyboardSelfTest-go.exe` returns `keybd_event`, `user32.dll`,
`GetTickCount`, `kernel32.dll` in plain ASCII. A static grep alone names the mechanism.
This is on top of the dynamic finding the user reported (API-tracing the emitter call
gives exact hold durations directly, no decryption needed).

## Design (all six items, one integrated redesign)

Six ciphertext "slots" replace the single blob + 12 inert decoy tables. One slot is
transmitted per pass (6 passes, unchanged runtime shape/timing budget). Two slots
(chosen by `tools/layout.slot_layout(token)`, hashed from the token, no metadata file)
carry the real message under independently salted keys; the other four are permanent
random noise. The Go runtime treats all six identically — no branch anywhere on which
is real (fixes #6: decoys now genuinely hit the emitter, and there is no structural
tell to spot statically).

1. **Emitter** — `win32u.dll!NtUserSendInput` (undocumented, one syscall below
   `SendInput`), resolved via `syscall.NewLazyDLL`/`NewProc` using DLL/proc name
   strings that are only ever plaintext transiently in RAM (XOR-obfuscated at rest,
   same keystream scheme as the welcome banner). No `keybd_event`/`SendInput`/
   `user32.dll` string anywhere in the image. Confirmed present in Wine's
   `win32u.dll` (`NtUserSendInput` export verified via `objdump -p`).
2. **Plaintext lifetime** — per-pass, per-BYTE decrypt-on-demand in the transmit
   loop (one ciphertext byte → one plaintext byte → up to 8 symbols' worth of
   duration values, `curByte` a single local var), never a persisted 61-element
   `[]int`. Old code kept a fully-expanded 61-element array alive for the entire
   ~117s run; new code holds ≤1 byte (~8 bits) at a time.
3. **Timing** — per-symbol jitter (±35 ms) added to the 150/450 base, generated
   from a second xorshift stream seeded `tick() ^ slotSalt[pass]`. Ratio-based
   decode (dash threshold 2.0x) tolerates this; prior testing established clean
   decode through ±60ms, collapse at ±74ms — 35ms leaves real margin against wine's
   measured +6.6ms/element bias too.
4. **Repeated transmissions** — no longer verbatim. Each of the 6 passes transmits
   a DIFFERENT ciphertext (different slot); only 2 of 6 decode to the flag, the
   other 4 decode to Morse-shaped noise. Redundancy is 2x, not 6x, and no two
   passes are byte-identical.
5. **GetTickCount** — real role now: seeds the per-pass jitter stream (item 3).
   Freezing/patching it changes the jitter pattern but not the underlying bits, so
   decode still succeeds — verify this empirically, don't just assert it.
6. **Decoys** — genuinely transmitted (see above), same derive/decrypt code path,
   same key-derivation source (`.text` checksum XOR slotSalt), zero runtime branch
   on real-vs-decoy. Old `decoys.go`/`RunSelfTestSuite` (12 tables that fed a sink
   and never reached the emitter) is retired for the Go build.

## File changes needed (tracked here so a resumed session doesn't have to re-derive)

- [x] `tools/layout.py` — new, shared slot layout.
- [x] `tools/gen_payload.py` — go branch rewritten for 6 slots + dll/proc blobs.
- [ ] `tools/patch_blob.py` — add `--slots N` (default 1, back-compat for C);
      when >1, use `layout.slot_layout(token)` to find real slot offsets + salts,
      patch each real slot with its own key (`checksum ^ salt[i]`).
- [ ] `tools/verify_runtime.py` — same multi-slot awareness; check both real slots
      decode to the expected token.
- [ ] `src/go/main.go` — drop `plan`/`scratch` fields, drop `unpack()`, drop
      `warmup()` stage (decoys.go retired), keep `derive()`/`tick()`/`stored()`
      (stored() now returns the whole `numSlots*patBytes` region), add
      `slotSeed(i)`, make `next()` a wrapper around a free `next(state *uint32) byte`.
- [ ] `src/go/transmit.go` — full rewrite: resolve NtUserSendInput dynamically,
      build the 40-byte x64 `INPUT` struct by hand (offsets: type@0 u32, pad@4,
      wVk@8 u16, wScan@10 u16, dwFlags@12 u32, time@16 u32, pad@20, dwExtraInfo@24
      u64, unused@32..39 — matches known sizeof(INPUT)=40 on x64), per-pass
      per-byte decrypt loop with jitter as above.
- [ ] `src/go/designer.go` — print base (unjittered) durations for the FIRST real
      slot only, so build-go.sh's designer-vs-patcher diff still gates correctness.
- [ ] delete `src/go/decoys.go` (Go-only; `tools/gen_decoys.py`'s `--lang c` path
      and `decoys.h` for the C build are UNTOUCHED).
- [ ] `build-go.sh` — drop the `gen_decoys.py --lang go` call; add `--slots 6` to
      the `patch_blob.py`/`verify_runtime.py` invocations; keep every existing
      verification stage (designer diff, tamper check, leak grep, symbol grep) —
      extend the leak grep to also check for `win32u`, `NtUserSendInput`,
      `SendInput`, `LoadLibrary` plaintext (should all be absent/encrypted-only
      except LoadLibrary which is a legitimate generic call, fine either way).

## Verification / attack plan (after it builds)

1. `build-go.sh` green (designer diff, tamper check, leak grep, symbol grep).
2. `strings`/`strings -el` on the new binary: confirm no `keybd_event`, `SendInput`,
   `win32u`, `NtUserSendInput`, token, `HELDNOTTYPED` anywhere.
3. `WINEDEBUG=+relay wine build/KeyboardSelfTest-go.exe 2>relay.log` — grep the
   log for `user32.*keybd_event|SendInput` (should be ABSENT) and for
   `win32u.*NtUserSendInput` (may or may not appear depending on whether wine's
   relay tracing covers win32u.dll at all — record whatever actually happens,
   do not assume).
4. Memory: run the binary under wine, get its PID, dump `/proc/PID/mem` (or use
   gdb attached to the wine process) partway through a run, grep for a run of ≥8
   consecutive `150`/`450`-valued little-endian int32s (the old smoking gun) —
   should not be findable at that scale any more.
5. Python jitter-decode simulation (reuse `decode_log.py` + `encode_morse.py
   --jitter`) to confirm ±35ms per-symbol jitter still decodes correctly, N seeded
   trials.
6. Re-attempt the ORIGINAL "trivial" attack (hook keybd_event/SendInput args+time)
   — confirm it now returns nothing.
7. Try the NEXT-easiest generic attack against the NEW design (e.g. hook
   `GetProcAddress`/`LoadLibraryA` args to recover "win32u.dll"/"NtUserSendInput"
   names, or hook every export of win32u.dll generically) — report honestly
   whether this still works; it is expected to, and that's fine, name the residual
   gap rather than oversell.
8. Report format required by the user: what changed / why / attack used per item /
   still works? / remaining trivial path / intended solve / build+run commands /
   design.md + PROGRESS.md updates.

Token: `HELDNOTTYPED` (unchanged, `src/token.txt`). Flag: `Securinets{HELDNOTTYPED}`.

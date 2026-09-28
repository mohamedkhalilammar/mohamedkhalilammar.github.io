# SecureStore — hardening item #2, renamed from FlagVault (2026-09-11)

Originally moved the five FlagVault flags (challenges 3, 4, 5, 6, 8) from
Kotlin XOR-masked byte arrays into `flagvault.c`. The Kotlin byte arrays
were recoverable with jadx + one line of Python — the mask sat right next
to the ciphertext in the same class. That's exactly the "static flag string
in the APK" non-negotiable #1 rules out.

**Renamed 2026-09-11** as part of the project-wide naming pass and the
structural-distinguishability fix (see `docs/CHALLENGE-AUDIT-2026-09-11.md`,
"PROJECT-WIDE: naming pass" and "PROJECT-WIDE: the flag row is structurally
distinguishable in source"). `libflagvault.so` and the class `FlagVault`
were themselves a leak — a file/class name that says "the answers are in
here" before any decompilation happens — and the five accessor names
(`frontDoor`, `looseLips`, `evidenceLocker`, `internalToken`, `faceValue`)
were the challenge codenames spelled out as method names. Both problems are
now gone: the library is `libsecurestore.so`, the class is `SecureStore`,
and it exposes exactly one opaque accessor, `external fun read(slot: Int)`.

**Also folded in:** every call site that used to insert the flag as the one
function call among a wall of literal decoys (`LocalStore`'s settings rows,
`BackupManager`'s log lines, `AppBridge`'s single `getInternalToken()`) now
routes every decoy value through the same `SecureStore.read(slot)` call as
the real one. The native library now serves many slots, not five — most of
them decoys that were previously plain string literals in Kotlin. See the
audit's structural-distinguishability section for why this mattered: a
function call standing alone among literals is as loud a signpost as a
descriptive method name.

## Scheme (unchanged)

Copied vaultcrypto.c / sirr_unseal's pattern rather than inventing a new one
(explicitly requested — stay consistent with how this project already does
runtime key derivation). This did **not** change in the rename:

- Key derived from `/proc/self/cmdline` (the running process's package
  name), never stored as a constant next to the ciphertext.
- FNV-1a over the package name, folded with a per-slot nonce, seeds an
  xorshift32 keystream (byte-for-byte identical to vaultcrypto.c).
- `RegisterNatives` in `JNI_OnLoad`, matching `libcrackingtheshell.so`'s
  posture (not vaultcrypto's — vaultcrypto is deliberately legible because
  reading it *is* target 16's solve; nothing here should be).
- Linked with `-s` like crackingtheshell.

**Why every slot gets its own nonce.** All five real slots share the
`Securinets{` prefix. A single seed derived only from the package name
(public — it's the app ID, visible in the manifest) would give an attacker
known plaintext against a shared keystream: XOR any one ciphertext with
`Securinets{` to recover 11 keystream bytes, then reuse those bytes against
the other four. Distinct nonces per slot make each value its own one-time
pad instance. This mirrors why `SirrCrypto.seal()` takes a per-note nonce —
same failure mode, same fix. Decoy slots get their own nonces too, for the
same reason and so the real slots cannot be singled out by nonce shape.

**Slot numbers and nonces are arbitrary, not derived from the challenge id
or from each other.** The old per-accessor nonces
(`LOOSELIPS_NONCE = 0x4c4c334c`, etc.) each visibly embedded the ASCII code
of their challenge id in a middle byte — `0x33` = '3', `0x34` = '4', and so
on. That pattern is gone: the new slot values (`0x4b`, `0x18`, `0x63`,
`0x2f`, `0x76` for the five real ones) and nonces were chosen without
reference to challenge id, accessor name, or each other.

**Threat model, matched to target 16's, not stronger.** The package name
itself is not secret; anyone who reads the derivation out of the
disassembled `.so` can reimplement it offline in Python, same as
`sirr_unseal`. That is accepted here for the same reason it's accepted
there: the bar is "jadx + one line of Python" must stop working, not "no
one with a disassembler and time can ever recover this." Item #3
(control-flow obfuscation, not yet built) is what raises that bar further —
scoped for these libs, deliberately excluded from vaultcrypto.

## Generator

`secrets/gen_securestore_blobs.py` (gitignored, outside `app/src`, renamed
from `gen_flagvault_blobs.py`) produces every `*_CT` array — the five real
slots from `secrets/flags.json` plus every decoy slot — from the explicit
slot/nonce/plaintext tables at the top of the script. Re-run it and paste
the output into `securestore.c` if a flag or a decoy value ever changes.
`ctf-app/tools/verify_securestore.c` is the host-compiled harness with the
same ciphertexts and derivation logic, used to confirm every slot still
round-trips to the expected plaintext after any change.

## What did NOT change

- The crypto: `derive_seed`, `next_key_byte`, and `unseal` in `securestore.c`
  are byte-for-byte the same algorithm as `flagvault.c` had. Only the
  dispatch (one `read(slot)` instead of five named externs) and the slot
  table (more entries, decoys mixed in) changed.
- `ChallengeRegistry.flagSha256` for ids 3, 4, 5, 6 — scoring stays local,
  SHA-256 of the returned string, same as before.
- Challenge 8 (Face Value) has no `flagSha256` and auto-captures via
  `onCapture(flag)` directly; unaffected by this change beyond the accessor
  call itself.

---

# vaultcrypto.c — target 16 (Sirr)

Moved here from the source's own comments when comments were stripped
(session 26). The code is unchanged; only the prose moved.

## This library is the OPPOSITE of libcrackingtheshell.so, deliberately

The two must never be merged. `libcrackingtheshell.so` is stripped,
dynamically registered and padded with decoys, because **finding** it is
target 15's challenge. This one is meant to be found in the first thirty
seconds:

```
$ nm -D libvaultcrypto.so | grep -i seal
00000000000063a0 T sirr_unseal
```

The lesson is not "find the hidden thing", it is **"call the thing you
found"**. `sirr_unseal` has no JNI binding at all — it is not on
`SirrCrypto`, not on any class, and `Java.use()` cannot reach it. The only
way in is `Module.getExportByName` + `NativeFunction`, which is exactly the
skill this target exists to teach and which targets 08 and 15 (both plain
`Java.use` return-value hooks) do not.

**Do not "tidy" this by adding a JNI binding for `sirr_unseal`.** That
single change deletes the entire challenge.

## The key is never stored

It is derived at call time from the running process's own package name, read
from `/proc/self/cmdline` — argv of the live process. That value is not
present anywhere in the file on disk, so lifting `SEALED_NOTE` out of the
`.so` and XOR-ing it in Python gets you nothing until you have also read the
derivation.

A determined analyst will read it. That is fine and expected. **The bar is
that calling the function is EASIER than reimplementing it**, not that
reimplementing it is impossible.

## The nonce is load-bearing, not decoration

`derive_key` runs FNV-1a over the package name, then over the nonce.
`nonce == 0` reproduces the plain package-name digest exactly — that is the
legacy format the archived note was sealed in, and the one
`secrets/gen_sirr_blob.py` mirrors.

Every note sealed by *this* build passes a fresh nonce. With a single fixed
keystream, an attacker seals a known plaintext, XORs it against their own
text to recover the keystream, and XORs that against the archived blob to
get the flag — **without ever attaching a debugger.** A two-time pad, and a
complete bypass of the only skill this target teaches. The nonce is what
stops that.

The same reasoning is why `flagvault.c` uses a distinct nonce per flag: all
five share the `Securinets{` prefix.

## Other invariants

- `xorshift32`, low byte per round, is mirrored in `secrets/gen_sirr_blob.py`.
  Change one and you must change both.
- No package name means no key: emit nothing rather than garbage.
- This library **stays legible**. Hardening item #3 (control-flow
  obfuscation) applies to `flagvault.c` and `crackingtheshell.c`, never here
  — reading this is part of a challenge.

---

# decoys.c is generated — never hand-edit it

`decoys.c` is produced by `generate_decoys.py` (seed 1337, 400 functions,
deterministic) and is gitignored. It used to carry a
`/* Generated decoys — do not edit manually */` banner; that banner was a
comment in challenge source, so it was removed from the emitter in session 26
and the note lives here instead. Regenerate with:

```
python3 ctf-app/app/src/main/cpp/generate_decoys.py ctf-app/app/src/main/cpp/decoys.c
```

The generator itself keeps its docstring — it is build tooling, not something
the player ever sees, and `CLAUDE.md` exempts generators.

## Target 15 — the licence nonce must never be the thing the player patches

The first build of this challenge had one native function doing two jobs:

```c
static uint32_t entitlement(void) { volatile uint32_t slot = 0u; return slot; }

native_is_licensed()  -> entitlement() == 1u
native_compute_flag() -> sx_entry(body, LICENCE_BODY, len, entitlement(), 0)
```

`tools/gen_blob.py` seals `LICENCE_BODY` with `"nonce": 0x00000001`, so the blob only decrypts
when `entitlement()` returns 1. That looked elegant — the gate *is* the key, so flipping the gate
is the solve — and it was wrong for one reason: `isLicensed()` was a **native** method, so there
was no boolean in the DEX to patch. A player working in smali could only patch the call site in
`LicenseActivity`, which flips the UI while the native `entitlement()` still returns 0. The flag
then decrypts under the wrong keystream, `legible()` maps every non-printable byte into ASCII, and
the app hands the player a printable 35-character string that looks exactly like a flag and is not
one. `capture()` is local, so the app then stored it and said "Flag Captured".

The primer for target 15 told players to patch the smali boolean, which is the one thing that could
not work. That combination produced a support ticket during the event.

So the two jobs are split:

- **The gate lives in Kotlin.** `LicenseCheck.isLicensed(context)` compares a stored receipt to an
  issued one and returns the result. In smali that is `Intrinsics.areEqual` → `move-result v1` →
  `return v1`, which is the canonical patch target the primer describes.
- **The nonce is a sealed constant in native code** (`LICENCE_STEP`), so `nativeComputeFlag()`
  always decrypts correctly. Whatever the player patches above it, the flag is either shown in full
  or not computed at all.

**Do not pass the nonce in from Kotlin** to make it feel less hardcoded. A Java-supplied nonce is a
32-bit value the player controls, and `Securinets{` is a perfect oracle on a 35-byte blob — the
challenge becomes an offline brute force with no patching at all.

What still binds the blob is the process identity: the cascade folds `/proc/self/cmdline` into the
keystream, so the library only yields the flag inside a process named `tn.securinets.ctf`. That is
why a host harness gets garbage at the correct nonce until it is run with that argv[0], and it is
the reason `applicationId` must not pick up a debug suffix — a `.debug` applicationId would change
the identity and break the flag in debug builds only.

`LicenseActivity` now checks the computed string for the `Securinets{…}` envelope before calling
`onCapture`. That is a shape check, not a content check, so it does not reintroduce the client-side
flag oracle RT-02a removed; it exists so a broken or swapped native library reports itself instead
of presenting garbage as a captured flag.

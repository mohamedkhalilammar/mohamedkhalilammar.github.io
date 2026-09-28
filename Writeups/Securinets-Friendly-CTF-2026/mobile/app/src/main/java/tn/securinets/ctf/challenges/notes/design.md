# Challenge screens — design notes

Rationale for decisions in `challenges/`. The source itself carries no comments
(see `CLAUDE.md`); this is where the reasoning lives.

## #5 Front Door Trick (`frontdoor/`)

### The admin password is generated, never stored

`LoginGate.onCreate` seeds `admin` with 24 bytes of `SecureRandom` rendered as hex,
bound as a **parameter** so it never enters a query string that could be logged.

Why: a literal (`K7#mP2vQ9xL4` until 2026-09-12) is the easiest path in the whole
challenge. A player already in jadx for #1 and #2 reads it, logs in normally, and never
writes an injection — a complete bypass of the M4 lesson this challenge exists to teach.

Consequences, all accepted:

- The password differs per install, so there is no fixed value to leak or to pass between
  students during the event.
- A rooted player can still dump `auth.db` and read the generated value. That is *harder*
  than the intended SQLi and is a real skill, so it is not a shortcut. Known, not a defect.
- `guest`/`guest1234` **stays a literal on purpose.** It is a realistic demo account and it
  is the scaffolding: the player signs in as guest, sees the form work, and learns the goal
  is `admin` specifically.
- The vulnerable `attempt()` query **stays string-concatenated.** Only the seeding insert is
  parameterised. Parameterising the login query would delete the challenge.

Designer mode seeds `DesignerMode.key` as the admin password instead of a random value.
Build flag only (`-PctfDesigner=<value>`), unreachable from input, intent or network.

The DB version moved 1 → 2 with a drop-and-reseed `onUpgrade`, so an install carrying the
old literal-seeded `auth.db` re-seeds instead of silently keeping the old password.

### Guest gets a real signed-in screen

A successful guest login used to set a one-line banner on the same form. It now transitions
to signed-in product chrome: Guest profile header, an empty accounts list, a locked
Administration panel, and Sign out back to the form.

Why: it rewards the guest login and makes the goal concrete — get in as `admin`, not "get in
somehow".

Two hard constraints on that panel:

- **No hint at the technique.** "Administrator access required" is *situation*. Nothing about
  queries, quotes, `OR 1=1`, or injection appears anywhere on the screen.
- **No flag in any form** — not in an invisible composable, not in a disabled field, not in a
  `contentDescription`. Otherwise a `uiautomator` view-hierarchy dump skips the injection
  entirely.

## #8 Face Value (`facevalue/`)

The lesson is a client-side biometric gate with no `CryptoObject`, bypassed by a Frida hook.
**Do not add a `CryptoObject` — its absence is the vulnerability.**

The `else` branch used to call `openVault()` directly whenever `canAuthenticate` was not
`BIOMETRIC_SUCCESS`. That is the case on any stock emulator and any phone with fingerprint
unlock switched off, so the vault opened on one tap with no tooling — and a player could opt
in by removing their fingerprint in Settings. A banner announced it.

That is the designer-mode constraint implemented backwards: designer mode must be forced by
the **builder**, never triggered by device state the **player** controls. The branch order is
now `DesignerMode.enabled` → `hasHardware` → block with a message naming the device
requirement and nothing else. The banner is gone.

`onAuthenticationFailed` and `onAuthenticationError` were empty bodies, so cancelling the
prompt gave no feedback and read as a broken control. Both now surface a short neutral
message through `onMessage`.

## #16 No Caller (`sirr/`)

### Capture

#16 needs no bespoke flag box. `ChallengeScreen` renders the shared `FlagSubmit` for every
challenge with a `flagSha256`, and `ProgressStore.submitFlag` hashes the entry and calls
`markCaptured`. #16 has a hash in the registry, so the manual path already registers the
solve. A second entry box in the activity would be a duplicate mechanism.

### Designer mode

The native side already carried `sirr_designer_open()` behind `#ifdef SX_DESIGNER`, but
nothing defined `SX_DESIGNER` and there was no JNI binding, so it was unreachable. Now:

- `build.gradle.kts` passes `-DSX_DESIGNER=ON` to CMake **only** when `-PctfDesigner=<value>`
  is set, so the app-side `DesignerMode` boolean and the native define come from one switch
  and cannot drift apart.
- `CMakeLists.txt` applies the define to `vaultcrypto` and `crackingtheshell`. In the maze it
  also enables `SX_DESIGNER_ID`, which forces the process identity the cascade binds to — the
  "wrong process identity → garbage" property becomes testable without a device.
- `vaultcrypto.c` gains a JNI wrapper inside the existing `#ifdef`.

Verified: a `-PctfDesigner=` build exports `sirr_designer_open` and
`..._SirrCrypto_designerOpen`; the shipping build exports exactly three symbols
(`sirr_unseal`, `..._seal`, `..._archivedBlobHex`) and neither designer symbol. AGP keys its
CMake output directory on the argument set, so the two variants do not contaminate each
other — but note the packaged APK is what must be checked, not
`intermediates/cxx/Debug/*/`, which holds both.

`SirrCrypto.designerOpen()` is an `external fun` with no implementation in a shipping build.
Calling it there throws `UnsatisfiedLinkError`, so the call site is guarded by
`DesignerMode.enabled`. The two are driven by the same gradle property, which is what keeps
that guard honest.

## Network cluster — #9, #11, #14 (2026-09-12)

### Capture on #9, #10, #11 (and #16) was already wired

The audit reads `onCapture` being unused on these screens as "the challenge never registers a
solve". It does. `ChallengeScreen` renders the shared `FlagSubmit` for every challenge that has a
`flagSha256`, and `ProgressStore.submitFlag` hashes the entry and calls `markCaptured`. All 15
registry entries have a hash, so every one of these screens already registers the solve through the
manual path. Nothing was added — a per-screen box would have been a second mechanism doing the
same job.

Worth knowing if this is ever changed: `BaseChallengeActivity.capture()` marks progress *and*
celebrates, but `onFlagAccepted` only celebrates — because `submitFlag` has already marked it.

### #9's nudge

`"Signed in."` → `"Signed in. Did you see it?"`, chosen on `response.isSuccessful` alone.

The body is never parsed to decide it. If `session_note` ever landed in a Kotlin variable the value
would be in the dex and a `strings` hit, which is the exact leak this challenge exists to avoid.
A longer draft ("the server sent back more than this screen is showing you") was rejected earlier as
too big a hint: it names where to look. The bare question only resolves for someone already watching
the traffic. Failure stays `"Invalid credentials."` with no nudge, so the nudge fires only after a
request worth capturing was actually made.

### The hand-rolled hostname verifier is gone

Both clients set `hostnameVerifier { hostname == HOST }`. The certificate's SAN already covers the
hostname, so it was redundant — and a hand-rolled verifier is the thing that silently becomes a real
bypass the day someone edits it to `true`. `HOST` and `CERT_PIN` are untouched;
`tools/set_backend_host.py --check` still reports all three in agreement.

### #14 — the flag left the APK

`FLAG_BYTES`, `MASK_KEY`, `unmask()` and the `diagnostic_trace` payload field are **deleted**. A
5-byte repeating XOR in Kotlin source against the known prefix `Securinets{` falls to automated
analysis without anyone reading the code; it was the worst leak in the project.

The collector now returns the flag as `sync_token` on a well-formed POST (`device_id`, `install_id`,
`event`). **The app must never read that field** — the player takes it off the wire. The
designer-mode surface deliberately records only `response.code`, never the body, for the same
reason.

Two other defects in the same file:

- `GlobalScope.launch` replaced with a `CoroutineScope(SupervisorJob() + Dispatchers.IO)` held by the
  object.
- Both catch blocks were empty, so a collector that is down made the challenge a silent no-op that
  read as broken. Still silent for the player (realistic for an SDK), but `MetricFlowKit.lastDelivery`
  now records the outcome when `DesignerMode.enabled`.

**Firing app-wide** is the point of a supply-chain lesson: the library runs with the app's
permissions everywhere, not only on the screen that told you to look. `track()` now fires from
`CtfApplication.onCreate` (`app_session_start`) and from `BaseChallengeActivity.onCreate`
(`screen_view`), so the rogue POST is one stream among many that the player stumbles onto while
working other challenges. That also serves non-negotiable #4. `CtfApplication` is registered as
`android:name` on `<application>`; it did not exist before.

### Still not built — native request signing

`SecureStore.sign()` does **not** exist. Until it does, #9 and #10 still fall to plain `curl` and
interception is not actually mandatory for either. This is the one item of the network work order
that is untouched; the contract it must satisfy is fixed and reproduced in
`backend/testing/sign_request.py`.

## Native request signing — `SecureStore.sign()` (2026-09-12)

Built and verified end to end. Interception is now **mandatory** for #9 and #10: without a valid
`X-Sig` header every one of those endpoints returns 401, so plain `curl` no longer reaches them.

### Where the key comes from

`securestore.c` already contained a full SHA-256 (`digest_bytes`) and HMAC (`keyed_digest`) — the
brief's suggestion to import `game-track/native/src/sha256.c` was unnecessary and was not done.

`K` is never stored. What ships is `SIG_MATERIAL`, which is `K` folded twice:

1. against the package-derived xorshift keystream the slot ciphertexts already use
   (`derive_seed(package, SIG_NONCE)` → `next_key_byte`), and
2. against `SHA-256(package name)`.

The second fold is the point. Reusing only the slot keystream would mean a script that cracks any
one slot recovers `K` for free — the cost of attacking one would transfer to the other, which is
exactly what RT-01's "per-slot distinct schemes" ruling exists to prevent. `tools/gen_sig_key.py`
generates the array and `--verify` round-trips it back to `K`.

Confirmed against the shipped `libsecurestore.so`: `K` is absent as raw bytes **and** as a hex
string, `strings` returns nothing, only `JNI_OnLoad` is exported, and both natives register.

### The three constructions

| Endpoint | Signed input | Mode |
|---|---|---|
| `POST /login` | raw request body bytes | `SIG_BODY` |
| `GET /profile/<id>` | bearer token — path **excluded** | `SIG_TOKEN` |
| `GET /audit` | bearer token + `"/audit"` — path **included** | `SIG_TOKEN_AUDIT` |
| `/admin/report`, `/activate` | no header at all | — |

The `"/audit"` literal is appended **in C**, not Kotlin, so it is not sitting in the dex.

The asymmetry is load-bearing, not an oversight. `/profile` excludes the path so a captured
signature stays valid when the player rewrites `/profile/6` → `/profile/4` in flight — signing the
path there would make #10 unsolvable by any route. `/audit` includes it so a signature lifted from
the unpinned `/profile` call cannot be replayed against the pinned `/audit`, which is what makes
#11's certificate-pinning bypass mandatory rather than optional.

**No timestamp is in any signature** — a phone with a skewed clock would otherwise lose every
network challenge at once with an error that reads as a server fault. The signature is **never
logged and never shown in the UI**; one `Log.d` would make it readable with `adb logcat` and the
whole proxy requirement would evaporate.

### Verified

C output matched `backend/testing/sign_request.py` byte for byte on all three constructions, then
the C-produced signatures were run against the live backend:

```
/login   signed                                 200
/login   no sig                                 401
/profile/6 own id, signed                       200
/profile/4 rewritten id, same sig               200   <- #10 stays solvable
/profile/4 no sig                               401   <- curl alone is not enough
/audit   with the /profile sig replayed         401   <- #11's pinning stays mandatory
/audit   with the audit sig                     200
```

### A backend blocker found by running it

`/login` minted `"sub": user_id` as an **integer**. PyJWT 2.10+ rejects a non-string `sub` on
decode (RFC 7519 StringOrURI), so the server could not decode the token it had just issued and
**every `/profile` and `/audit` request returned 401** — #10 and #11 were unsolvable for reasons
that had nothing to do with signing, and the 401 gave no hint why.

`backend/requirements.txt` pins `PyJWT==2.8.0`, which tolerates it, so a correctly provisioned
host would have worked and this would have detonated only if anyone installed unpinned or ran
`pip install -U`. Fixed at the mint site (`str(user_id)`), which is correct under both versions.
Nothing server-side reads `sub`, so nothing else moved — #12 forges `role`, not `sub`.

## #17 Second Hand (`chronos/`) — 2026-09-12

Built to `docs/CHALLENGE-17-BUILD.md`. That file is the design; this records the two
things the build had to decide that the spec did not settle, and the traps in the module.

### The spec's "reroll the targets" could not be built as written

`docs/CHALLENGE-17-BUILD.md` and the audit both say round expiry *rerolls the targets*,
and both also say the accumulator folds the target value in (`acc = SHA256(acc ||
HMAC(K, target || i))`) and that `open()` decrypts a **static** `SHIFT_CT`. Those cannot
both hold. Rerolled targets give a different accumulator every round, so the ciphertext
would decrypt to noise even for a player who did everything right, and there is no
server to re-wrap it per round.

Resolved as: **five fixed target times, canonical slot indices 0-4, presentation order
freshly shuffled on every `begin()`.** `fold()` is called with the target's *canonical
slot*, not its position in the current round, so completing all five in any presentation
order lands on the same accumulator.

What that keeps from the original intent:

- A hardcoded straight-line script still fails. The script cannot know which target is
  live without reading it, which is exactly the reactive loop the audit asked for.
- Failure still costs the whole round: expiry calls `Chronos.reset()`, so partial
  progress is worth nothing and manual clock-setting still cannot finish inside the
  global window.

What it gives up: the target values themselves are stable across rounds. That is already
inside the accepted residual — the targets are printed on screen, so a player who can
read them can read them once.

### Why a fourth library

`libchronos.so` carries K and the flag. It is not a tenant of `vaultcrypto` (#16's note
lives there) or `crackingtheshell` (#15's players are told to patch and re-sign that
one). Its maze comes from its own profile seed, `chronos` / `0x7A11C10`, so unpicking
either of the other two transfers nothing.

### SHA-256 and HMAC are duplicated from `securestore.c` on purpose

Not an oversight and not worth factoring into a shared `sha256.c`. `securestore.c`'s
copy is verified byte-for-byte against `backend/testing/sign_request.py` and against a
live server; refactoring it to share a header would put that verification at risk to buy
nothing — the two libraries ship as separate binaries either way, so the object code is
duplicated regardless.

### The two clocks

`ShiftRoster.displayed()` reads `Calendar.getInstance(TimeZone.getTimeZone("Africa/Tunis"))`
and is what the player sees. `ShiftRoster.observed()` reads
`LocalTime.now(ZoneId.of("Africa/Tunis"))` and is the only thing compared against a
target. Both are pinned to the same zone so that an unhooked app in any timezone shows a
display that agrees with the checker — a mismatch there reads as our bug, not as a clue.

`System.currentTimeMillis()` must never become the bait. Both of these bottom out there,
which is precisely why the two *Java* call sites have to be distinct: hooking
`currentTimeMillis` also moves Choreographer, animations and OkHttp timeouts, and the app
falls apart in ways that look like a defect in the challenge.

The countdown is `SystemClock.elapsedRealtime()` and must stay there. It is monotonic, so
spoofing the wall clock buys no extra time, and it is a third API so hooking it is a
separate deliberate act rather than a side effect.

### No oracle, deliberately

`chronos_open()` never inspects the accumulator. Four folds, or five against the wrong
slots, produce a different keystream, a different cascade input, and printable mojibake —
not a refusal. `FinalCountdownActivity` prints whatever comes back and does **not** call
`onCapture`; the player reads it and pastes it into the shared `FlagSubmit` box, which
hashes against the registry. Do not "improve" this by validating the result in the
Activity: a boolean on screen is a boolean one hook away, and it hands back the oracle.

### Designer mode

`DesignerMode.enabled` fixes the presentation order to 0-4, makes both deadlines
unreachable (`expired()` returns false, the remaining-time helpers return the full
window), and adds a **record this check-in** button that folds the current target
directly. Without that last one the module cannot be exercised at all without a device
whose clock you can drive, which is the whole point of the standing constraint in
`CLAUDE.md`.

`SX_DESIGNER` also reaches the C: `read_package_name()` honours `SX_DESIGNER_ID` under it,
mirroring the maze's own identity node. That is what lets `tools/verify_chronos.py`
compile the real `chronos.c` for the host and drive the real accumulator. Verified absent
from the shipping APK — no `getenv`, no `SX_DESIGNER_ID`.

### Drift is the failure mode to fear

`ShiftRoster.TARGETS` and `secrets/gen_chronos_blobs.py`'s `SHIFTS` must stay identical.
If they diverge the challenge silently becomes unsolvable, with no error anywhere.
`tools/verify_chronos.py` reads both and fails on mismatch; run it after touching either.

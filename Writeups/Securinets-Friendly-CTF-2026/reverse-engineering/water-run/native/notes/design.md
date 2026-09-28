# game-track native library — design notes

Rationale lives here, not in the source. `src/*.c` ships to players and must read
like software someone wrote, so it carries no comments.

## Why there is a GDExtension at all

The whole challenge is that a player finds the score with a Cheat Engine 4-byte
scan and edits it. That only works if the score is a plain `int` at a stable
offset inside a heap allocation the process owns.

Godot stores every GDScript value as a `Variant` on an engine-managed heap.
A score kept in GDScript is therefore not a findable 4-byte integer — it is a
tagged union that Godot may move, box, or copy. Scanning for it produces noise
and editing a hit does not reliably change the game.

So `cg_ctx` (calloc'd in `cg_create`) is the single source of truth for score,
bottles, game id and nonce, and `src/gdext.c` is a thin forwarding layer that
lets GDScript reach it. **State must never migrate back into GDScript**, no
matter how much simpler that would make the game code. If a future change needs
a value the game reads every frame, add an accessor here; do not cache it in
GDScript.

## The clamp is the challenge

`cg_add_score` clamps at `CG_SCORE_CLAMP` (99999) and `cg_token` refuses below
`CG_TOKEN_MIN` (100000). The gap is deliberate and exactly one unit wide: no
sequence of legal play can ever mint a token. The only way past it is to write
the score in memory. `tools/gdextcheck/check.gd` asserts this — it banks a
million points, confirms the score sits at the clamp, and confirms `token()` is
still empty. If that assertion ever starts failing, the challenge has been
given away.

## The two variants must not share a struct offset

Water Run and Revenge are the same game compiled twice. Revenge exists to teach a
different scan: the beginner build has a persistent `lift` float you can find with
`Float, exact value 9.25347`, Revenge has no such field, only a live `vertical`
that is 0.0 on the ground, 9.0 for one frame at the jump, and falling every frame
after that. Neither an exact-value scan nor a static read of the value works there.

That difference was undone by the struct layout. Both variants put the interesting
float at `score + 8`, so anyone who had finished the beginner build could find the
score in Revenge — a changing int, the easiest landmark in the process — add eight
bytes, and be on the velocity field without ever running an unknown-initial-value
scan. The whole lesson was reachable by arithmetic carried over from the previous
challenge.

So in `CG_VELOCITY_MODE` the struct puts `game_id` and `phase` where the beginner
build keeps `lift`. `score + 8` now reads a constant `1`, `score + 12` reads `0`
until the gate is armed, and `vertical` sits at `score + 16`. Both dead ends read
as uninteresting integers rather than as plausible-but-wrong floats, which matters:
`peak` must **never** be the field at `score + 8`. Writing `peak` above 12.5
satisfies the `cg_phase_a` check while changing nothing about the jump, so it would
be a decoy that silently swallows a correct-looking edit.

Struct size is unchanged at 64 bytes, and the beginner layout is untouched — a
rebuild of that variant is byte-identical to the shipped DLL apart from the three
mingw timestamp fields.

What this does **not** close is browsing the memory region around the score in
Cheat Engine and noticing which nearby value moves when the player jumps. That is
left open deliberately: observing what changes under a known action is the lesson,
not the shortcut. If it ever needs closing, the lever is a second heap allocation
for the motion state so nothing interesting is adjacent to the score at all — at
the cost of a pointer in the struct, which is its own breadcrumb.

## What the player is told to scan for

**The description no longer publishes a solve at all** — it taunts and states the
premise, and that is deliberate. The intended path is still the one below, and it is
recorded here because nothing player-facing records it any more.

The intended scan is `Increased value` while rising, `Decreased value` while falling,
and a `Value between -1 and 1` pass while standing still. **That works, and it works on
`vertical` — the live velocity — with no change to this library.**

The reason it works is that Cheat Engine's increased/decreased comparison is against
the value it recorded at the **previous scan**, not against a continuously tracked
trend. `vertical` does decrease monotonically across a single jump, which makes it
tempting to conclude that `Increased value` cannot match it. It can, because the
snapshots the player takes are on the ground (`0.0`), then mid-rise (positive), then
mid-fall (negative): `0.0 -> +6` is an increase, `+6 -> -5` is a decrease, and
`-5 -> 0.0` lands inside the standing-still range filter. An earlier pass of this
file argued the opposite and was wrong; the error was reasoning about the value's
shape instead of about what CE actually compares.

`cg_motion` sets `vertical` to exactly `0.0` on every grounded frame, which is what
makes the `-1 to 1` pass land cleanly. Do not change that to a small non-zero resting
value — the range filter in the description depends on it.

**What the filter does not separate is height.** The Godot node's `position.y` rises
and falls with the player, so it survives increased/decreased, and the player rests at
`y = 0.1`, so it survives the range filter too. It is therefore a live wrong answer:
writing it moves the player over the wall and `cg_phase_a` still refuses, because the
gate only inspects `peak`, which only tracks `vertical`. That is the same
silently-swallowed-edit hazard this file warns about two sections up, except it lives
in the player-facing copy rather than in the struct. It is **not** closed in the shipped copy: the
hint that distinguished them (height is never negative, velocity is negative for the
whole descent) was cut when the description was stripped of hints. It belongs in a
CTFd hint or in support answers, not in the code — do not move the struct for it.

An altitude-based redesign was scoped and rejected once the CE semantics were
understood: it would have made the hunted field height, moved the struct, replaced the
`peak >= 12.5` gate with a live altitude check, invalidated both offset verifiers and
the packaged zip, and taught the less useful of the two values. Nothing in this
library or in the GDScript changed.

**The speedhack is required in practice and is no longer mentioned to the player.** Airtime is `2 * 9.0 / 24.0` = 0.75 s, split into a 0.375 s rise and a
0.375 s fall, and the player has to click the right button inside each half. Godot's
delta comes from the timing APIs the speedhack hooks, so the physics stretches in real
time and a jump becomes roughly fifteen seconds. This is reasoned from the code, **not
observed** — the speedhack path has never been run against the shipped build on real
Windows.

Two verification pokes hardcode these offsets and must move with the struct:
`tests/test_challenge.c` (compiled per variant, so it uses `#ifdef`) and
`tools/prove_flag.sh` (one binary driving both `.so` files, so the offset is an
argument). A wrong offset there does not fail loudly — `cg_phase_a` just refuses
and the harness reports a build that looks broken.

## Choices in the binding

**Plain C against `gdextension_interface.h`, not godot-cpp.** The API surface is
eleven methods on one class. godot-cpp is a large build dependency that would
dominate the build time and the artifact for no benefit here.

**`classdb_register_extension_class6` / `classdb_construct_object3`.** These are
the 4.7-native entry points, which is why `cgchallenge.gdextension` declares
`compatibility_minimum = "4.7"`. Dropping to an older revision of the API to widen
that floor is possible but has not been needed.

**Both `call_func` and `ptrcall_func` are implemented for every method.** Godot
calls `ptrcall` whenever GDScript knows the static types and crashes outright if
`ptrcall_func` is null, so neither path is optional.

**`token()` returns an empty String on failure rather than an error code.** The
game never needs to distinguish "no nonce" from "score too low" — both mean
"there is nothing to show the player yet".

**The `cg_*` functions stay exported from the extension library.** `CG_API` in
`challenge.h` forces default visibility, and that header is fixed. This does not
open a bypass: an attacker who calls `cg_token` directly still has to get the
score past the clamp, which is the same memory edit the intended solve requires.

## What build.sh proves, and why each stage exists

- **cross-check** — the compiled binary and the Python service compute the same
  token. A Python reimplementation agreeing with itself proves nothing.
- **entry symbol vs manifest** — the classic silent GDExtension failure is an
  `entry_symbol` that does not match the library: everything loads, and the class
  simply never appears in ClassDB.
- **the Godot load check** — runs a throwaway project, not `game1`, so the stage
  fails only when the extension is broken and never because someone is mid-edit
  on the game.
- **the leak check inside it** — Godot's own allocator leak reporting is compiled
  out of the binaries people actually have, so it cannot be relied on. Measuring
  resident growth across 400k create/destroy cycles catches both a missing
  `cg_destroy` and a missing `mem_free`; each shows up as ~12 MB against a
  baseline that is flat at 0 KB.

Note that the DLL checks read the PE export directory via `objdump -p`, not `nm`.
The DLL is linked with `-s`, so `nm -g` on it reports no symbols at all and every
`nm`-based grep over it would pass vacuously.

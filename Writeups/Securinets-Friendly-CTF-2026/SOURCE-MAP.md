# Source Map

What lives where in this bundle, and which writeup each piece belongs to. Flags
are public (the event is over); no TLS private keys or live API keys are shipped
here. Challenge defaults such as the backend signing key remain because they are
part of the exercises. See `HOSTING.md` to run anything.

## `reverse-engineering/`

Standalone challenges. Each is a self-contained binary or script build — nothing
to host. Read the source, run `build.sh` if you want the artifact, and check
`solution/` for the intended solve. The flag itself is never a stored literal in
these; it is derived at build time from inputs that are **not** in this bundle
(the `*.txt` seed/flag files were left out on purpose), so `strings` on a rebuilt
artifact returns nothing.

| Directory | Challenge | Language / focus |
|---|---|---|
| `01-doorman` | Doorman | C — hidden control flow / patched entry |
| `02-night-shift` | Shift | C — schedule-gated check |
| `03-broken-lock` | Patch&Go | C — single-byte patch |
| `04-paper-trail` | Paper Trail | Python — encrypted payload recovery |
| `05-dead-air` | Relay | C — protocol/relay token |
| `07-debug-me-anyway` | AntiDbg | C — ptrace / anti-debug, multi-stage |
| `straight-key` | Straight Key | Go + C — Morse-over-keyboard exfil sample (malware track) |
| `water-run` | Water Run | Godot game + native C GDExtension — memory hacking |
| `tools/` | shared build tooling | `seal.py`, `codekey.py`, `rota.py`, `gen_notice.py` — take the secret as an argument, none embedded |
| `README.md` | re-track overview | — |

Per-challenge layout: `src/` (source, minus secret `.txt` inputs), `build.sh`,
`notes/design.md` (why the challenge is built the way it is), `solution/`
(reference solve). `water-run/` splits into `native/` (the `cgchallenge.dll`
source, `build.sh`, tests, tools) and `game1/` (Godot project source — scripts,
scenes, `.tscn`, project files; binary art, audio, `.godot/` caches, and built
exports were left out). `water-run/build_game1_variants.sh` builds the beginner
and advanced variants.

## `mobile/`

The Android app plus the services three of its challenge classes talk to.

| Directory | What it is | Belongs to |
|---|---|---|
| `app/src/main/java/tn/securinets/ctf/` | Kotlin app source — one package per challenge under `challenges/`, plus `net/`, `ui/`, `challenge/` (registry, primers) | every mobile writeup |
| `app/src/main/cpp/` | native (JNI) source: crypto, chronos, securestore, decoys | Warmup, Final Countdown, and hardening |
| `app/src/main/AndroidManifest.xml` | components, exported activities, cleartext + backup flags | Off the Map (exported activity) and others |
| `backend/` | Flask backend (`app.py`), rogue SDK collector, Docker setup | First Contact, Echoes, What Remains, L0gIn, Off the Map, Face to Face, Open Lines, DOR, PINNED, Forged Papers, Strangers, Final Countdown |
| `firebase/` | Realtime Database emulator config + seed generator | WideOpen |

The app source is read-only reference: there is no Gradle project, keystore, or
built APK here. To point a rebuilt APK at your own backend you edit
`app/src/main/java/tn/securinets/ctf/net/NetworkConfig.kt` — see `HOSTING.md`.

One value was redacted from the source: the Firebase Web API key in
`challenges/wideopen/WideOpenConfig.kt` (it identified the author's live Firebase
project). Replace it with your own — `HOSTING.md` §9 explains WideOpen setup.

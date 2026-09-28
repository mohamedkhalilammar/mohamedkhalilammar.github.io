---
id: "securinets-friendly-ctf-2026"
title: "Securinets Friendly CTF 2026: the challenges I built"
category: "CTF Writeups"
date: "2026-09-28"
summary: "Twenty-six challenges I shipped for Securinets INSAT's beginner CTF: one Android app holding the whole Mobile track, and a reverse engineering ladder from ltrace to Cheat Engine."
tags:
  - "Challenge Design"
  - "Mobile Security"
  - "Reverse Engineering"
---

I shipped twenty-six challenges for Securinets INSAT's Friendly CTF 2026: the entire Mobile
category and most of Reverse Engineering. This page is the index. Every challenge has its own
writeup, and the source for all of them is published alongside.

The audience was students, most of them doing their first or second CTF. That shaped
everything. A challenge that only rewards someone who already owns an IDA licence teaches
nobody anything, so each one is built around a single technique, and no two in a track are
solved the same way.

## Mobile — one APK, seventeen challenges

The whole Mobile category is a single Android app, `fadigattack.apk`. You install it once and
work through a menu. Behind it sits a small Flask backend: one auth endpoint and one profile
endpoint carry most of the challenges between them.

Challenges are grouped by what they actually ask you to do, not by difficulty number, so you can tell before opening one whether it wants a decompiler, a proxy or a hooking framework.

The track walks the OWASP Mobile Top 10 in roughly increasing difficulty, starting from "an
APK is a ZIP file" and ending at challenges the server has to verify because the client cannot
be trusted to.

A six-step setup walkthrough ships inside the app, because the most common way to lose a beginner is to make them fight their toolchain before they ever see a challenge.

Open any screenshot to see the full-size capture.

| Landing screen | Challenge menu |
|---|---|
| [![The app's landing screen](/media/friendly-ctf-2026/splash.webp)](/media/friendly-ctf-2026/splash.webp) | [![The challenge menu](/media/friendly-ctf-2026/menu.webp)](/media/friendly-ctf-2026/menu.webp) |
| Category grouping | Setup walkthrough |
| [![The menu's category grouping](/media/friendly-ctf-2026/menu-categories.webp)](/media/friendly-ctf-2026/menu-categories.webp) | [![The in-app setup walkthrough](/media/friendly-ctf-2026/guide.webp)](/media/friendly-ctf-2026/guide.webp) |

- **[Plain Sight](/writeups/friendly-ctf-plain-sight)** — The flag is a plain string resource inside the APK; decoding the app's resources reads it straight out.
- **[First Contact](/writeups/friendly-ctf-first-contact)** — A hardcoded account name and an MD5 of its password ship in the APK; crack the hash, then let the server verify the plaintext.
- **[Warmup](/writeups/friendly-ctf-warmup)** — The app encrypts a value with AES but ships the key and IV right beside the ciphertext, so you decrypt it yourself.
- **[Echoes](/writeups/friendly-ctf-echoes)** — The app writes a sensitive value to logcat during normal use; capture it with adb while driving the screen.
- **[What Remains](/writeups/friendly-ctf-what-remains)** — A value the app saved to its own private SQLite database stays on disk after the screen stops showing it.
- **[L0gIn](/writeups/friendly-ctf-l0gin)** — The login form builds a SQL query by pasting your input into a string, so a single quote breaks out and you log in as admin without the password.
- **[Off the Map](/writeups/friendly-ctf-off-the-map)** — A diagnostics screen was dropped from the app's navigation but is still exported in the manifest, so it can be launched directly with adb.
- **[Face to Face](/writeups/friendly-ctf-face-to-face)** — The biometric prompt's success callback is trusted client-side with no CryptoObject, so hooking it opens the vault with no fingerprint.
- **[Open Lines](/writeups/friendly-ctf-open-lines)** — The login talks over plain HTTP and the response carries an extra field the app never shows.
- **[DOR](/writeups/friendly-ctf-dor)** — The profile endpoint takes any valid token and hands back whichever account id you ask for.
- **[PINNED](/writeups/friendly-ctf-pinned)** — The app pins its server certificate so an ordinary proxy fails; you disable the pinning on the device with Frida.
- **[Forged Papers](/writeups/friendly-ctf-forged-papers)** — The JWT is signed correctly with HS256 but the secret is a rockyou.txt word, so you crack it offline and mint your own admin token.
- **[WideOpen](/writeups/friendly-ctf-wideopen)** — The Firebase database has its read rule left open, so the whole customer table dumps over REST.
- **[Strangers](/writeups/friendly-ctf-strangers)** — A bundled analytics SDK the app didn't write ships telemetry to its own server on a different host and port.
- **[License](/writeups/friendly-ctf-license)** — The paid-or-not decision runs entirely on the device, so you patch the smali that makes it.
- **[Nobody Called](/writeups/friendly-ctf-nobody-called)** — A native library still exports the unseal routine for a note format the UI dropped, and nothing in the app calls it.
- **[Final Countdown](/writeups/friendly-ctf-final-countdown)** — Five times to hit in a row, each counted only while the app genuinely reads that time, verified by the server.

## Reverse engineering

The RE category had no on-ramp, so I built one: five Linux ELF challenges that each teach a
different move. `strings` is useless on all of them by design, because the flag is never a
stored string anywhere. It is sealed and rebuilt in memory at run time, which means the only
way through is the technique the challenge is actually about.

- **[Doorman](/writeups/friendly-ctf-doorman)** — The badge code is scrambled so strings finds nothing, but the program checks your input with strcmp, and ltrace prints both sides of that comparison.
- **[Shift](/writeups/friendly-ctf-shift)** — A hand-written XOR-and-rotate loop compared against a table in .rodata. No library call to trace, so you read the loop and run it backwards.
- **[Patch&Go](/writeups/friendly-ctf-patch-and-go)** — A licence check you are not meant to pass. Instead of satisfying it, patch the function to return 1.
- **[Paper Trail](/writeups/friendly-ctf-paper-trail)** — A 10 MB binary that is really a PyInstaller bundle. Recognising the container is most of the work.
- **[Relay](/writeups/friendly-ctf-relay)** — The binary checks in with a server that no longer exists. Read the hostname out of its DNS query, point it at yourself, and answer correctly.

Then the harder end, which is Windows-heavy:

- **[AntiDbg](/writeups/friendly-ctf-antidbg)** — An anti-debug check refuses to run under a debugger, so you let it run, let it get its answer, then change that answer in a register before the program reads it.
- **[Straight Key](/writeups/friendly-ctf-straight-key)** — A Windows utility types one key over and over. The message is not in the letters but in how long each key is held.
- **[Water Run](/writeups/friendly-ctf-water-run)** — A Godot runner with a wall you cannot jump. The jump-power value is a plain float in memory, so a 4-byte Cheat Engine scan finds it.
- **[Water Run: Revenge](/writeups/friendly-ctf-water-run-revenge)** — The same wall, but the convenient value is gone. Only live vertical velocity is in memory, so it takes an unknown-initial-value scan.

## Running them yourself

The source for everything is in `Writeups/Securinets-Friendly-CTF-2026/` in this site's repo,
organised by track, with each RE challenge's build script, design notes and reference solver
next to its source.

Most of the RE challenges are standalone binaries with nothing to set up. The Mobile track
needs the backend, and `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md` walks through
standing it up: creating your own flags file, the container permission trap that kills the
services silently, TLS for the pinned challenge, and pointing the app at your own host.

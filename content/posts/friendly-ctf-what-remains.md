---
id: "friendly-ctf-what-remains"
title: "What Remains"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A value the app saved to its own private SQLite database stays on disk after the screen stops showing it; read it off the data directory."
flag: "Securinets{pl41nt3xt_3v1d3nc3_7de67f16}"
tags:
  - "Mobile Security"
  - "Forensics"
---

This screen has one button: save data to the device. Press it and it writes, tells you it is done, and shows you nothing else. That is the trick. The app stored something and moved on, but "the UI stopped showing it" is not the same as "it is gone." It is still sitting in the app's own storage, and the device owner can read that.

## What you're looking at

You drive the app on a device or emulator, then read files off it with `adb`. The value that gets saved is fetched from the backend, so the server needs to be up. Setup and backend details are in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`.

The class is `tn.securinets.ctf.challenges.whatremains.WhatRemainsActivity`, and the storage lives in `LocalStore`.

## Finding the way in

`LocalStore` is a normal `SQLiteOpenHelper`. When you press save, it fills two tables, `items` and `settings`, in a database called `app_data.db`. The `settings` rows are mostly filler, but one of them is the real thing:

```kotlin
val SETTINGS = listOf(
    "storage_budget_gb" to 0x07,
    "upload_on_wifi"    to 0x1d,
    "thumbnail_cache_mb" to 0x39,
    "sync_account"      to 0x52,
    "api_base"          to 0x6a,
    "session_token"     to 0x18,   // fetched from /backup/session_token
)
```

The `session_token` value is pulled from the server and written into the database as plain text. The screen never displays it. But `SQLiteOpenHelper` writes to a fixed place: `/data/data/tn.securinets.ctf/databases/app_data.db`. So once you have pressed save, the flag is on disk, and all you need is to get at that file.

The activity itself hands you the paths to check:

```bash
ls -la /data/data/tn.securinets.ctf/databases/
ls -la /data/data/tn.securinets.ctf/shared_prefs/
```

## The solve

That data directory is private — to *other apps*. As the person holding the device, you get in through `adb`. On a rootable emulator image this is one command:

```bash
adb root
adb shell
ls -la /data/data/tn.securinets.ctf/databases/
```

Then read the database. The cleanest way is to pull it and open it with `sqlite3` on your own machine:

```bash
adb pull /data/data/tn.securinets.ctf/databases/app_data.db .
sqlite3 app_data.db "SELECT key, value FROM settings;"
```

```
storage_budget_gb|128
upload_on_wifi|true
thumbnail_cache_mb|512
sync_account|owner@fadigattack.local
api_base|http://backup.internal/v1
session_token|Securinets{pl41nt3xt_3v1d3nc3_7de67f16}
```

There it is, in the `session_token` row.

A note on the non-rooted route. This app sets `android:allowBackup="false"`, so the old `adb backup` trick will not pull the data directory. `adb shell run-as tn.securinets.ctf` reads the private files without root, but only when the build is debuggable. For a release build like this, the practical path is a rootable emulator (a plain, non-Google-Play system image lets `adb root` succeed). If `adb root` is refused, your image ships Google Play — make a new emulator with a plain image and try again.

Source is in `tn.securinets.ctf.challenges.whatremains` inside the APK; the packaged challenge is `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

"Private storage" on Android means the app's data directory is isolated from other apps by the Linux user sandbox. It does not mean it is hidden from the person who owns the phone. Root, or a debuggable build, or a rooted emulator, all give you a shell that can read those files directly, and a plain SQLite database is about as readable as data gets.

This is the same insecure-data-storage theme as the logging challenge, from the other side: instead of a value leaking through the log, it persists on disk long after it was needed. The takeaway is that anything the app writes to its own storage in the clear survives, and survives the UI forgetting about it. Sensitive values that must be stored should be encrypted with a key the app does not itself hold in the clear, or better, not stored on the device at all.

## Notes from building it

I filled the `settings` table with believable neighbours — a storage budget, a wifi toggle, a cache size — so the real row does not stand out, and I stored the token in the settings table rather than shared preferences because a database reads more like something an app would keep around than a stray XML file. The session token comes from the server, not from the APK, on purpose: the flag should only ever be on the device because *you* pressed save and made it land there, not because it was baked in.

## Beyond the challenge

Imagine a support app that removes a conversation from the screen on logout but keeps its messages in a local database. Someone examining the app's data could still find that conversation. This is why a forensic review follows data through storage and deletion, rather than treating an empty screen as evidence that the data is gone.

Android's app sandbox still matters: the challenge's lab access is not something an arbitrary unprivileged app automatically has. The lesson is to store only what is needed, define retention and logout behavior, and test those behaviors against the files actually left behind.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Android Debug Bridge guide](https://developer.android.com/tools/adb): practice inspecting files through an emulator shell.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with local storage and sensitive data left on disk.

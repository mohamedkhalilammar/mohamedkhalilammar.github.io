---
id: "friendly-ctf-echoes"
title: "Echoes"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The app writes a sensitive value to logcat during normal use; capture it with adb while driving the screen."
flag: "Securinets{th3_app_t4lks_t00_much_9cd6e756}"
tags:
  - "Mobile Security"
  - "Forensics"
---

This screen has one button that says it writes to the system log. That is not a distraction, it is the whole challenge. Apps talk to the log constantly while they run — debug lines, status messages, the sort of thing developers add and forget to remove. This one logs something it should not, and if you are watching the log when it does, you read it straight off.

## What you're looking at

This one is done on a running app, so you need a device or an emulator, `adb` connected to it, and the backend reachable, because the value that gets logged is fetched from the server first. Setup and backend details are in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`.

The class behind the screen is `tn.securinets.ctf.challenges.echoes.EchoesActivity`, and the thing doing the logging is `BackupManager`.

## Finding the way in

You could stare at the decompiled code first, and it is worth a look because it shows you exactly what to expect. `BackupManager.run()` does this:

```kotlin
suspend fun run() {
    PRE_EMISSIONS.forEach  { slot -> Log.d(TAG, SecureStore.read(slot)) }
    Log.d(TAG, fetchRealNote())        // the real one, fetched from /backup/note
    POST_EMISSIONS.forEach { slot -> Log.d(TAG, SecureStore.read(slot)) }
}
```

`TAG` is `"BackupManager"`. Notice the shape: a handful of decoy lines, then the real note, then a few more decoys. The decoys are genuine-looking log entries so a single glance at the log does not immediately land on the flag. You have to actually read it, which is realistic — production logs are noisy.

The mistake here is to press the button first and then start logcat. The log gets written the instant you press, and if you were not already capturing, you have to press it again. The screen even tells you: start logcat first.

## The solve

Clear the log, then start streaming it, before you touch the app:

```bash
adb logcat -c
adb logcat
```

Now press **RUN IT** on the screen. A burst of lines from the `BackupManager` tag appears. To cut the noise, filter by that tag, or just grep for the flag prefix:

```bash
adb logcat -s BackupManager
```

```
D BackupManager: backup slot 03 restored
D BackupManager: backup slot 0f restored
D BackupManager: Securinets{th3_app_t4lks_t00_much_9cd6e756}
D BackupManager: backup slot 58 restored
...
```

If you did not know the tag, the blunt instrument works just as well:

```bash
adb logcat | grep Securinets
```

Either way the flag is `Securinets{th3_app_t4lks_t00_much_9cd6e756}`.

If nothing shows up: check `adb devices` first to be sure the device is actually attached, and make sure logcat was already running when you pressed the button.

Source is in `tn.securinets.ctf.challenges.echoes` inside the APK; the packaged challenge is `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

`Log.d` looks harmless because it is meant for development, but it does not disappear in a release build unless someone strips it out on purpose. Anyone with `adb` access to the device — the owner, an attacker with physical access, sometimes another app on older Android versions — can read everything the app logs. So a debug line that prints a token, a session value, or a note is a live data channel that ships to production.

That is the insecure-data-storage angle in the OWASP Mobile Top 10, in its plainest form. The fix is not clever: do not log sensitive values, and gate debug logging behind a build flag that is off for release. The habit to build is to run `adb logcat` while you use any app you are testing and see what it says about itself. Apps talk too much, and the log is listening.

## Notes from building it

The decoy lines around the real one matter more than they look. An earlier version logged only the flag, and it was too clean — a beginner would `grep Securinets` and never learn to read a noisy log, which is the actual skill. Wrapping it in plausible neighbours forces you to actually look. I fetch the real note from the server rather than storing it in the app so that the value only exists on the wire and in the log at the moment you trigger it, not as a static string someone could pull straight out of the APK.

## Beyond the challenge

Imagine a login screen that writes an entire authentication response to a diagnostic log. The screen might hide the token perfectly while a support capture retains it. In this challenge, the flag makes the leak easy to recognize; a real review would look for tokens, personal information and other values the logs do not need.

Inspect what gets recorded during sensitive actions, not just what the UI displays. Log access depends on the device, app and collection setup; this is not a claim that any installed app can read every other app's logs. Redacting secrets before logging is safer than hoping nobody obtains the diagnostic output.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Android Logcat guide](https://developer.android.com/tools/logcat): learn the filters and output formats for tracing a single app.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): read the testing material on sensitive data in logs.

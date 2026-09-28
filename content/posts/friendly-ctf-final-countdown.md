---
id: "friendly-ctf-final-countdown"
title: "Final Countdown"
category: "CTF Writeups"
date: "2026-09-27"
summary: "Five times to hit in a row, each counted only while the app genuinely reads that time, verified by the server, so you hook the exact clock the checker reads and feed it each target."
flag: "Securinets{th3_cl0ck_1s_th3_1nput_fa51adbf}"
tags:
  - "Mobile Security"
---

The last one on the track, and it does not sit still. Press Start and the server hands the app five times to hit, one after another. Each one only counts while the app genuinely reads that time — held for a moment, not just glimpsed once. There is a countdown, and you are not going to sit around until 02:17 actually rolls around five times over. So the app has to believe the clock says what you need it to say, on demand.

The obvious version of this, set the device clock, or hook `System.currentTimeMillis` — does not work here, and the ways it fails are the whole lesson. This is the challenge where you have to hook the exact right clock and nothing else.

## What you're looking at

The flow is entirely server-checked. `POST /clock/start` mints a session and returns five target times. As you hit each one, the app records it. When you have all five, `POST /clock/finish` sends the session and the recorded times back, and the server compares them against the targets it issued. Only an exact, complete match returns the flag. Faked local state does not score, because the app never decides, the server does.

You need the backend running (see `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`), Frida attached to the app, and `frida-trace` to find your target. Read the challenge code in jadx alongside; the logic lives in a plain class, `ShiftRoster`, which is very readable.

## Finding the way in

Look at how the app decides you hit a target. Two facts do all the work.

The app compares against a clock, and it holds several samples 200ms apart before it counts a hit — a single momentary match is not enough, you have to keep reading the target time for a few ticks in a row. So whatever you spoof, it has to stay spoofed across those samples.

The bigger trap is which clock. `ShiftRoster` reads time in more than one place, and they are not the same call. The time shown on screen comes from a `Calendar`. The time actually compared against the target comes from `LocalTime.now(zone)`. And the countdown runs on `SystemClock.elapsedRealtime()`. Three different time APIs, on purpose.

That means the shortcuts fail in instructive ways. Hook `System.currentTimeMillis` and you move everything at once, the display, the animations, the network timeouts — and the app falls apart in ways that look like a bug, not a bypass. Set the device wall clock and the countdown does not care, because `elapsedRealtime()` is monotonic and counts real seconds no matter what the calendar says, so you buy no extra time. Spoof the display clock and the checker is reading a different call, so nothing counts. Only one specific method feeds the comparison, and you have to find it and hook exactly that.

That is why the screen tells you to `frida-trace` first: watch which time method actually fires when the app checks a target, and you will see `LocalTime.now` light up while `currentTimeMillis` and the `Calendar` call sit there or do harmless work.

## The solve

Trace the time calls to confirm which one the checker uses:

```bash
frida-trace -U -f tn.securinets.ctf \
  -m "*!*now*" -m "*!*currentTimeMillis*"
```

Start a round in the app and watch `java.time.LocalTime.now(java.time.ZoneId)` fire on every sample while the round is running. That is the one.

Now, the target changes every step, and the app shuffles the order it shows them in, so a hook that returns one fixed time only ever lands a single step. The clean way is to make the hook return whatever the app's current target is, read straight from `ShiftRoster`. Then every sample of every step reads its own target, held automatically, and all five complete in sequence:

```javascript
Java.perform(function () {
  var LocalTime = Java.use("java.time.LocalTime");
  var Roster = Java.use("tn.securinets.ctf.challenges.finalcountdown.ShiftRoster");

  LocalTime.now.overload('java.time.ZoneId').implementation = function (zone) {
    var target = Roster.INSTANCE.value.getCurrentTarget();     // "HH:mm", or "" when idle
    if (target && target.length === 5) {
      return LocalTime.parse(target);                          // e.g. 02:17
    }
    return this.now(zone);
  };
});
```

Load it and start the run:

```bash
frida -U -f tn.securinets.ctf -l clock.js
```

Press Start. The hook feeds each live target back into the exact call the checker reads, so every step registers, held across its samples, and the app records all five in one run before the countdown expires. It then posts them to `/clock/finish`, the server matches them against the targets it issued, and the result block prints the flag: `Securinets{th3_cl0ck_1s_th3_1nput_fa51adbf}`.

The app does not judge the result, it says as much on screen — it just prints whatever the server sends. Get all five and the server sends the flag; anything short and it refuses, so there is no partial credit to fake.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This closes out the M7 run, and its point is different from the others. It is not about defeating one check, it is about picking the right thing to control and leaving everything else alone. When behaviour depends on state, the attacker who can rewrite that state wins, but only if they rewrite the specific state the decision reads. Move too much and the app breaks around you; move the wrong value and nothing changes. Precision is the skill. The three-clock split is there so that the lazy, sweeping hooks either do nothing useful or visibly wreck the app, and only a targeted hook on the one method that feeds the comparison gets you through.

The other half is server-side verification. The client reports what it recorded, but the server re-checks it against the targets it issued for that session, so a hooked app that lies about finishing gets nothing — the values still have to be right, and they still have to be complete. That is the correct shape for anything that matters: the client can act, but the authority to say "this counts" lives on the server. It is the same principle as the licence challenge, applied to a stream of events instead of a single boolean.

## Notes from building it

The three separate time APIs are the design, not an accident. The display clock and the checked clock are pinned to the same timezone so an honest app in any region shows a time that agrees with what it is grading, a mismatch there would read as my bug, not as a clue. But they are deliberately distinct calls, because hooking the shared bottom-level `currentTimeMillis` also drags Choreographer, animations and OkHttp timeouts along with it, and the app disintegrates in a way that looks broken rather than bypassed. Keeping the countdown on monotonic `elapsedRealtime` was the other firm choice: it means winning the timezone game buys you no extra time, so you cannot stall your way to the finish and actually have to make the hook reactive.

## Beyond the challenge

Imagine a rewards app accepting “I waited thirty seconds” because the phone supplies two timestamps thirty seconds apart. A controlled client can manufacture those readings without experiencing the delay. Feeding this challenge the values its checker expects is a small version of that problem.

Ask which clock the security decision trusts. If elapsed time is a condition for a server-side reward, the server should track that condition using its own state and time. This CTF deliberately accepts client observations as part of its protocol; completing it does not demonstrate that you changed the server's clock.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Frida JavaScript API](https://frida.re/docs/javascript-api/): practice tracing call order and capturing arguments.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with dynamic analysis and authentication testing.

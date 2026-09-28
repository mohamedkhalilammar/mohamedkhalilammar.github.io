---
id: "friendly-ctf-pinned"
title: "PINNED"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The app pins its server certificate so an ordinary proxy fails, so you disable the pinning on the device with Frida and read the audit response the app never shows."
flag: "Securinets{sealed_envelope_pin_bypassed}"
tags:
  - "Mobile Security"
---

Everything you set up for DOR is still needed here, and it still will not be enough. That is the screen talking, and it is being honest. You install your CA, you point the device at your proxy, you sign in, you press the button that fetches the report, and the request dies before it leaves the phone. No traffic in Burp. Just a certificate error.

The reason is certificate pinning. This screen does not care what your device trusts. It carries a copy of the real server's certificate baked into the app and compares the server it reaches against that copy. Your proxy's certificate is not a match, so the app refuses to talk to it. The flag is in a response the app never displays, so you have to get the proxy working, which means getting past the pin.

## What you're looking at

The flag lives in `GET /audit`, which returns a `note` field the app throws away, the same shape as the earlier network challenges. The difference is entirely in how the app connects. Looking at the networking code, there are two HTTP clients: an ordinary one, and a pinned one built with OkHttp's `CertificatePinner`:

```kotlin
val pinnedClient: OkHttpClient = OkHttpClient.Builder()
    .certificatePinner(
        CertificatePinner.Builder()
            .add(HOST, CERT_PIN)
            .build(),
    )
    .build()
```

The `/audit` call uses the pinned client. That single `CertificatePinner` is what your proxy is running into.

You need the DOR setup (proxy, CA installed — see `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`), plus Frida running against the app. `objection` sits on top of Frida and is the easy way in for this one.

## Finding the way in

The instinct after DOR is to just replay a signature again. It does not work here, and understanding why is the point of the challenge.

In DOR, the `X-Sig` header was computed over the token only, so a captured signature replayed onto any path. You might think you can lift a signature from a `/profile` call and reuse it against `/audit`. Try it and you get a `401`. The `/audit` signature is bound to the literal path `/audit`, not just the token, so a `/profile` signature is the wrong value and the server rejects it. The app builds the correct one itself, from inside the native library, and never shows it to you. So you cannot forge your way around the pin. You have to make the pinned request actually happen and then read it.

That means the pin has to go. And here is the thing about pinning: it is a check that runs on your device, in your process, on hardware you control. Anything that runs on your side can be turned off on your side. Frida attaches to the running app and rewrites the pinning logic in memory so it always says "certificate is fine", while your proxy carries on doing its job.

## The solve

Start your proxy and confirm the CA is still installed from the DOR challenge. Then, with the app running, disable pinning in one command. The screen tells you which one:

```bash
objection -g tn.securinets.ctf explore -s "android sslpinning disable"
```

`objection` hooks the common pinning implementations, OkHttp's `CertificatePinner` among them, and neuters them. Keep it attached.

Now sign in and press "Request the report". This time the pinned request goes through your proxy instead of dying. The app just says "Report retrieved" — it does not show you the report, on purpose. Read it in Burp's HTTP history:

```http
GET /audit HTTP/1.1
Host: 20.199.16.42:28443
Authorization: Bearer eyJ0eXAiOiJKV1Qi...
X-Sig: 4b7d...aa
```

```json
{
  "report": "Q3 access log — 14 anomalies flagged",
  "note": "Securinets{sealed_envelope_pin_bypassed}"
}
```

The `note` field is the flag: `Securinets{sealed_envelope_pin_bypassed}`.

If you prefer a raw Frida script over objection, the universal-pinning-bypass scripts published for Frida do the same job — hook the pinner, force it to pass. objection just packages that so you do not have to.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

Pinning is a good control, and this challenge is not here to tell you it is useless. It is here to show you exactly what it buys and what it does not. Pinning raises the cost of intercepting an app's traffic — a plain man-in-the-middle with a trusted CA no longer works, which stops a lot of casual snooping and a lot of malware. What it cannot do is stop someone who owns the device. On your own phone, in a process you can attach a debugger to, every check the app makes is a check you can rewrite. Pinning is a speed bump for the attacker who is also the device owner, not a wall.

The comparison with DOR is the real lesson, and it is about signing, not pinning. `/profile` signs the token and leaves the path free, so a captured request replays and IDOR is possible by hand. `/audit` folds the path into the signature, so a lifted signature is useless and you are forced onto the device to make the genuine, correctly-signed request fire. Same signing scheme, one deliberate difference in what it covers, and the whole shape of the attack changes. When you design one of these, decide on purpose what the signature protects, because that decision is the security boundary.

## Notes from building it

The signature is never logged and never shown in the UI anywhere. That was a firm rule while building this — a single `Log.d` of the header and you could read the correct value with `adb logcat`, forge the request from a laptop, and the entire "you must bypass pinning on the device" requirement would quietly evaporate. Pinning challenges love to leak their own escape hatch through debug logging, and I wanted this one to actually require the device work it advertises.

## Beyond the challenge

During an authorized mobile assessment, an app may reject your proxy certificate because it pins a particular server identity. Instrumenting the app can let you observe the plaintext at the endpoint or change the local check. That does not mean you have broken TLS between two uncompromised endpoints.

Keep the attacker model clear. This challenge gives you control of the device and process. Pinning can raise the cost of interception, but the server still needs authorization checks and careful response contents when a legitimate client is instrumented.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Frida JavaScript API](https://frida.re/docs/javascript-api/): review Java method replacement before adapting the hook.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with TLS and certificate pinning tests.

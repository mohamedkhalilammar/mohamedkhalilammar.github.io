---
id: "friendly-ctf-open-lines"
title: "Open Lines"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The login talks over plain HTTP and the response carries an extra field the app never shows, so anything sitting on the wire reads the flag out of the body."
flag: "Securinets{party_line_was_never_encrypted}"
tags:
  - "Mobile Security"
  - "Web Security"
---

Open this one and you get a login form with the username and password already typed in for you. There is nothing to guess and nothing to bypass. The screen even says it out loud: point your proxy at the device first, then sign in. So the whole challenge is about what you can see while the app talks to its server, not about the app itself.

The catch is that the flag never appears on the screen. You sign in, the app says something cheerful, and that is it. The flag is in the reply the server sends, in a field the app quietly ignores. Your job is to be standing between the phone and the server when that reply comes back.

## What you're looking at

The app makes one request here: a `POST /login` to its backend over plain `http://`, not `https://`. Because it is cleartext, you do not need to install any certificate. You just need a proxy the traffic passes through and a way to point the device at it.

You need three things:

- A running instance of the challenge backend. The shared setup for the whole track — how to start the server, the emulator, `adb` and the proxy — is written once in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`. Get that going before anything else.
- Burp Suite (Community is fine) or any intercepting proxy.
- The device pointed at the proxy.

To point the device at Burp:

```bash
adb shell settings put global http_proxy 10.0.2.2:8080
```

`10.0.2.2` is how an Android emulator reaches the machine it runs on. If your setup is different, HOSTING.md has the address to use.

## Finding the way in

The mistake beginners make here is trying to attack the app. You open jadx, you go looking for the flag in the code, you grep the APK for `Securinets{`. All dead ends, because the flag is not in the APK at all. It lives on the server, and it only travels to you when you log in.

The other trap is a proxy that is running but not actually seeing anything. Two things catch people:

First, Burp by default binds its listener to `127.0.0.1` only, which the emulator cannot reach. Go to *Proxy > Proxy settings > Proxy listeners*, edit the listener, and set the bind address to *All interfaces*. Miss this and the app just fails to connect and you think the challenge is broken.

Second, once traffic flows, you do not need to intercept and hold each request. Turn intercept off and let it all through. Everything gets logged in *Proxy > HTTP history* anyway, and that is where you read the answer.

With the proxy set up, sign in with the credentials already in the form (`player` / `player123`). The app shows:

```
Signed in. The server replied with more than this screen shows you.
```

That line is the nudge. It is telling you the reply had more in it than what you can see.

## The solve

Go to *Proxy > HTTP history* and find the `POST /login`. Click it and look at the response tab:

```http
POST /login HTTP/1.1
Host: 20.199.16.42:28000
Content-Type: application/json

{"username":"player","password":"player123"}
```

```json
{
  "token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9...",
  "session_note": "Securinets{party_line_was_never_encrypted}"
}
```

The `token` is what the app keeps and uses. The `session_note` is the field it throws away, and that is your flag: `Securinets{party_line_was_never_encrypted}`.

If you would rather not run the emulator, the exact same reply comes back to `curl`, because the request is just JSON over HTTP. You would need the request signature the app attaches, which is the one part that is not trivial to reproduce by hand — so the proxy route is the intended one and the easiest by far. Watch the app do the work and read what comes back.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This is the plain-transport lesson, the M5 corner of the OWASP Mobile Top 10. When an app talks over `http://`, everything in every request and every response is out in the open for anyone on the network path — the wifi you are on, a rogue access point, your own machine's proxy. Encryption is not something you can add later by being careful; if the connection is not TLS, the bytes are readable, full stop.

The specific bug on top of that is a server returning data the client does not need. The app only wants the token, but the server also sends `session_note`, and once it is on the wire it belongs to whoever is listening. The fix is two-part: use HTTPS so the body is not readable in transit, and do not send fields the client will not use. A response is not private just because your own UI does not render it.

## Notes from building it

I put the credentials right in the form so nobody wastes time thinking this is a login-bypass challenge. It is not. The whole point is to make you set up the proxy for the first time on the easiest possible case, so that when DOR and PINNED ask for the same skill against harder targets, the plumbing already works and you can trust it. The "more than this screen shows you" line is deliberately vague — it tells you to look at the traffic without telling you which field, because spotting the extra field yourself is the small thing worth learning here.

## Beyond the challenge

Imagine a companion app sending a session token over HTTP on a shared network. An observer on the traffic path could read the token, even if the screen never shows it. Likewise, hiding a field in the UI does not remove it from a response sent to the device.

Check both transport protection and response contents. Use TLS with certificate validation, and return only data the authenticated client needs. The CTF makes the observer's position easy to arrange; a real finding must explain how the attacker reaches that position and what the exposed data allows.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): work through network communication testing and cleartext traffic.

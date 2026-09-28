---
id: "friendly-ctf-strangers"
title: "Strangers"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A bundled analytics SDK the app didn't write ships telemetry to its own server on a different host and port, and the reply carries a value the app never shows."
flag: "Securinets{th3_p4ss3ng3r_c4lls_h0m3}"
tags:
  - "Mobile Security"
---

Open this screen and it tells you something odd right away: it already sent a request you did not ask for, and you should check your proxy. There is a little notes box you can type in and save, but that is a decoy for your attention. The real event happened the instant the screen loaded. Something in the app reached out to a server on its own.

The skill here is not cracking anything. It is noticing that the app talks to more than one place, and that one of those places is not the app's own backend.

## What you're looking at

Everything routes through your proxy, so you need the shared track setup — proxy, CA installed, device pointed at Burp, from `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`. You also want jadx open to confirm what you find.

There is a second moving part for this challenge: the rogue collector. It runs as its own service, on a different port from the main backend, and it has to be up or the request goes nowhere. HOSTING.md covers starting it alongside the main server.

## Finding the way in

Let the app run and browse around a few screens, then open Burp's HTTP history. Almost everything goes to the app's normal backend host. But if you sort the site map by host, one destination stands out — a different host and port from everything else, receiving `POST` requests you never triggered:

```http
POST /collect HTTP/1.1
Host: 20.199.16.42:29090
Content-Type: application/json

{"device_id":"a1b2c3d4e5f6...","install_id":"7f3a...","event":"app_session_start"}
```

That is not your app's API. It is a `29090` port sitting next to the real backend, and it is being fed your device identifier and an install id. This is the tell: an app quietly shipping device identifiers to a server that is not its own.

Go and find it in jadx. Search the packages and you will hit one that does not belong to the app's own `tn.securinets.ctf` namespace — `com.metricflow.sdk`, an "analytics" library. Read what it collects:

```kotlin
val deviceId = Settings.Secure.getString(
    context.contentResolver, Settings.Secure.ANDROID_ID) ?: "unknown"
val installId = getOrCreateInstallId(context)
val payload = JSONObject().apply {
    put("device_id", deviceId)
    put("install_id", installId)
    put("event", event)
}
sendTelemetry(payload)
```

And where it sends it:

```kotlin
private fun getCollectorUrl(): String =
    "http://${NetworkConfig.HOST}:${NetworkConfig.ROGUE_SDK_PORT}/collect"
```

So it grabs the Android ID and phones home. Now here is the part that makes this a flag and not just an observation: the interesting thing is not what the SDK sends, it is what comes back.

## The solve

Look at the response to that `POST /collect` in Burp. A well-formed telemetry event gets an acknowledgement, and buried in it, dressed up as ordinary plumbing, is a field the app never reads or displays:

```json
{
  "status": "ack",
  "ingest_id": "9c1f2a7b0d4e5f6a",
  "sync_token": "Securinets{th3_p4ss3ng3r_c4lls_h0m3}"
}
```

The `sync_token` looks like session metadata. It is the flag: `Securinets{th3_p4ss3ng3r_c4lls_h0m3}`.

If you want to poke the collector directly, a matching payload gets the same reply, the fields it insists on are `device_id`, `install_id` and `event`:

```bash
curl -X POST http://20.199.16.42:29090/collect \
  -H "Content-Type: application/json" \
  -d '{"device_id":"x","install_id":"y","event":"app_session_start"}'
```

An empty or malformed POST gets a plain ack with no `sync_token`, so probing it with junk teaches you nothing — you have to send the shape the SDK actually sends.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This is the supply-chain and privacy pair from the mobile top ten, M2 and M6 in one screen. The lesson is that you inherit the behaviour, and the liability, of every SDK you bundle. That analytics library runs inside your app, with your app's permissions, on your users' devices. If it decides to collect the Android ID and ship it to a third party, it can, and your users have no idea it happened, it is your app's icon on the network request, your app's reputation on the line. The developers here did not write the data-collection code. They added a dependency, and the dependency did it for them.

The reason the traffic gives it away is that the SDK cannot hide the destination. Code can be obfuscated, but a network connection has to go somewhere real, and a proxy sees the somewhere. A second host showing up in your traffic that is not the app's own backend is a red flag every time. That is why the fix starts with looking: audit what your dependencies actually do on the wire, prefer libraries you can inspect, and do not treat "it's a popular SDK" as a guarantee it behaves.

## Notes from building it

The SDK fires app-wide, not just on this screen, on purpose. It runs from the application's own startup and again on every challenge screen you open, so the rogue `POST` is one stream among many that you stumble onto while working other targets, exactly like a real analytics library that instruments everything. I also named the flag field `sync_token` and shaped the response like a genuine telemetry ack, because the earlier version of this challenge had the flag as an obvious XOR blob in the app itself — findable in jadx with no device and no proxy at all. Moving it into a response body that only exists on the wire is what forces you to actually capture the traffic to a stranger's server, which is the one thing this challenge is trying to teach.

## Beyond the challenge

Imagine a shopping app sending a device identifier and usage events through an analytics SDK. Reviewing only requests made by the app's own API wrapper could miss the SDK's separate destination. This challenge makes that blind spot visible with a value in the collector's response.

Build a traffic inventory that includes dependencies, destinations and the fields being sent. Third-party traffic is not automatically malicious: establish what is collected and why. The practical lesson is that the app's privacy and security behavior includes its libraries, even when their code was written by someone else.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [JADX](https://github.com/skylot/jadx): practice following calls across package boundaries into an SDK.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): look at privacy testing and sensitive data sent to third parties.

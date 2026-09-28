---
id: "friendly-ctf-dor"
title: "DOR"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The profile endpoint takes any valid token and hands back whichever account id you ask for, so you change the id in the request and read someone else's record."
flag: "Securinets{f4c9a2e7-b81d-4f3a-9c6e-2b7d5a1f8e3c}"
tags:
  - "Mobile Security"
  - "Web Security"
---

Same idea as Open Lines, with the difficulty turned up one notch. You log in, you load your own profile, and everything looks normal. The twist is that the request which fetches your profile names a specific account by number, and the server does not check that the account is yours. Ask it for a different number and it hands over a stranger's record.

The other change is that this screen talks over HTTPS. Your proxy will see nothing until the app agrees to trust it, so there is a small setup step before you can even start looking.

## What you're looking at

Two requests matter. `POST /login` gets you a token. Then `GET /profile/<id>` fetches an account's details, sending your token in an `Authorization: Bearer` header. Both go to the backend over `https://`.

Because it is HTTPS, your proxy cannot read the traffic out of the box — the app checks the certificate and yours is not it. This particular app was built to trust user-installed certificates (a real app should never do this, which is part of the lesson), so you can slip your proxy's certificate onto the device and it will accept it.

You need the shared track setup from `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`, plus the Burp CA installed on the device. The short version:

```bash
adb shell settings put global http_proxy 10.0.2.2:8080
# then export the Burp CA and install it:
# Settings → Security → Encryption & credentials → Install a certificate → CA certificate
```

Getting the CA in the right slot is the part that trips people up; HOSTING.md walks through it if the app still refuses to connect.

## Finding the way in

With the proxy working, sign in and press "Load my profile". Watch it in Burp's HTTP history. You will see the fetch go out as something like:

```http
GET /profile/6 HTTP/1.1
Host: 20.199.16.42:28443
Authorization: Bearer eyJ0eXAiOiJKV1Qi...
X-Sig: 9f2c...e1
```

Your account is id `6`. The response is your own boring record:

```json
{
  "id": 6,
  "name": "Player Account",
  "email": "player@example.com",
  "internal_ref": "REF-2024-88204"
}
```

Now, the obvious question. What happens if you ask for `/profile/5`? Or `/profile/4`? The id is right there in the URL and it is just a small number. Nothing about the request says "this is mine" except the token, and the token is the same no matter which id you request. If the server trusts the token and never cross-checks it against the id, then any logged-in user can read any account.

There is one thing that usually stops this kind of edit: a signature. The app attaches an `X-Sig` header, an HMAC over part of the request. If that signature covered the URL, then changing `6` to `4` would break it and the server would reject the request. Here it does not. The signature is computed over the token only, and the token does not change when you change the id. So the same `X-Sig` stays valid for every id you try. That is the detail that makes this solvable by hand, and it is deliberate, you will meet the opposite choice in the next challenge.

## The solve

Right-click the `GET /profile/6` request in Burp and send it to Repeater. Change the number in the URL to `4` and hit Send. Leave the `Authorization` and `X-Sig` headers exactly as they were:

```http
GET /profile/4 HTTP/1.1
Host: 20.199.16.42:28443
Authorization: Bearer eyJ0eXAiOiJKV1Qi...
X-Sig: 9f2c...e1
```

```json
{
  "id": 4,
  "name": "Diana Prince",
  "email": "diana@example.com",
  "internal_ref": "Securinets{f4c9a2e7-b81d-4f3a-9c6e-2b7d5a1f8e3c}"
}
```

Diana's `internal_ref` is not a reference code like yours — it is the flag: `Securinets{f4c9a2e7-b81d-4f3a-9c6e-2b7d5a1f8e3c}`.

Walking the ids up and down is worth doing anyway; most return ordinary `REF-...` strings, and id `4` is the one holding the answer.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why this was the bug

This is IDOR — Insecure Direct Object Reference, also called Broken Object Level Authorization. It sits in the M3 part of the mobile top ten, and it is one of the most common real bugs there is. The server authenticated you correctly: it checked that your token is valid. What it forgot is authorization, checking that the specific thing you asked for is a thing you are allowed to have. Authentication answers "who are you"; authorization answers "are you allowed to touch this". They are not the same step, and skipping the second one is how you end up reading other people's accounts.

The fix is one line of logic on the server: compare the token's subject (its `sub` claim, which says who you are) against the id being requested, and refuse when they do not match. Sequential, guessable ids like `/profile/4` make it worse because there is nothing to enumerate — you just count. But note the id itself is not the flag; the flag is an opaque field on the record, so you cannot skip the exploit and guess your way to it.

## Notes from building it

The signature scheme here is the quiet star of the challenge. I sign the token but not the path on `/profile` precisely so that a captured request replays when you rewrite the id, sign the path and IDOR becomes unsolvable by interception, which would defeat the whole point. The very next challenge, PINNED, signs the path on purpose, and comparing the two is the thing I most want a player to notice: the same signing machinery, one small difference in what it covers, and a completely different outcome for an attacker.

## Beyond the challenge

Think of a customer portal fetching `/invoices/1042`. If changing the number to `1043` returns another customer's invoice under the same login, authentication worked but object authorization failed. That is the same mistake as this profile endpoint.

Test with two accounts you control and compare ownership checks, rather than enumerating unrelated users. The server must check permission for the requested object on every request. Random identifiers can make guessing harder, but they do not establish ownership. [PortSwigger's IDOR material](https://portswigger.net/web-security/access-control/idor) provides practice examples of this distinction.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [PortSwigger IDOR labs and explanation](https://portswigger.net/web-security/access-control/idor): practice checking object ownership across two accounts.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): connect the API authorization issue to mobile authentication testing.

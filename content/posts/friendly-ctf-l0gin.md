---
id: "friendly-ctf-l0gin"
title: "L0gIn"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The login form builds a SQL query by pasting your input into a string, so a single quote breaks out and you log in as admin without the password."
flag: "Securinets{qu0t3_y0ur_w4y_1n_3b3a28f6}"
tags:
  - "Mobile Security"
---

A login form. Username, password, sign in. The instruction on screen is direct: sign in as the admin, and no, you do not have the password. There is a guest account you can find your way into, but guest gets you nothing. You need the admin row, and you are going to get it by breaking the query instead of guessing the password.

## What you're looking at

![The L0gIn sign-in form](/media/friendly-ctf-2026/challenge-login-form.webp)

You read the code with `jadx`, then interact with the running app on a device or emulator. The flag is confirmed by the backend, so the server needs to be up. Setup and backend details are in `Writeups/Securinets-Friendly-CTF-2026/HOSTING.md`.

The class is `tn.securinets.ctf.challenges.l0gin.L0gInActivity`, and the login logic is in `LoginGate`.

## Finding the way in

Open `LoginGate` and look at how it checks your credentials. The whole bug is one line:

```kotlin
val query = "SELECT * FROM users WHERE username = '$username' AND password = '$password'"
val cursor = readableDatabase.rawQuery(query, null)
```

Your username and password are pasted straight into the SQL text. There is a local `users` table with two rows, a `guest` and an `admin`, and the app decides what you are based on which rows the query returns: an admin row means admin, any row means guest, no rows means rejected.

The screen gives you the nudge, too: "Try breaking the query first. A single quote is enough to see if it reaches the database." That is the classic first probe. Type a single `'` into the username and submit. Because the quote lands inside a string that is being concatenated, it unbalances the SQL and the query errors out — which tells you your input is reaching the database as code, not just as data.

Once you know that, you do not need the password at all. You need to make the query return the admin row.

## The solve

Put this in the username field:

```
admin'--
```

and leave the password as anything (or blank). The query the app builds becomes:

```sql
SELECT * FROM users WHERE username = 'admin'-- ' AND password = ''
```

The `'` closes the username string right after `admin`, and `--` turns the rest of the line into a comment, so the password check never happens. The query returns the admin row, the app sees `role = admin`, and you are in as admin.

A more general payload does the same job by making the condition always true:

```
' OR 1=1 --
```

That returns every row, and since the admin row is among them, the app lands on admin.

There is one more moving part. Reaching admin locally is not the finish line — the app then calls `/frontdoor/verify` on the backend, which re-runs the same injection against its own copy of the table and returns the flag only if an admin row comes back. So the injection has to genuinely work, and a dead server means no flag on this one. With the server up, a working payload pops the flag at the top of the screen:

```
Securinets{qu0t3_y0ur_w4y_1n_3b3a28f6}
```

Source is in `tn.securinets.ctf.challenges.l0gin` inside the APK; the packaged challenge is `Writeups/Securinets-Friendly-CTF-2026/mobile/app0gin`.

## Why it works

SQL injection is what happens when a program cannot tell the difference between the query it meant to run and the data you handed it, because it built the query by gluing strings together. Your `'` stops being a character in a username and starts being punctuation in the SQL, and from there you steer the statement. This is exactly the same bug that lives on web backends, except here it runs on the phone, against a local SQLite database.

The fix is old and boring and correct: parameterised queries. If the code had used `rawQuery("... WHERE username = ? AND password = ?", arrayOf(username, password))`, your quote would have been treated as a literal quote inside the username, matched nothing, and the injection would be dead. This is the input-validation lesson from the OWASP Mobile Top 10 — untrusted input has to be kept as data, and the database driver already gives you the tool to do it.

## Notes from building it

I deliberately left the vulnerable query string-concatenated and only parameterised the *seeding* insert, because parameterising the login query would delete the challenge. The admin password is random per session and comes from the server, not a literal in the APK, so there is no fixed value to read in jadx or pass between players — reading the code tells you *how* to get in, not a password to type. Guest stays a real literal (`guest/guest1234`) on purpose: it is the scaffolding that shows you the form works and makes the goal concrete, which is admin specifically, not "get in somehow."

## Beyond the challenge

Imagine a local notes app that builds a search query by concatenating whatever the user types. A quote can change the SQL statement instead of remaining part of the search text. A login form makes the effect obvious here, but the same mistake can occur in search, filters or a content provider.

Follow input into the database call and check whether it is bound as data. The fix is parameterized queries, not a list of forbidden characters. The [Android SQL injection guide](https://developer.android.com/privacy-and-security/risks/sql-injection) covers SQLite examples. A local injection's impact still depends on which data and operations that query can reach.

## Keep learning

- [Hextree](https://www.hextree.io/courses): continue with Android security courses and hands-on exercises.
- [Frida JavaScript API](https://frida.re/docs/javascript-api/): read Java.use and method replacement, then write a hook for another local check.
- [OWASP Mobile Application Security Testing Guide](https://mas.owasp.org/MASTG/): study authentication checks enforced by the client.

# Straight Key — challenge page copy (FINAL)

**Category:** Malware / RE · **Difficulty:** medium · **Flag:** one, submitted manually
**Attachment:** `KeyboardSelfTest.exe` — this is `build/KeyboardSelfTest-go.exe` renamed at
packaging time. The Go build is the one that ships.

## Brief

We pulled this off a workstation that had been behaving oddly. It is a keyboard self-test
utility — it types a fixed pattern into whatever window is in front of it, then stops.

The odd part is that it keeps typing the same character. Whatever it is saying, it isn't
saying it in the letters.

**Get the message out of the keyboard.**

## Before you run it

- **Run it in a VM, and snapshot first.** Standard practice for this track. This sample
  presses a key and waits — it does not touch your disk, your registry or the network, and
  it starts no other process — but you should never make an exception to the rule, and we
  would rather you did not start here.
- **Put an empty Notepad in front before you start it.** The keystrokes go to whichever
  window has focus, and you do not want them landing in a browser address bar.
- It runs for about **2 minutes**, repeating itself six times. If you miss the start, wait
  — the next pass is coming.
- **Running it under Wine? Check your display first.** Wine can pick the Wayland driver even
  when you launched it under `xvfb-run`, and then the keystrokes go to your real desktop
  instead of the isolated display. Unset `WAYLAND_DISPLAY` and give it a dedicated Xvfb
  prefix, or run it in a Windows VM and avoid the question entirely.

## Submitting

What you recover is **uppercase letters only**. Wrap it exactly:

```
Securinets{YOURRECOVEREDSTRING}
```

Do not lowercase it, do not add underscores, and do not go looking for another layer —
**what you decode is the answer.** The transport used here physically cannot carry
lowercase, underscores or braces, which is why this flag looks different from the others.


## Hint ladder

CTFd tiered hints, unlocked at a point cost. Agreed in session 17, written down here in
session 18.

| # | Hint |
|---|---|
| 1 | Every character it types is identical. So what is different between them? |
| 2 | It's Morse code. |
| 3 | The dots and dashes are how long each key is **held** — not the gaps between them. |
| 4 | Windows will tell you if a key is currently down. Look up `GetAsyncKeyState`. |

**The ordering is deliberate and it is counterintuitive: protect the mechanism, not the
encoding.** Decoding Morse is a lookup table and teaches nothing. Realising that *duration*
carries the data, and building a monitor that captures it, is the entire lesson. So the
cheap hint gives away the encoding and the expensive ones give away the channel.

If a player is stuck with a correct capture and no idea what it means, spend hint 2 — that
is a stalled lookup, not a stalled lesson. **Never spend hint 3 to unstick someone who has
not yet tried to build a monitor**; it hands them the thing they came to learn.

## Editor's notes — not player-facing

- **The Go build ships; the C build is superseded.** `build/KeyboardSelfTest-go.exe` is the
  release artifact and is renamed to `KeyboardSelfTest.exe` for players. The "six times /
  about 2 minutes" line above is correct for it. The old C binary at
  `build/KeyboardSelfTest.exe` transmits once in about 17 seconds and must not be attached —
  it is kept only as build history.
- **Do not ship a starter monitor**, and do not link one. Writing the receiver is the
  exercise; `solution/keyboard-monitor.ps1` is the answer key and stays internal.
- The brief points at the keyboard and never at the method — keep it that way. "Whatever it
  is saying, it isn't saying it in the letters" is as far as it goes.

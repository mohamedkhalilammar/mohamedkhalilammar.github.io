---
id: "friendly-ctf-water-run-revenge"
title: "Water Run: Revenge"
category: "CTF Writeups"
date: "2026-09-27"
summary: "A Sekiro-inspired fly-hacking challenge: find live vertical velocity with Cheat Engine, or recover the flag offline by reversing the native DLL."
flag: "Securinets{v3locity_is_gr4vity_with_attitude}"
tags:
  - "Reverse Engineering"
  - "Game Hacking"
---

Same game, same wall at 8,000 points, same jump that isn't enough. If you cleared Water Run, you already know what to reach for: find the jump value, make it bigger, hop over. So you attach Cheat Engine, set the type to `Float`, scan for `9.25347`, and get nothing. That value doesn't exist in this build. There's no helpful `JUMP` readout on the HUD either, and nothing on screen tells you what to look for.

That's the point of Revenge. The wall is beaten by the same *technique* — scan, edit, clear, but there's no constant to hand you the address this time. You have to find a value you don't know the value of.

## What you're looking at

Windows x64 Godot build again. Unzip, keep `WaterRunRevenge.exe` and `cgchallenge.dll` together, run the exe. Cheat Engine again, and the same promise: no anti-cheat, no anti-debug, no integrity checks. The intended route uses memory scanning. Solve Water Run first if you have not used Cheat Engine before; this route assumes you can already attach and narrow a scan. There is also an offline static reverse engineering route below.

## Finding the way in

Why does the exact-value scan fail? In the first game the jump was a persistent number that just sat there at `9.25347`, so you could search for it directly. Here there's no such stored field. What actually controls your jump is your **vertical velocity**, how fast you're moving up or down right now — and that number is never sitting still. On the ground it's `0`. The instant you jump it spikes to a positive value. Then gravity eats it, frame by frame, until it goes negative on the way down and returns to `0` when you land.

A value that's different every time you look at it can't be found with an exact-value scan, because you never have an exact value to type in. But Cheat Engine has a whole workflow for exactly this, and it's the thing this challenge exists to teach: the **Unknown Initial Value** scan, followed by rounds of *increased* / *decreased* / *unchanged* to squeeze the list down by how the value moved rather than what it is.

One warning before the steps, because it's the trap here. Your character's *height* off the ground moves up and down with the jump too, so it survives the same filters velocity does, and if you edit height you'll fly over the wall, but the wall's gate checks how high your velocity peaked, not where your body is, so a height edit sails over and the game still refuses you. You want velocity, not position. The tell that separates them: your height is never negative, but your velocity goes negative for the entire descent. When you're pinning candidates, the value that dips below zero while you're falling is the one you want.

## Method 1: find and freeze vertical velocity

This route can be unreliable in practice. Cheat Engine can sometimes cause the game to crash during a session, and editing or freezing the wrong address can corrupt game state. A crash does not prove you found the right value. Change one candidate at a time, undo edits that do not behave as expected, and start a fresh scan after restarting the game because addresses can change. If repeated crashes get in the way, use the static route below; it does not attach to or run the game.

The idea is to snapshot the velocity in a known state, do a known action, and scan for how it changed. Stand still, jump, fall, repeat, filtering each time:

1. **Attach** to the `WaterRunRevenge` process.
2. Set **Value Type** to `Float`, **Scan Type** to `Unknown initial value`, and click **First Scan**. This grabs every float in the process as a baseline — millions of them. That's fine; you're about to filter.
3. Stand still on the ground for a moment. Your velocity is `0` and steady. Set Scan Type to `Unchanged value` and click **Next Scan** a couple of times. This throws out everything that's fluctuating on its own.
4. Jump, and while you're rising, click **Next Scan** with Scan Type `Increased value`. Velocity went from `0` up to positive, so this keeps values that went up.
5. While you're falling, use `Decreased value`. Velocity is dropping toward and past zero, so this keeps values that went down.
6. Back on the ground, standing still, use `Value between ...` and enter a small range like `-1` to `1`, since a resting velocity sits right at `0`. (Height doesn't rest at zero, so this pass helps drop it.)

Repeat the jump/rise-fall/land cycle, alternating `Increased value` / `Decreased value` / the near-zero range, and the list collapses each time. With well-timed scans, you can narrow it to a handful of candidates. If you filter out the real value, start a fresh scan. To confirm you've got velocity and not height, watch a candidate go negative while you fall, height won't.

Once you have the velocity address, the edit is the same shape as the first game, but you have to make it stick against gravity. Add it to the table and **freeze** it (tick its box) at a value comfortably above the wall's clearance threshold. Freezing holds it there instead of letting gravity pull it back down frame by frame. Now jump into the wall. A short way past it the game stops and shows the flag.

```
Securinets{v3locity_is_gr4vity_with_attitude}
```

Source for both variants is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/water-run/`; Revenge is compiled with `CG_VELOCITY_MODE`.

## Method 2: recover the flag offline

The [static method in Water Run](/writeups/friendly-ctf-water-run) also applies here: follow the native `open` callback in Ghidra, identify the flag reconstruction code, and reproduce its byte operations. The velocity and score checks gate the gameplay path, but they do not contribute a secret to flag decryption. The required data is already inside `cgchallenge.dll`.

Use the DLL from **WaterRun-Revenge-windows-x64.zip**. Do not reuse the beginner solver's offsets: Revenge has a different layout and a 45-byte encrypted flag. Its `.rdata` section starts at virtual address `0x357a17000`, backed by file offset `0x5200`.

| Data | Virtual address | File offset | Bytes |
|---|---|---|---|
| Scattered ciphertext | `0x357a17380` | `0x5580` | 45 |
| Permutation | `0x357a173c0` | `0x55c0` | 45 |
| XOR mask | `0x357a173f0` | `0x55f0` | 16 |
| Masked master key | `0x357a17400` | `0x5600` | 32 |
| Authentication tag | `0x357a17460` | `0x5660` | 16 |
| Salt | `0x357a17470` | `0x5670` | 16 |

The calculation is unchanged: unmask the master key, derive the working key with HMAC-SHA256 over `vault|` plus the salt, reorder the ciphertext, verify its tag, and XOR it with the HMAC-generated stream. The linked walkthrough explains how to recognize these steps in the native code.

Save this as `solve_static_revenge.py`:

```python
#!/usr/bin/env python3
"""Recover Water Run: Revenge's flag from the distributed DLL, without loading it."""
import hashlib
import hmac
import sys
from pathlib import Path

blob = Path(sys.argv[1]).read_bytes()
expected = '5800f63582f8d603effaaf114e07eeac70745b0ecab23ce355d870957047f2c1'
if hashlib.sha256(blob).hexdigest() != expected:
    raise SystemExit('Different DLL build: recover its table offsets before using this solver.')

# Raw file offsets, not virtual addresses.
scattered = blob[0x5580:0x5580 + 45]
order = blob[0x55c0:0x55c0 + 45]
mask = blob[0x55f0:0x55f0 + 16]
store = blob[0x5600:0x5600 + 32]
tag = blob[0x5660:0x5660 + 16]
salt = blob[0x5670:0x5670 + 16]

master = bytes(value ^ mask[i % 16] for i, value in enumerate(store))
key = hmac.digest(master, b'vault|' + salt, 'sha256')
cipher = bytes(scattered[i] for i in order)
# Authenticate the reordered ciphertext with the same 10-byte suffix.
check = hmac.digest(key, cipher + b'|8000|A7C3', 'sha256')[:16]
if not hmac.compare_digest(check, tag):
    raise SystemExit('Tag mismatch: check the table offsets and byte order.')

stream = b''.join(
    hmac.digest(key, salt + counter.to_bytes(4, 'little'), 'sha256')
    for counter in range((len(cipher) + 31) // 32)
)
print(bytes(a ^ b for a, b in zip(cipher, stream)).decode('utf-8'))
```

Then run it with Python 3.8 or later:

```bash
python3 solve_static_revenge.py cgchallenge.dll
```

I verified this against the DLL extracted directly from the distributed Revenge ZIP. It prints `Securinets{v3locity_is_gr4vity_with_attitude}` without loading the DLL, running Godot or attaching Cheat Engine. The script rejects other builds rather than silently reading the wrong offsets. It is also included in the source bundle at `water-run/native/tools/solve_static_revenge.py`.

## Why it works

The unknown-initial-value scan is the workhorse of memory editing, and it's worth understanding what it actually compares. Cheat Engine doesn't track a trend over time. Each `Next Scan` compares the current value of every surviving address against what that address held at the *previous* scan. That's why the counterintuitive part works: velocity falls steadily during a jump, so it's tempting to think "increased value" could never match it. But your snapshots aren't continuous — you snapshot on the ground (`0`), then mid-rise (positive), then mid-fall (negative). `0 → +6` is an increase, `+6 → -5` is a decrease, `-5 → 0` lands in your near-zero range. You're comparing discrete moments you chose, not the smooth curve in between.

The bigger lesson is that "the value isn't there" almost never means you're stuck; it means the value doesn't persist, and you switch from *what is it* to *how does it change*. Any quantity you can make move on command, by jumping, taking damage, spending a coin — you can corner with increased/decreased passes even when you never once know its number. That's a strictly more powerful tool than the exact-value scan, and it's why this one's worth the extra effort.

## Notes from building it

The idea came from trying fly hacking in Sekiro myself. [This Sekiro fly-hacking video, starting at 0:18](https://www.youtube.com/watch?v=jDjXB7atDDM&t=18s), was the inspiration: search for movement-related values, watch how they change during a jump, then edit them to affect movement. Revenge turns that experiment into a smaller practice challenge. The memory-editing route can still be fiddly or crash-prone, which is why I have included the static alternative too.

Revenge is the same source as Water Run compiled a second time, and the interesting design problem was stopping it from being trivially reducible to the first game. Both builds keep the score as an easy-to-find changing integer, and in an early layout the interesting float sat 8 bytes after the score in both, which meant anyone who'd solved Water Run could find the score in Revenge, add eight, and land on velocity without ever running an unknown-value scan, skipping the entire lesson. So in this build those bytes hold dull constants instead, and velocity lives elsewhere. The height-versus-velocity decoy is real and I left it in on purpose: editing height clears the wall visually and the gate still refuses, because noticing that your body position and your jump strength are different values is part of what the challenge is teaching. Freezing rather than one-shot editing is the practical detail playtesters missed most — set the value once and gravity claws it back before you reach the wall.

## Beyond the challenge

The Sekiro experiment linked below is the personal connection behind this challenge: watching a movement value change, then testing how an edit affects the character. The useful mental model is to distinguish position, velocity and the rules that update them instead of treating every changing float as the same thing.

For a multiplayer system, the corresponding defensive question is whether the server accepts client movement as truth. [Server-authoritative logic](https://docs.unity.com/en-us/cloud-code/server-authority) can validate shared state. This offline CTF deliberately leaves its state editable; success here does not imply that the same memory edit would bypass a real game's server checks.

## Keep learning

- [Sekiro fly-hacking reference (0:18)](https://www.youtube.com/watch?v=jDjXB7atDDM&t=18s): the video that inspired my own experiment and this challenge.
- [Water Run static reverse engineering walkthrough](/writeups/friendly-ctf-water-run): follow the shared flag reconstruction logic before applying Revenge’s offsets.
- [Cheat Engine tutorial](https://wiki.cheatengine.org/index.php?title=Tutorials:Cheat_Engine_Tutorial_Guide_x64): focus on unknown initial values, repeated scans and floating-point types.
- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): compare how the two DLLs store and update movement state.

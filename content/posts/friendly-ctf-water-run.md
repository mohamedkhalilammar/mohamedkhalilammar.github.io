---
id: "friendly-ctf-water-run"
title: "Water Run"
category: "CTF Writeups"
date: "2026-09-27"
summary: "Two ways through a Godot runner: edit jump power with Cheat Engine, or reverse the native DLL and recover the encrypted flag offline."
flag: "Securinets{1f_th3_g4me_w0nt_l3t_y0u_w1n_ch4nge_1t}"
tags:
  - "Reverse Engineering"
  - "Game Hacking"
---

You play a runner. You're carrying a six-pack of water down a Tunisian medina alley, a crowd is chasing you, and you dodge and jump obstacles while the score climbs. It plays fine. Then at 8,000 points a wall drops across all three lanes at once, and your jump doesn't clear it. It's not a timing thing you can practice. The wall is taller than you can jump, full stop.

The description says it out loud: the game will not let you win, so change the game. This is a memory-editing challenge, and if you've never done one, this is a good first one to do slowly, because the technique is the same on every game you'll ever look at.

## What you're looking at

It's a Windows x64 build made in Godot. Unzip it and keep `WaterRun.exe` and `cgchallenge.dll` in the same folder — the DLL is where the game's real state lives, and it matters later. You need one tool: **Cheat Engine** (from `cheatengine.org`). Two things to expect installing it. The installer tries to bundle extra software, so untick that. And your antivirus may complain, because Cheat Engine is a debugger that attaches to other processes, that's normal for what it does, and the official site is the safe download.

There's no anti-cheat here, no anti-debug, no integrity check. Attaching a scanner and editing memory is the intended solution, not something you're sneaking past.

## Finding the way in

The game gives you a gift on the HUD: a `JUMP` readout, showing a very specific number, `9.25347`. That precision is the tell. Normal gameplay values are round-ish or noisy; a number sitting at five decimal places like that is a constant someone put there, and constants are easy to find in memory because almost nothing else in the process holds that exact value.

The wall itself confirms when you've won. Its face shows `JUMP POWER` and a `CLEARANCE` verdict. At the normal jump value it reads `INSUFFICIENT`. Raise the value high enough and it flips to `SUFFICIENT`. So you have a live indicator telling you whether your edit has taken effect before you ever commit to the jump.

The first thing a beginner tries that doesn't work: playing better. There's no skill ceiling that clears this wall, and grinding the score higher does nothing to your jump. The value on the HUD is the only thing that decides whether you make it over, and it's a real number in the game's memory, so that's what you change.

## Method 1: edit the jump power

Start the game and leave it running. Then, in Cheat Engine:

1. **Attach to the game.** Click the flashing-computer icon in the top-left corner, and pick the `WaterRun` process from the list.
2. **Scan for the jump value.** Set **Value Type** to `Float`, type `9.25347` into the value box, and click **First Scan**. Because that number is so precise, you should come back with just a few results, maybe one.
3. **Change it.** Double-click the result to send it to the table at the bottom, then set it to something much larger — `30` is plenty.
4. **Check the wall.** Look at the `CLEARANCE` line on the wall's face. It should now read `SUFFICIENT`.
5. **Jump it.** A short way past the wall the game stops and puts the flag on screen with a copy button. You don't have to land on anything — clearing the wall is the whole trick.

If your first scan gives back more results than you can deal with, narrow them the way you would in any game. The score is in memory too, as a `4 Bytes` value that keeps changing while you run: scan for the number on the HUD, run a little, scan again for the new number, repeat until one address survives. The jump value lives 8 bytes after the score, in the same block of game state, so once you've pinned the score you know exactly where to look.

Overshoot and fly off the map? Lower the number and try again. Nothing is lost.

```
Securinets{1f_th3_g4me_w0nt_l3t_y0u_w1n_ch4nge_1t}
```

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/water-run/`.

## Method 2: recover the flag with static reverse engineering

You can also solve this without launching the game. Import `cgchallenge.dll` from the **Water Run ZIP** into Ghidra and let analysis finish. Searching for `Securinets{` gets you nowhere: the flag is encrypted. The useful question is which code turns those encrypted bytes into the text the game displays.

### Find the native reward code

The DLL exports `cg_gdextension_init`; the source-level names such as `cg_receipt` are not exported functions you can simply search for. Follow the Godot method registration for `open` to its callback, then follow the call that fills the output buffer. In the beginner DLL checked here, that reconstruction routine starts at virtual address `0x357a12ac0`. Its caller checks a score of at least 8,000 and a phase value of `0x847e7445` before entering it.

These checks describe the gameplay route: cross the wall, then trigger the artifact pickup. They do not supply a secret to the decryption routine. All its inputs are constants in the DLL, so you can reproduce the calculation in Python.

Inside the routine, look for a loop that XORs 32 stored bytes with a repeating 16-byte mask. Next come the HMAC padding constants `0x36` and `0x5c`, SHA-256 operations, a byte-reordering loop, and a final XOR loop. The compiler builds `vault|` from immediate values, so a plain strings search may miss it. Use the disassembly alongside the decompiler when the optimized code looks confusing.

### Recover the data and translate the calculation

These are the arrays used by that routine in the distributed beginner build. The names below describe their roles; they are not symbols preserved in the DLL.

| Data | Virtual address | File offset | Bytes |
|---|---|---|---|
| Scattered ciphertext | `0x357a17380` | `0x5380` | 50 |
| Permutation | `0x357a173c0` | `0x53c0` | 50 |
| XOR mask | `0x357a17400` | `0x5400` | 16 |
| Masked master key | `0x357a17420` | `0x5420` | 32 |
| Authentication tag | `0x357a17490` | `0x5490` | 16 |
| Salt | `0x357a174a0` | `0x54a0` | 16 |

The `.rdata` section begins at virtual address `0x357a17000` and file offset `0x5000`. To convert an address in this section, subtract its virtual start and add its file start. Do not pass a virtual address directly to a Python byte slice.

The reconstruction is:

1. XOR the stored key bytes with the repeating mask to recover the 32-byte master key.
2. Derive the working key with `HMAC-SHA256(master, b"vault|" + salt)`.
3. Undo the scatter with `cipher[i] = scattered[permutation[i]]`.
4. Check the first 16 bytes of `HMAC-SHA256(key, cipher + b"|8000|A7C3")` against the stored tag. The suffix is exactly 10 bytes, with no NUL terminator.
5. Generate 32-byte stream blocks with `HMAC-SHA256(key, salt + counter)`, starting at zero and encoding the counter as four little-endian bytes. XOR the stream with the ciphertext.

Save this as `solve_static.py` beside the extracted DLL:

```python
#!/usr/bin/env python3
"""Recover Water Run's flag from the distributed beginner DLL, without loading it."""
import hashlib
import hmac
import sys
from pathlib import Path

blob = Path(sys.argv[1]).read_bytes()
expected = '74b64f87fbdea2e9573d622010fa67a83f40a1ba9deab262e7d0ef639c5c7e98'
if hashlib.sha256(blob).hexdigest() != expected:
    raise SystemExit('Different DLL build: recover its table offsets before using this solver.')

# Raw file offsets, not virtual addresses.
scattered = blob[0x5380:0x5380 + 50]
order = blob[0x53c0:0x53c0 + 50]
mask = blob[0x5400:0x5400 + 16]
store = blob[0x5420:0x5420 + 32]
tag = blob[0x5490:0x5490 + 16]
salt = blob[0x54a0:0x54a0 + 16]

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

Run it with Python 3.8 or later:

```bash
python3 solve_static.py cgchallenge.dll
```

It prints the same flag as the memory-editing route. I checked this against the DLL read directly from the distributed `WaterRun-windows-x64.zip`, without executing it or reading a plaintext flag file. The hash guard makes the offsets explicit: a rebuilt DLL or Water Run: Revenge needs its own analysis. The script is also included in the source bundle under `water-run/native/tools/solve_static.py`.

## Why it works

A game is a program, and the numbers it uses, score, health, your jump power — are sitting in the process's memory while it runs. Cheat Engine's whole job is to search that memory for a value you know (the number on screen), narrow the results until one address is left, and then let you edit or freeze it. An exact-value scan works here because the jump power is stored as a plain 4-byte float that doesn't move around, which is the simplest case there is.

The reason it's stored that simply is deliberate on my end, but the transferable point is about *where* game state lives. This value sits in a small native library the game calls, not in the game's scripting layer, precisely so that a 4-byte scan behaves the textbook way. When a scan for an on-screen number gives you clean results, you're looking at a plain value in memory. When it gives you nothing or noise, the value is boxed, encoded, or moved around, and you need a different technique, which is exactly the next challenge.

## Notes from building it

The hard part of a first game-hacking challenge isn't the hack, it's making the scan behave. Godot keeps script variables as tagged unions on a managed heap, so a score kept in GDScript scans as noise and editing a hit doesn't reliably change anything — the classic "I found it but it won't change" frustration. So the state that matters lives in a C library the game calls every frame, which is why the exact-value scan works the way the tutorials promise. The `JUMP` HUD readout and the wall's `CLEARANCE` verdict are both there as training wheels: they tell you the value exists, tell you what to scan for, and tell you the moment your edit is enough, so you can learn the loop without also guessing whether it's working.

## Beyond the challenge

Think of an online game whose client reports that a player made an unusually large jump. If the server accepts the movement without checking it, editing local state can affect the shared game. Water Run gives you a small offline model where changing one float is enough to alter movement.

The static solve adds a second lesson: a client that carries both encrypted content and everything required to decrypt it can be inspected offline. For shared game state, [server authority](https://docs.unity.com/en-us/cloud-code/server-authority) places trusted decisions outside the player's process. That would require more than moving this CTF's flag check into another client function.

## Keep learning

- [Cheat Engine tutorial](https://wiki.cheatengine.org/index.php?title=Tutorials:Cheat_Engine_Tutorial_Guide_x64): work through exact-value and floating-point scans.
- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): practice navigating cross-references and defining byte arrays.
- [Python hmac documentation](https://docs.python.org/3/library/hmac.html): review the digest calls used in the offline solver.

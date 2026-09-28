---
id: "mojo-slow"
title: "Slow: GDB-Python Automation"
category: "CTF Writeups"
date: "2026-05-01"
dateIsPlaceholder: true
summary: "Bypassing massive function-pointer obfuscation and usleep slowdowns via GDB scripting."
flag: "MOJO-JOJO{GD8_Scr1pt1ng_1s_4_Pow3rful_Sk1ll!}"
tags:
  - "Reverse Engineering"
---

This binary was an exercise in pure frustration—it was engineered to be agonizingly slow. The author implemented a massive array of *usleep* delays wrapped inside a labyrinthine dispatcher utilizing over **68 distinct function pointers** just to validate a single character. Manual brute-forcing would take years.

To break the temporal constraints, I turned to the **GDB Python API**. I engineered an automated debugging script that attached to the process, bypassed the sleep calls by manipulating the instruction pointer, and hooked directly into the return states. I commanded GDB to trace the EAX register for the sentinel value `0x22`, which indicated a successful character match.

This script tore through the first segment of the flag in seconds. The final hurdle was a hardcoded **SHA256 hash check** masking the last 4 characters. I quickly threw together a parallelized Python script to crack the remaining combinations, bridging the gap and capturing the flag.

---

## Key Takeaways

- The GDB Python API is incredibly powerful for automating complex memory manipulation and timing bypasses.
- Never manually brute-force timing delays. Nullify them at the instruction level.

## Beyond the challenge

Imagine analyzing a program that repeats the same validation path thousands of times with deliberate delays. Debugger automation can turn a repetitive observation into a script, making it possible to compare many runs consistently.

Changing execution also changes your experiment. Skipping a sleep in this challenge removes a delay, but in another program it could affect synchronization or state. Confirm that the modified path still reaches the behavior you mean to study. The lasting skill is automating a well-understood observation, not blindly skipping anything that takes time.

## Keep learning

- [GDB Python API](https://sourceware.org/gdb/current/onlinedocs/gdb.html/Python.html): automate breakpoints and repeated observations in a debugger.

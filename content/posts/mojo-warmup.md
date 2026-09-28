---
id: "mojo-warmup"
title: "Warmup: ELF Header Recovery"
category: "CTF Writeups"
date: "2026-06-01"
dateIsPlaceholder: true
summary: "Repairing a corrupted ELF header and bypassing multiple layers of ptrace anti-debugging."
flag: "MOJO{On_Th3_FlY_D3crypt1on_Is_Pr0!}"
tags:
  - "Reverse Engineering"
---

A classic reverse engineering scenario with a nasty twist. Booting the binary immediately returned a bleak `'Exec format error'`. Dumping the binary into a hex editor (*xxd*) exposed the first sabotage: the ELF magic header had been maliciously altered to read **.ASS** instead of **.ELF**. A quick hex patch brought the binary back to life.

Loading the patched file into **Ghidra**, I ran into the next wall: aggressive anti-debugging mechanisms. The binary was utilizing constructor-based `ptrace` calls to instantly self-destruct if a debugger attached. I patched out the ptrace checks, allowing for clean dynamic analysis.

With execution flowing, I mapped out the core decryption routine stationed at memory address **0x13e0**. It utilized a custom LCG seeded with `0x4b1d2c3a`. By actively ripping the generated key stream from memory and reversing the XOR logic against the protected data segment, the flag was successfully decrypted.

---

## Key Takeaways

- Deep understanding of the raw ELF specification is mandatory for diagnosing corrupted executables.
- Always intercept and nullify constructor-level ptrace checks before attempting dynamic instrumentation.

## Beyond the challenge

An analyst may receive a damaged or deliberately altered executable that standard tools reject before useful analysis begins. Comparing its header with the file-format specification can distinguish a small recognizable defect from an unsupported or unrelated format.

Work on a copy and record every repair. Making a loader accept a file is not evidence that the program is safe, and an anti-debug check may change what you see afterward. The useful habit is to separate format problems, analysis resistance and application logic instead of treating them as one mystery.

## Keep learning

- [ELF header specification](https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html): compare identification bytes and header fields with the actual format.
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): inspect early execution and debugger-related checks.

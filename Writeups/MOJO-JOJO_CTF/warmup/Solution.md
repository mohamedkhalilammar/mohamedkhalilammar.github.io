# Warmup
* ***Category : Reverse Engineering***  
 >   Description : **A quick stretch for your brain.**

An executable was attached ( *chall* ).

## Solution
Let's run this file :
```bash
$ ./chall
bash: ./chall: cannot execute binary file: Exec format error
```

Wait, what? A Linux binary that says "Exec format error" usually means it's for a different architecture or it's corrupted. Let's check the file type with `file`:

```bash
$ file chall
chall: data
```

"data" — that's definitely not right. Let's take a peek at the hex with `xxd`:

```bash
$ xxd chall | head -n 1
00000000: 7f41 5353 0201 0100 0000 0000 0000 0000  .ASS............
```

Aha! Every ELF binary should start with `\x7fELF`. Here it starts with `\x7fASS`. Someone is being funny. We can fix this with a quick python script to restore the magic bytes:

```python
with open('chall', 'rb') as f:
    data = f.read()
with open('chall_fixed', 'wb') as f:
    f.write(b'\x7fELF' + data[4:])
```

Now let's try running it again:
```bash
$ chmod +x chall_fixed
$ ./chall_fixed
System Boot...
Usage: ./chall <command>
```

Great, it works. Let's try some commands like `status` or `maint`:
```bash
$ ./chall_fixed status
System Status: NORMAL
```

Still no flag. Let's take a look at the disassembly using [Ghidra](https://ghidra-sre.org/).

Looking at the entry point and constructors, we see a call to `ptrace(PTRACE_TRACEME, ...)`. This is a classic anti-debugging trick. If we try to run it in a debugger, it will detect us and exit.

> **Note:** There are actually two ptrace checks. One in a constructor (init_array) and one deep inside the decryption routine. We'll need to patch these out with NOPs or just jump over them.

### Reversing the Logic

After bypassing the ptrace checks, we find a function at `0x13e0` that looks interesting. It uses a custom **Linear Congruential Generator (LCG)** to decrypt the flag.

1. **Seed Derivation:** It starts with `0x4b1d2c3a` and runs 100 iterations of bitwise rotations and XORs to get our starting seed.
2. **Decryption:** It uses that seed to generate a keystream that is XORed against encrypted data at `0x4080`.

Since we can see the constants for the LCG and the transformation, we can just write a script to do it for us.

***This is the script that will make that happen [solver.py](./solver.py)***

* By executing the script we find the following flag:

-------->     ***MOJO{On_Th3_FlY_D3crypt1on_Is_Pr0!}***

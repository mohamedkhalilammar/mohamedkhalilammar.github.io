---
id: "friendly-ctf-shift"
title: "Shift"
category: "CTF Writeups"
date: "2026-09-27"
summary: "The key check is a hand-written XOR-and-rotate loop compared against a table in .rodata, so there is no library call to trace and you have to read the loop and run it backwards."
flag: "Securinets{r0ll_1t_b4ck_0n3_byt3_4t_4_t1m3_5e02}"
tags:
  - "Reverse Engineering"
---

Same shape as Doorman. The program asks for a rotation key, you type something, it says `denied`. The challenge page warns you up front that tracing library calls won't help this time, because the program never hands your input to anything to be compared. It does the checking itself.

I built Shift specifically to break the Doorman trick. If you solve this one with `ltrace`, I got something wrong.

## What you're looking at

A stripped 64-bit Linux ELF called `nightshift`.

```bash
chmod +x nightshift
./nightshift
```

```text
rotation key: hello
denied
```

You'll need `objdump` (part of binutils, almost certainly installed already) and Python. Ghidra makes the reading part friendlier, but everything below works with `objdump` alone.

## Finding the way in

Try `ltrace` anyway. Seeing a tool fail tells you something too.

```bash
printf 'hello\n' | ltrace ./nightshift
```

```text
fgets("hello\n", 128, 0x7f48bdbb68e0)            = 0x7fff1e22a630
strcspn("hello\n", "\r\n")                       = 5
strlen("hello")                                  = 5
puts("denied")                                   = 7
+++ exited (status 1) +++
```

No `strcmp`. The only new thing is `strlen`, which is a small hint: it's checking how long your input is before anything else.

So you have to read the code. The binary is stripped, so there's no function called `check_key` to jump to. In Ghidra, the quickest route is to find the string `denied`, look at what references it, and you land in `main`. In `objdump -d`, you can find the call right after `strcspn`, which is where `main` hands your trimmed input to the checking function. In this binary that's `call 122f`. Here's that function:

```bash
objdump -d -M intel --no-show-raw-insn --start-address=0x122f --stop-address=0x1287 nightshift
```

```text
    122f:	push   rbx
    1230:	mov    rbx,rdi
    1233:	call   1040 <strlen@plt>
    1238:	mov    rdx,rax
    123b:	mov    eax,0x0
    1240:	cmp    edx,0x14
    1243:	jne    127e
    1245:	mov    edx,0x5a
    124a:	mov    ecx,0x0
    124f:	lea    rsi,[rip+0xe1a]        # 2070
    1260:	mov    eax,edx
    1262:	xor    al,BYTE PTR [rbx+rcx*1]
    1265:	rol    al,cl
    1267:	cmp    BYTE PTR [rsi+rcx*1],al
    126a:	jne    1280
    126c:	add    rcx,0x1
    1270:	add    edx,0x7
    1273:	cmp    rcx,0x14
    1277:	jne    1260
    1279:	mov    eax,0x1
    127e:	pop    rbx
    127f:	ret
    1280:	mov    eax,0x0
    1285:	jmp    127e
```

If you've never read assembly, this looks like a wall. It's about ten meaningful lines, so take them one at a time.

- `rbx` holds a pointer to your input.
- `cmp edx,0x14`: the length has to be 0x14, which is 20. Wrong length, return 0.
- `edx` starts at `0x5a`, and `add edx,0x7` bumps it by 7 every time round. So on round `i` it holds `0x5a + 7*i`.
- `rcx` (and its low byte `cl`) is the counter `i`.
- `rsi` points at address `0x2070`, a table of bytes.

The loop body, from `1260` to `1277`, does this for each `i`: take the low byte of `0x5a + 7*i`, XOR it with your input byte, rotate the result left by `i` bits, and compare with `table[i]`. Any mismatch returns 0. Survive all 20 rounds and it returns 1.

One thing trips people up here. You'd expect the rotate to be by `i & 7`, since a byte only has 8 bits, but the instruction just says `rol al,cl` with `cl` climbing to 19. That's fine. For 8-bit rotates the CPU reduces the count itself, and rotating a byte by 8 lands you back where you started. For reference, here's the C I actually wrote:

```c
static int accepts(const char *key)
{
    unsigned int n = (unsigned int)strlen(key);
    if (n != ROSTER_LEN)
        return 0;

    for (unsigned int i = 0; i < n; i++) {
        unsigned char v = (unsigned char)key[i];
        v ^= (unsigned char)(0x5au + 7u * i);
        v = spin(v, i);
        if (v != ROSTER[i])
            return 0;
    }
    return 1;
}
```

`spin` is a rotate left by `i & 7`. Now go get the table. `.rodata` is where compiled programs keep constant data:

```bash
objdump -s -j .rodata --start-address=0x2070 --stop-address=0x2084 nightshift
```

```text
 2070 352e34e8 8182f8f1 e6694b46 ad9a3255  5.4......iKF..2U
 2080 a57fd747                             ...G
```

Twenty bytes, matching the length check.

## The solve

The program applies two steps: XOR, then rotate left. To undo them you go in the opposite order, rotate right first, then XOR. Same as getting dressed: socks go on before shoes and come off after them.

```python
TABLE = bytes.fromhex("352e34e88182f8f1e6694b46ad9a3255a57fd747")


def ror(v, n):
    n &= 7
    return ((v >> n) | (v << (8 - n))) & 0xFF


key = bytes(ror(t, i) ^ ((0x5A + 7 * i) & 0xFF) for i, t in enumerate(TABLE))
print(key.decode())
```

```bash
python3 solve.py
python3 solve.py | ./nightshift
```

```text
overnight-rotation-7
rotation key: Securinets{r0ll_1t_b4ck_0n3_byt3_4t_4_t1m3_5e02}
```

If you undo the steps in the wrong order, you'll get mostly garbage but not total garbage. Bytes 0, 8 and 16 come out right (`o`, `t`, `o`), because those are the rounds where the rotation is zero and the order doesn't matter. If you see that pattern, you've got the pieces right and the order wrong.

Source for this one is in `Writeups/Securinets-Friendly-CTF-2026/reverse-engineering/02-night-shift/`.

## Why it works

A check that transforms your input and compares it with a stored answer is only as secret as the transform is one-way. XOR undoes itself. A rotate left is undone by a rotate right of the same amount. Neither loses any information, so the table in `.rodata` is really just the key with its coat on.

That's the general move for this whole family of crackmes. Find where the input gets compared, work out every step applied to it before the comparison, and apply the inverse of each step in reverse order to the stored value. When the steps stop being reversible, like a real hash, this stops working and you need a different idea. That's the next challenge.

## Notes from building it

The transform lives in one small Python file in my build tooling: the build uses it to generate the table and the official solver uses it to invert, so the challenge and its answer can't drift apart without the build failing. The build also runs `ltrace` against the finished binary and fails if a library compare ever shows up, because that would quietly turn Shift into a copy of Doorman. I kept each byte independent of the others on purpose, which is why the wrong-order mistake above still shows you a few correct letters instead of pure noise.

## Beyond the challenge

Imagine a proprietary configuration file with bytes XORed and rotated before they are stored. Recovering the transform can let an analyst read the configuration or build an interoperable parser. A custom-looking operation is not automatically cryptographic protection.

Translate the loop carefully and test the inverse on known input/output pairs. Order, byte width and overflow behavior matter more than how complicated the decompiler looks. The challenge gives you a small reversible transform; a real format may combine encoding, compression and proper encryption, so identify each layer before deciding how to handle it.

## Keep learning

- [Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html): practice translating byte loops into a small Python script.
- [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/): inspect loop inputs and outputs to check your translation.

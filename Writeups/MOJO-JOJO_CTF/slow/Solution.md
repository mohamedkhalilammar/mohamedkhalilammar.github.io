# Slow
* ***Category : Reverse Engineering***  
 >   Description : **Wait for it...**

An executable was attached ( *challenge* ).

## Solution
Let's run this file :
```bash
PS C:\Users\MSI\CTF\Slow> .\challenge.exe
Usage: ./challenge <flag>

PS C:\Users\MSI\CTF\Slow> .\challenge.exe AAAA
Wrong Flag!

PS C:\Users\MSI\CTF\Slow> .\challenge.exe AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
(Hang...)
```

Wait, it's just hanging. Looking at the process, it's not even consuming much CPU. It's just... *slow*.

Let's take a look at the disassembly using [IDA](https://hex-rays.com/ida-pro/).

Looking inside, the binary has some serious obfuscation. It uses a massive array of function pointers at `0x4350a0`.

1. **Dispatcher:** It takes each character of our input and uses it as an index to a function pointer table.
2. **Slowdown:** It calls `usleep` and has some complex-looking thread logic that's purely there to confuse us and waste time.

If we give it a length of 44, it starts the dispatcher. For each position, there are 68 functions. One for each character in our alphabet (`a-zA-Z0-9{}_-!`).

> Each "wrong" function returns a specific sentinel value (`0x22`). The "right" function does not.

If we were to brute force this manually, it would take weeks because of the `usleep` calls. But we have a trick.

### GDB Scripting

Since we know the "right" function has a different return value (and therefore different machine code), we can just inspect the memory of each function without having to *actually wait* for them to finish.

We can automate this process using GDB's Python API. We'll skip the `usleep` calls and just check the opcode at each function's address to see if it moves `0x22` into the `eax` register.

***This is the script that will make that happen [solve.py](./solve.py)***

### Final Hash

After the first 40 characters are recovered, the binary takes the *full* flag and hashes it with **SHA256**, then compares it against a hardcoded hash.

Since we know the first 40 characters, we only need to brute-force the last 4 characters (`3 suffix characters + the closing brace '}'`).

***This is the script that will make that happen [brute_suffix.py](./brute_suffix.py)***

* By executing the scripts we find the following flag:

-------->     ***MOJO-JOJO{GD8_Scr1pt1ng_1s_4_Pow3rful_Sk1ll!}***


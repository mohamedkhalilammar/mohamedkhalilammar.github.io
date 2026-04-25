# 13337
* ***Category : Reverse Engineering / Network***  
 >   Description : **The Final Boss.**

An executable was attached ( *chall* ).

## Solution
This is the final stage. Let's start with some basic reconnaissance. Running `strace` or checking network activity reveals that the binary is trying to connect to `127.0.0.1` on port `13337`.

```bash
$ strace -e network ./chall
socket(AF_INET, SOCK_STREAM, IPPROTO_IP) = 3
connect(3, {sa_family=AF_INET, sin_port=htons(13337), sin_addr=inet_addr("127.0.0.1")}, 16) = -1 ECONNREFUSED (Connection refused)
```

It looks like we need to have a server listening on that port to see what happens next.

### Static Analysis: The Fog of War

Opening the binary in IDA or Ghidra is... difficult. The binary is stripped, and most of the interesting logic seems to be in a custom section that is not immediately readable.

Looking at the `main` function, we see calls to `mprotect` and some XOR loops. This is a classic sign of **self-modifying code**. The binary decrypts its own logic at runtime using a static key (`0xDE`) and then another random "stabilization" key.

> To see the real code, we must use **pwngdb** to dive deep and set a watchpoint or a breakpoint after the decryption happens.

### Dynamic Analysis: Diving with PwnGDB

1. **Break on `mprotect`:** This is where the binary prepares to modify its own code memory.
2. **Identify the second decryption:** After the first decryption with `0xDE`, it uses a random key. We can break right before the code is executed and dump the memory.

Once the memory is dumped, we can see the logic of the network handshake. The client doesn't just send random data; it identifies itself with a **magic DWORD**: `0x0DF7B6FA`.

> 🔍 **Handshake Recon:** We can find this magic value by breaking on the first `send` call or by statically analyzing the decrypted code. The server must respond with its own magic: `0x8AE1AFF5` followed by some metadata.

### Analyzing Transformations with Watchpoints

To truly understand how our 32-byte payload is being mangled, we can set a **watchpoint** on the buffer in GDB:

```gdb
(gdb) watch *0x[buffer_address]
```

This lets us see every time the binary touches our data. We notice it's being XORed with the `session_id` (a 4-byte value the client sends at the start) and then passed to an MD5 routine.

### The Brute-Force Hack

The hashes look like this:
- `f9bfe8cb45a6795747f4d512ebc178d2`
- `afc285b2084d852d72b90c0afc40bfb1`
- ...

Since each chunk is only 4 bytes (a 32-bit integer), we can **brute-force** these hashes! Finding the 32-bit integer that produces each MD5 is a matter of a few minutes on a modern CPU.

### Reversing the Cipher

Once the 8 chunks are correctly validated, they are used as keys for a **multi-round custom cipher** that decrypts the actual flag.

- **Round 1:** XOR with the chunk byte.
- **Round 2:** Modular subtraction of another chunk byte.
- **Round 3:** XOR with the position index and a fixed chunk byte.

By setting breakpoints at each round and inspecting the registers, we can map out exactly which bytes of our payload are being used for which transformation. Reversed, we can recover the flag string.


***This is the script that will handle the handshake and send the corrected payload [solver.py](./solver.py)***

* By running the solver and letting the binary connect to it, we successfully receive the flag:

-------->     ***C0ngratulations_U_3arn3d_1t_19432850&***

# Intended solve

```
$ gdb ./gate
pwndbg> break ptrace
pwndbg> run
pwndbg> finish
pwndbg> set $rax = 0
pwndbg> continue
pwndbg> call (char *) reveal()
```

Last line prints the flag. `build.sh` replays this every build, under pwndbg and
under plain gdb.

## Why each line

| Command | What it does |
|---|---|
| `break ptrace` | Break on the library call. Resolves to `<ptrace@plt>` **even though the binary is stripped** — the dynamic symbol table has to survive for the linker, so no address hunting. |
| `run` | Stops inside `ptrace`, before it has done anything. |
| `finish` | Lets the call return. gdb shows the value: `-1` — "already traced". |
| `set $rax = 0` | The lie. A function returns its result in `rax` and the program is about to test it. **Must come after `finish`** — set it while still inside `ptrace` and the real `-1` lands on top of yours. |
| `continue` | The check passes, the program unpacks its builder, reports that it is **not invoking** it, and then calls `ptrace` a second time — so the breakpoint you already set fires again. No second breakpoint to guess at. |
| `call (char *) reveal()` | `reveal` is a function the program never calls. You call it. The cast is needed because the binary has no debug info; gdb tells you so if you forget. |

## Finding `reveal` yourself

Everything is stripped except that one name, and the program tells you a builder
is loaded but not invoked:

```
pwndbg> info functions
Non-debugging symbols:
0x000000000040165d  reveal
```

## Why the register write is not optional

`reveal()` jumps into a section that ships **encrypted**. It is only plaintext
after the program has passed its own check and unpacked it. Call `reveal()` at the
first breakpoint, before the flip, and you are jumping into ciphertext — you get
nothing. `build.sh` asserts that.

So the order matters: defeat the check, let it unpack, *then* call the dead
function.

## Alternatives, all fine

- `search -t bytes Securinets{` after `call (char *) reveal()` — finds it in the
  static buffer. (`-t bytes`, not `-t string`: pwndbg appends a NUL and the flag
  continues past `Securinets`.)
- Call the builder by address instead of via `reveal`; the program prints where it
  loaded it.
- `break *<address of the cmp>` instead of `break ptrace` + `finish`, found with
  `objdump -d gate | grep -A4 'ptrace@plt>$'`.
- `set $rip` to skip the branch rather than change the value.

Nothing in this binary punishes experimenting.

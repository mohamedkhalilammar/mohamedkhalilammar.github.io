# Night Shift — design notes

The second rung, and the one that exists specifically to **take challenge 01's tool away**. If a
player solves this with `ltrace`, the challenge has failed.

## Mechanism

The comparison is a hand-written byte loop, not a libc call: each input byte is XORed with
`0x5A + 7i` and rotated left by `i & 7`, then compared to a 20-byte table in `.rodata`. No
`strcmp`, no `memcmp`, nothing on the PLT to hook. The player has to read the loop, invert it,
and run the table backwards. The recovered key then unseals the flag.

## Decisions that are load-bearing

- **The transform lives in exactly one place**, `tools/rota.py`. `build.sh` generates the shipped
  table with it and `solution/solve.py` inverts with it, so the challenge and its official solver
  cannot drift apart without the build failing. The C in `src/nightshift.c` mirrors it by hand —
  **change one, change both.**
- **`build.sh` asserts `ltrace` shows no comparison.** That is not a style check. If a refactor
  ever reintroduces a library compare, this challenge silently collapses into a duplicate of 01,
  and the assertion is the only thing that would notice.
- **The transform is per-index but not chained.** Byte `i` depends only on byte `i`, so a player
  who gets the rotation slightly wrong still sees most bytes come out as printable text and knows
  they are close. A chained/CBC-style transform would turn one wrong assumption into total
  garbage with no feedback — per `docs/CTF-DESIGN-GUIDELINES.md`, that is the difference between
  "challenging" and "frustrating".
- **The rotation amount is `i & 7`, not a constant.** A constant rotation is solvable by staring;
  a varying one requires actually writing the inverse, which is the skill being taught.

## Do not add

Chained state, a length that is not obvious from the table, or a second round. Each of those
raises difficulty without teaching anything new, and this is rung two of five.

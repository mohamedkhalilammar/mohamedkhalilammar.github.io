# Doorman — design notes

The on-ramp. A player who has never opened a binary before should solve this in ten minutes and
come away with one durable idea: **`strings` only shows what the author left lying around; the
running process knows more than the file does.**

## Mechanism

The badge code is sealed in `BADGE` under a fixed numeric seed and rebuilt into a stack buffer at
startup, then `strcmp`'d against what the player typed. `strings` shows the prompts and the
refusal but not the code. `ltrace -e strcmp ./doorman` prints both arguments of that comparison,
including the real one. Type it back in, and the flag is unsealed under the badge code itself.

## Decisions that are load-bearing

- **`strcmp` is deliberate and must not be replaced with a hand-rolled loop.** The traceable
  library call *is* the lesson here. Challenge 02 is the one that takes it away; if this one also
  hid it, the ladder would have no first rung. Do not "harden" this.
- **The binary stays dynamically linked.** Static linking would remove the PLT entry and `ltrace`
  would print nothing, silently converting the easiest challenge in the track into a
  disassembly exercise.
- **`GATE_SEED` is `volatile`.** Without it, `-O2` is free to constant-fold the unwrap of a
  const array under a const seed and store the plaintext badge code in `.rodata` — the exact leak
  the challenge exists to avoid. `-O1` plus `volatile` is belt and braces; `build.sh` asserts the
  outcome rather than trusting the flags.

## Accepted residual

A player can read the unwrap in Ghidra and reimplement it instead of tracing. That is real code
comprehension and a perfectly good solve — the same residual accepted for the Android track's
#6b and #11. It is not a shortcut worth closing.

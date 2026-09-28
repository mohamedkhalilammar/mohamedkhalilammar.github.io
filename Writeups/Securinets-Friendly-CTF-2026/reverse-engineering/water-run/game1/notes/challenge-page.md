# Game 1 — player-facing page (draft)

Working title. Do not publish until the Cheat Engine solve path is confirmed on
`win11-malware-lab`.

## Brief

> You are carrying a six-pack of water through the medina, and half the city wants it.
>
> Run as far as you can. You will not get far enough.
>
> Download the game. When every lane closes, change the rules.

## What the player is given

- `WaterRun-windows-x64.zip` — the game.
- Nothing else. No source, no symbols file.

## The intended solve

Two separately compiled releases use the same wall and artifact:

- **Beginner:** the wall displays `JUMP POWER 9.25347`; find that exact native float and raise it.
- **Advanced:** no persistent jump-power field exists. Filter the changing native vertical
  velocity through ascent, descent and landing, then overwrite or freeze it upward. The field is
  deliberately **not** at the offset the beginner float sits at, so the beginner solve cannot be
  replayed as arithmetic — see `../../native/notes/design.md`.

The two releases have distinct native layouts and distinct flags. They are not runtime modes.

1. Reach **8,000** points. A wall closes all three lanes.
2. Use changing-score scans to locate the native state block, then inspect the nearby float
   holding the normal jump strength.
3. Raise that value enough to clear the wall.
4. Land beyond the wall and collect the artifact.
5. The game reveals the flag.

Normal movement cannot clear the wall. Passing it requires changing the live game state.

## What must be true, and must be re-checked before release

- **No anti-cheat of any kind.** No anti-debug, no integrity check, no scanner detection.
  Attaching a memory scanner *is* the intended solve. If a future pass proposes "make it more
  realistic" by adding protection, the answer is no — same standing rule as the Android track's
  ban on global Frida detection.
- The offline payload must never appear as plaintext in the executable, PCK, resources or
  tracked source. Its reconstruction stays behind native completion state.
- Jump strength must remain a native writable float. If it moves back into GDScript, the
  intended solve changes even though the game still runs.
- The advanced build's `vertical` must not sit at `score + 8`, which is where the beginner build
  keeps `lift`. Sharing that offset hands a beginner-build solver the advanced flag without the
  scan the challenge exists to teach.

## Hint ladder

Ordered to teach the memory-editing step without naming the exact value immediately.

1. "The wall is possible. Your current movement is not."
2. "The changing score is the easiest landmark in the native state. Find it first."
3. "Inspect the values beside the score. One nearby float controls how strongly you leave the ground."
4. "The normal jump strength is 9.25347. Raise it, clear the wall, and take what is beyond."

## Known ceiling, recorded rather than denied

An offline executable that can reconstruct a flag can ultimately be reversed or instrumented
to recover it. The native vault raises the cost of that shortcut; it cannot make offline
recovery impossible. The release criterion is that the intended Cheat Engine path is materially
easier than reconstructing the payload statically.

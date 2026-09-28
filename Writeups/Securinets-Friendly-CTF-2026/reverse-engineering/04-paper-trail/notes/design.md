# Paper Trail — design notes

The "this is not a native binary" lesson. A player who reflexively opens this in Ghidra will find
a CPython interpreter and 89 bundled files, and get nowhere until they recognise what they are
holding.

## Mechanism

An ordinary Python program packed with PyInstaller `--onefile`. The intended route is three
moves: recognise the bundle, extract it (`pyinstxtractor`), decompile the two `.pyc` files
(`uncompyle6` or `pycdc`). The recovered source hands over the passphrase derivation in plain
Python; running it gives the passphrase, and the passphrase unseals the flag.

## Decisions that are load-bearing

- **Python 3.8 is a hard pin, and it is the whole reason this challenge works.** uncompyle6 and
  pycdc decompile 3.8 bytecode cleanly. Built on the host's 3.14, every decompiler a beginner
  reaches for returns "version unsupported" and the challenge is unsolvable as designed. This is
  precisely the `docs/CTF-DESIGN-GUIDELINES.md` red flag about tasks whose research stage has no
  route available in the allotted time. **Do not bump the venv.**
- **`--noupx`.** UPX on top would add a second layer, but UPX-compressed PyInstaller bundles
  routinely confuse `pyinstxtractor` and the failure looks like a corrupt download, not a puzzle.
  Rung four of five is the wrong place to spend a player's afternoon on tooling.
- **The logic is split across two modules.** `payload.py` holds the two blobs, `paper_trail.py`
  holds the transform and the seed. It reads like software someone wrote rather than a crackme,
  and it means the player has to decompile both and put them together.
- **`build.sh` asserts the PyInstaller markers are still present** (`_MEIPASS`, `pyimod`, `PYZ`,
  `python3.8`). Note that the literal string "PyInstaller" is *not* in the binary — an earlier
  version of this check looked for it and failed the build for the wrong reason. `python3.8`
  staying visible is a genuine clue: it tells the player which decompiler to reach for.
- **`solution/solve.py` reimplements the unseal from the *decompiled* text**, not by importing
  the original module. A build that silently stopped producing decompilable bytecode would then
  fail here instead of passing on a technicality.

## Gotcha that cost real time

**The venv is not relocatable.** Renaming this directory left every console script's shebang
pointing at the old path; `uncompyle6` then failed with `FileNotFoundError` on a file that `ls`
shows exists, because the missing file is the *interpreter* in the shebang. Recreate the venv
after any move.

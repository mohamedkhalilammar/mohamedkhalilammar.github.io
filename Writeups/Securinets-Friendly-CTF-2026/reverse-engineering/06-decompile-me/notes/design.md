# Decompile Me — design notes

The sixth rung, and the only one that is not a Linux ELF. It exists because the ladder taught
five ways to read *machine code* and none of the ways to read *managed code* — and managed code
is where a beginner gets their first real win, because a .NET assembly decompiles back into C#
that reads like the source it was compiled from. The whole point of this challenge is that the
player installs dnSpy and ILSpy, opens the exe, and sees the program's own source staring back.

**No story.** The page says what the program is, what to install, and what to do — by the user's
instruction, and it applies to the program's own output too. The prompts are plain
(`Access code:`, `Wrong code. The form is KEY-XXXXXXXX.`), the name says the task out loud, and
the only flavour left is that the data it holds looks like real records instead of `foo`/`bar`.
That is the noise layer doing its job, not a narrative. **Do not put the fiction back.**

## Mechanism

The access code is not stored. `Authority.Expected()` builds it at run time: take the personnel
roster, keep the eight records at clearance 4, order them by hire year, and for each one index a
24-character wheel with `(Badge * 31 + HireYear) % 24`. Eight characters, prefixed `KEY-`. The flag is then sealed under that code — FNV-1a 32 for the seed, xorshift32 keystream,
the same primitive as the rest of the ladder — and `Vault.Open` unseals it in a byte array at
run time when the typed code matches.

So the program knows the answer and computes it in front of the player. That is the design, not
a leak: it is what makes both intended paths legitimate.

## Two paths, on purpose

- **Debugger (dnSpy).** Set a breakpoint on the comparison in `Main`, or inside `Expected()`,
  run, and read the value out of the locals window. Thirty seconds once you know the tool.
- **Static (ILSpy).** Read the derivation, filter and sort 24 records by hand or in a script,
  do eight multiplications mod 24. Ten minutes, no debugger, entirely fair.

`docs/CTF-DESIGN-GUIDELINES.md` is explicit that an Effect/Method task with more than one method
is a feature. The arithmetic is deliberately sized to sit right at the line where a beginner
starts wondering whether the machine could just tell them — which is the moment the debugger
becomes worth learning, and the reason this is the rung where it gets taught.

## Decisions that are load-bearing

- **.NET Framework 4.8, not .NET 8.** A net48 exe is a plain managed PE: dnSpy opens it, debugs
  it and can re-save it, and it runs on any Windows with nothing installed because 4.8 ships
  with the OS. Modern .NET would mean a runtime download for the player and rougher dnSpy
  support, and a single-file publish would bury the IL in a bundle that beginners cannot open at
  all. **Do not "modernise" this target.**
- **No PDB is shipped, and `build.sh` fails if one appears.** Symbols would hand over the
  original local-variable names and the source paths — dnSpy is quite capable of showing
  readable C# without them, and having to work from decompiler-generated names is part of what
  the player is learning.
- **The badge numbers are generated to hit the code, not picked by hand.** `build.sh` solves
  `(badge * 31 + year) ≡ wanted (mod 24)` for each supervisor. The generator and
  `Authority.Expected()` mirror the same arithmetic — **change one, change both**, and the build
  fails loudly if the generated roster does not derive `src/code.txt` (whose prefix, not a literal in the script, is what the build validates against).
- **Supervisor hire years are forced distinct.** The ordering is `OrderBy(HireYear)`, so a tie
  would make the code depend on `ThenBy` tie-breaking that a player reading the listing cannot
  see the effect of. The generator asserts distinctness rather than relying on luck.
- **The wheel omits B, I, O, S, Z, 0, 1, 5, 8.** A player who reads the code off a debugger
  window or a screenshot must not be able to typo their way into a wrong answer they cannot
  distinguish from a wrong derivation.
- **The solver goes through `ilspycmd`, the player's own decompiler** — not a private metadata
  parser. Everything it uses (roster, prefix, wheel, clearance level, multiplier) is regexed out
  of the decompilation of the shipped exe, so a green `build.sh` proves the packaged artifact is
  solvable, not that the build agrees with itself. The regexes are strict on purpose: refactor
  the C# so a constant stops appearing and the build breaks instead of drifting.
- **`Ledger` is real, boring, unrelated work** — straight-line depreciation over eight assets,
  printed on startup. It is there so the interesting 2% is not the only thing in the file.
  Non-negotiable #4. It does not participate in the derivation and never should, and it is data,
  not a plot.
- **Designer mode is a compile-time `DESIGNER` constant**, built to `build/designer/` every run.
  `build.sh` asserts the designer build prints the code and that the strings `designer` and
  `DESIGNER` do not survive into the shipped exe.
- **The notice is an embedded resource, not a string constant.** A managed assembly keeps its
  user strings as UTF-16, where plain `strings` misses them; an embedded resource is stored raw,
  so `strings` finds the notice immediately, ILSpy shows it under the Resources node, and
  `AssetRelease.exe /notice` prints it. That last one also gives the player a reason to look at
  the resources tree, which is where real .NET challenges hide things.

## Honest limits

- **The exe contains everything needed to solve it, offline.** Like rungs 01–04, and unlike the
  Android and game tracks, there is no server in the loop: a capable agent with the file can
  recover the code. That is a property of a self-contained beginner RE task, accepted
  deliberately. `NOTICE.txt` is the only mitigation and it is a speed bump, not a control.
- **Not yet played on real Windows with real dnSpy.** Everything here was built and verified on
  Linux, running the exe under wine. The claims that matter to a player — dnSpy loads it, a
  breakpoint in decompiled code hits, the locals window shows the code — have not been observed
  on `win11-malware-lab`. That pass is mandatory before it ships, and `build.sh` says so.

## Do not add

Obfuscation (ConfuserEx, control-flow flattening, string encryption), an anti-debug check, or a
packer. Every one of those breaks the *intended* path rather than a shortcut — the lesson here is
"managed code decompiles to readable C# and you can stop it mid-run", and there is nothing left to
teach if the player cannot read it or cannot attach. Also no second assembly, no reflection, and
no more arithmetic: rung 06 of six is not the place to raise the ceiling.

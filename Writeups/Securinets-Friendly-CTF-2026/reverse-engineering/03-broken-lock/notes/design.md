# Broken Lock — design notes

The patching lesson. The point is not "defeat a check" — it is **recognising that a check cannot
be satisfied, and choosing to change the program instead of the input.**

## Mechanism

`licensed()` looks for `/etc/meridian/service.lic`, requires exactly 16 bytes, and requires their
FNV-1a digest to equal a stored constant. That is a preimage problem, not a puzzle: there is no
licence a player can write. The intended move is to find the function, make it return 1, and run
it. `solution/patch.py` does it as six bytes — `mov eax,1 ; ret` — over the function prologue;
`gdb` or flipping the branch in `main` work just as well.

## Decisions that are load-bearing

- **The gate and the payload are deliberately split**, the same way the Android track's #11 is
  split. The check is trivially patchable; the flag is unsealed only once the success path
  actually runs. Nobody has to reverse the unseal to win — they have to open the gate.
- **The failure messages escalate**: no licence → wrong length → seal does not verify. A player
  who creates the file learns the length requirement, and a player who pads it to 16 bytes learns
  the check is cryptographic. That ladder is what makes "this is unsatisfiable, patch it instead"
  a conclusion a beginner can *reach* rather than guess.
- **`build.sh` asserts `/etc/meridian/service.lic` does not exist on the build host.** Otherwise
  the "the gate really is shut" test could pass for the wrong reason.
- **Two binaries, one compile.** `build/lock.dbg` keeps symbols for our tooling, `build/lock` is
  the same `.text` with the symbol table stripped by `objcopy`. `objcopy --strip-all` only drops
  trailing symbol/string tables, so file offsets into `.text` are identical — that is what lets
  the patcher locate a function in the stripped file by looking it up in the unstripped one.
  **Only `build/lock` ships.**

## Accepted residual

The unseal key is a constant in the binary, so a player who reverses `unwrap` can extract the
flag without ever patching. Accepted, and documented here rather than papered over: a key derived
from the code itself would break the moment the player patches the code — which is the one thing
this challenge requires them to do. Every alternative considered either broke the intended solve
or added machinery disproportionate to a 250-point beginner task.

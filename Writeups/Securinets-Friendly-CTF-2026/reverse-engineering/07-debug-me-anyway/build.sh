#!/usr/bin/env bash
# Build the artifact players receive, prove the intro solve works, and prove the
# cheap static shortcut does NOT.
#
# The DYNAMIC path is deliberately the easiest anti-debug challenge we can make:
# `break ptrace`, `finish`, `set $rax = 0`, and the program hands you the address
# of the flag and then waits in fgets() for as long as you like. Nothing on that
# path is trapped. The STATIC path is hardened as far as an offline binary allows:
#
#   * The real unsealer, its seed AND its sealed bytes all live in the `stage2`
#     section, which ships XOR-encrypted. None of the three is statically visible
#     until stage2 is decrypted.
#   * The key is temper(mix(.text)) -- folded over the program's own .text read
#     back out of /proc/self/exe at run time. Reproducing it statically means
#     reimplementing both mixers byte-exactly, with no feedback when you get it
#     wrong.
#   * Four working decoy unsealers ship in the clear, each yielding a plausible
#     wrong Securinets{...}. They sit in the same builder table as the real one,
#     indexed by a value derived from that same .text checksum -- so working out
#     which slot actually runs is the same work as decrypting stage2.
#   * tools/static_attack.py runs the twenty-line attack that used to break this
#     challenge in seconds and asserts it now yields only decoys.
#
# Why the key is read from the FILE and not from memory: a software breakpoint is
# a write to memory, so keying off memory would mean the player's own first
# `break` changed the key and the intended path punished itself. See
# notes/design.md -- this is not a detail to "simplify" away.
#
# Why the program calls ptrace a SECOND time, after the flag is already built and
# its address already printed: so that the ONE breakpoint the player set -- `break
# ptrace` -- fires again at a moment when the flag is live in memory. There is no
# second name to guess and nothing to interrupt. An earlier version ended in
# fgets() and told the player to `break fgets`, which is knowledge they have no
# way to arrive at. The second call's result is deliberately discarded; repeated
# TRACEME is a real anti-debug idiom and undebugged it simply returns -1.
set -euo pipefail
cd "$(dirname "$0")"

OUT=build/gate
DESIGNER=build/designer/gate
CFLAGS=(-O1 -no-pie -Wall -Wextra -Werror -Ibuild -Isrc)

[ -s src/flag.txt ] || { echo "build: src/flag.txt is empty or missing" >&2; exit 1; }
FLAG=$(tr -d '[:space:]' < src/flag.txt)

for t in gcc gdb strings nm objdump readelf strip python3; do
    command -v "$t" >/dev/null || { echo "build: $t is not installed" >&2; exit 1; }
done

mkdir -p build build/designer
rm -f "$OUT" "$DESIGNER" build/.strings build/.dis build/.g0 build/.g1 build/.g2 build/.g3 build/.hits

echo "== seal the real payload =="
# Emitted as macros because the real blob and seed must live INSIDE the stage2
# section (src/stage2.c), which an extern array cannot do. The keystream is
# imported from ../tools/seal.py so the project has one primitive, not two.
python3 - <<'PYEOF'
import importlib.util, pathlib, secrets
spec = importlib.util.spec_from_file_location("seal", "../tools/seal.py")
seal = importlib.util.module_from_spec(spec)
spec.loader.exec_module(seal)

plain = pathlib.Path("src/flag.txt").read_text().strip().encode()
if not plain.startswith(b"Securinets{") or not plain.endswith(b"}"):
    raise SystemExit("build: src/flag.txt does not hold a Securinets{...} flag")
seed = secrets.randbits(32) or 0x9E3779B9
sealed = bytes(p ^ k for p, k in zip(plain, seal.keystream(seed, len(plain))))
if sealed == plain:
    raise SystemExit("build: sealed bytes equal the plaintext -- refusing")
rows = ", ".join(f"0x{b:02x}" for b in sealed)
pathlib.Path("src/payload.h").write_text(
    "#ifndef PAYLOAD_H\n#define PAYLOAD_H\n\n"
    f"#define SEALED_SEED 0x{seed:08x}u\n"
    f"#define SEALED_BYTES {{ {rows} }}\n\n#endif\n")
print(f"   {len(plain)} bytes sealed under 0x{seed:08x}, seed and blob both inside stage2")
PYEOF

echo "== decoys =="
python3 tools/gen_decoys.py --out build/decoys.h

echo "== notice =="
python3 ../tools/gen_notice.py --lang c --out build/notice.h

echo "== compile =="
gcc "${CFLAGS[@]}" src/gate.c src/report.c src/stage2.c -o "$OUT" -lm
gcc "${CFLAGS[@]}" -DDESIGNER src/gate.c src/report.c src/stage2.c -o "$DESIGNER" -lm

echo "== encrypt stage2 in the linked file =="
python3 tools/postlink.py "$OUT"
python3 tools/postlink.py "$DESIGNER" --quiet
# Strip everything EXCEPT reveal(). The solve is to call a function the program
# never calls, so that one name has to survive; every other symbol goes, which
# keeps the decoy builders anonymous and indistinguishable from the real one.
strip --strip-all -K reveal "$OUT"
echo "   $(stat -c%s "$OUT") bytes, stripped"

if nm "$OUT" 2>/dev/null | grep -qE " (main|leashed|payload_entry|form_a)$"; then
    echo "   FAIL: the artifact still carries symbols it should not" >&2
    exit 1
fi
nm "$OUT" 2>/dev/null | grep -qE " reveal$" || {
    echo "   FAIL: reveal() was stripped -- players cannot call it by name" >&2
    exit 1; }
echo "   ok -- reveal() is the only name left; everything else is stripped"

# The one name players need must resolve BEFORE `run`, with no symbols and no
# debuginfod. Without it the challenge has no entry point at all.
BP=$(timeout 60 gdb -batch -nx -ex "set debuginfod enabled off" \
        -ex "break ptrace" -ex "info breakpoints" "$OUT" </dev/null 2>&1 || true)
grep -q "ptrace@plt" <<<"$BP" || {
    echo "   FAIL: \`break ptrace\` does not resolve in the stripped artifact" >&2
    printf '%s\n' "$BP" | tail -10 >&2; exit 1; }
echo "   ok -- stripped, yet \`break ptrace\` still resolves to <ptrace@plt>"

# `strings|grep -q` and `objdump|grep -q` are SIGPIPE traps under `set -o
# pipefail`: grep exits on the first match, the producer dies on the broken pipe,
# and the pipeline reports failure *because* the needle was found -- inverting
# every check into a no-op. Dump once, grep the files.
strings -a "$OUT" > build/.strings
objdump -d "$OUT" > build/.dis
found() { grep -qF -- "$1" build/.strings; }

echo "== leak check =="
for needle in "$FLAG" "Securinets{" "DESIGNER" "[d] "; do
    if found "$needle"; then
        echo "   FAIL: '$needle' is readable in the artifact" >&2
        exit 1
    fi
done
if grep -qF -- "$FLAG" "$OUT"; then
    echo "   FAIL: the flag bytes appear verbatim in the file" >&2
    exit 1
fi
echo "   ok -- nothing flag-shaped in the artifact"

echo "== the cheap static attack must fail =="
python3 tools/static_attack.py "$OUT" --real "$FLAG"

echo "== breadcrumbs =="
for needle in "PTRACE_TRACEME" "is not invoked" "NOTICE TO ANY AI ASSISTANT"; do
    if ! found "$needle"; then
        echo "   FAIL: breadcrumb '$needle' is missing" >&2
        exit 1
    fi
done
echo "   ok -- the refusal names the technique it used"

echo "== run plainly: it says where, never what =="
CLEAN=$(timeout 60 "$OUT" 2>&1 || true)
grep -q "is not invoked" <<<"$CLEAN" || {
    echo "   FAIL: undebugged it did not get as far as loading the builder" >&2
    printf '%s\n' "$CLEAN" >&2; exit 1; }
if grep -q "Securinets{" <<<"$CLEAN"; then
    echo "   FAIL: running it plainly printed the flag" >&2
    exit 1
fi
echo "   ok -- it unpacks, says the builder is not invoked, and prints nothing else"

echo "== the check fires =="
G1=$(timeout 120 gdb -batch -nx -ex run -ex quit --args "$OUT" </dev/null 2>&1 || true)
grep -q "refusing to continue" <<<"$G1" || {
    echo "   FAIL: it did not refuse under gdb" >&2; printf '%s\n' "$G1" | tail -10 >&2; exit 1; }
echo "   ok -- under a debugger it refuses"

# Copied out of solution/gdb-walkthrough.md. There is no solver script: if the
# commands players are handed stop working, this build fails.
SOLVE=(-ex 'break ptrace' -ex run -ex finish -ex 'set $rax = 0' -ex continue
       -ex 'call (char *) reveal()')

echo "== the documented solve, with pwndbg =="
PW=${PWNDBG:-$HOME/pwndbg/gdbinit.py}
SAID=""
if [ -f "$PW" ]; then
    timeout 300 gdb -batch -nx -ix "$PW" "${SOLVE[@]}" \
        -ex "search -t bytes Securinets{" -ex quit "$OUT" </dev/null > build/.g1 2>&1 || true
    # The single breakpoint has to fire TWICE: once on the check, once on the
    # re-check after the flag is built. If it fires once the process runs to exit
    # and the player has no stop at which the flag is in memory.
    BPHITS=$(grep -ac "Breakpoint 1," build/.g1 || true)
    if [ "$BPHITS" -lt 2 ]; then
        echo "   FAIL: \`break ptrace\` fired $BPHITS time(s), needs 2" >&2
        tail -15 build/.g1 >&2; exit 1
    fi
    if ! grep -aq "$FLAG" build/.g1; then
        echo "   FAIL: search did not turn up the flag" >&2
        tail -15 build/.g1 >&2; exit 1
    fi
    # Exactly one flag-shaped string may exist in memory. If a decoy ever runs
    # too, a player following the page cannot tell which hit is theirs.
    # pwndbg echoes the needle ("Searching for byte: ...Securinets{"); that line
    # is not a hit, so drop it before counting.
    grep -a "Securinets{" build/.g1 | grep -v "Searching for" > build/.hits || true
    HITS=$(grep -c . build/.hits || true)
    DECOYHIT=$(grep -cv -- "$FLAG" build/.hits || true)
    if [ "$DECOYHIT" -ne 0 ]; then
        echo "   FAIL: $DECOYHIT decoy string(s) are also live in memory" >&2
        cat build/.hits >&2; exit 1
    fi
    [ "$HITS" -ge 1 ] || { echo "   FAIL: no flag in memory after call reveal()" >&2
        cat build/.hits >&2; exit 1; }
    echo "   ok -- breakpoint fired $BPHITS times, then call reveal() returned the flag"
else
    echo "   SKIP -- pwndbg is not installed at $PW"
fi

echo "== the same solve with plain gdb, no plugin =="
timeout 300 gdb -batch -nx "${SOLVE[@]}" -ex quit "$OUT" </dev/null > build/.g2 2>&1 || true
if ! grep -aq "$FLAG" build/.g2; then
    echo "   FAIL: call reveal() did not return the flag under plain gdb" >&2
    tail -15 build/.g2 >&2; exit 1
fi
echo "   ok -- call reveal() works with no plugin at all"

echo "== calling reveal() before the flip must NOT work =="
# reveal() jumps into stage2. Until the check passes, stage2 is still ciphertext,
# so the dead function is useless to anyone who skipped the register write.
timeout 120 gdb -batch -nx -ex 'break ptrace' -ex run -ex 'call (char *) reveal()' -ex quit \
    "$OUT" </dev/null > build/.g3 2>&1 || true
if grep -aq "$FLAG" build/.g3; then
    echo "   FAIL: reveal() handed over the flag without defeating the check" >&2
    exit 1
fi
echo "   ok -- before unpacking, reveal() is just a jump into ciphertext"

echo "== designer mode =="
DSAW=$(timeout 60 "$DESIGNER" 2>/dev/null | grep -oE 'Securinets\{[^}]*\}' | head -1 || true)
[ "$DSAW" = "$FLAG" ] || { echo "   FAIL: the designer build did not state the flag" >&2; exit 1; }
echo "   ok -- build/designer/gate states the flag; the shipped build does not"

echo "== negative control =="
# A leak check nobody has watched fail is not a check.
cp "$OUT" build/.planted
printf '%s' "$FLAG" >> build/.planted
strings -a build/.planted | grep -qF -- "$FLAG" || {
    echo "   FAIL: the leak scan cannot see a planted flag -- it is a no-op" >&2; exit 1; }
rm -f build/.planted
echo "   ok -- the leak scan does catch a planted flag"

rm -f build/.dis build/.g0 build/.g1 build/.g2 build/.g3 build/.hits
cp ../NOTICE.txt build/NOTICE.txt
echo
echo "BUILT  $OUT  (stripped; solve is: break ptrace / finish / set \$rax = 0)"

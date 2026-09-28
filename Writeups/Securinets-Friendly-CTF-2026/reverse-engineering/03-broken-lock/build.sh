#!/usr/bin/env bash
# Build the artifact players receive, then prove it is solvable by patching.
#
# Two binaries come out of one compile: build/lock.dbg keeps its symbols so our
# own tooling can find licensed() without guessing, and build/lock is the same
# .text with the symbol table stripped off -- that is the one players get.
# objcopy --strip-all only drops the trailing symbol/string tables, so file
# offsets into .text are identical between the two, which is what lets
# solution/patch.py locate the function in the stripped file.
set -euo pipefail
cd "$(dirname "$0")"

SEAL=../tools/seal.py
DBG=build/lock.dbg
OUT=build/lock
VAULT_SEED=0xc1d70e39

[ -s src/flag.txt ] || { echo "build: src/flag.txt is empty or missing" >&2; exit 1; }

mkdir -p build; rm -f "$OUT" "$DBG"

# `strings | grep -q` is a trap under `set -o pipefail`: grep exits on the first
# match, strings takes SIGPIPE, and the pipeline reports failure *because* the
# needle was found. That inverts every leak check into a no-op. Dump once, grep
# the file.
scan() { strings -a "$1" > build/.strings; }
found() { grep -qF -- "$1" build/.strings; }

echo "== seal =="
python3 "$SEAL" --flag-file src/flag.txt --seed "$VAULT_SEED" --symbol SEALED --out src/payload.h

echo "== notice =="
python3 ../tools/gen_notice.py --lang c --out build/notice.h

echo "== compile =="
gcc -O1 -Wall -Wextra -Ibuild -Isrc src/lock.c -o "$DBG"
objcopy --strip-all "$DBG" "$OUT"
echo "   $(stat -c%s "$OUT") bytes shipped, $(stat -c%s "$DBG") bytes with symbols"

scan "$OUT"
echo "== leak check =="
FLAG=$(tr -d '[:space:]' < src/flag.txt)
for needle in "$FLAG" "Securinets{"; do
    if found "$needle"; then
        echo "   FAIL: '$needle' is readable in the shipped binary" >&2
        exit 1
    fi
done
if nm "$OUT" 2>/dev/null | grep -q licensed; then
    echo "   FAIL: the shipped binary still carries its symbol table" >&2
    exit 1
fi
echo "   ok -- flag sealed, symbols stripped"

echo "== the gate really is shut =="
if "$OUT" | grep -q "Securinets{"; then
    echo "   FAIL: the unpatched binary unlocks on its own" >&2
    exit 1
fi
echo "   ok -- unpatched, it refuses (no licence on this machine)"

echo "== the gate is unsatisfiable by input =="
# If a player could simply forge the licence file, patching would not be the
# lesson. The seal check is a 32-bit FNV preimage, so it is not forgeable --
# but a stray /etc/meridian/service.lic on the build host would hide that.
if [ -e /etc/meridian/service.lic ]; then
    echo "   FAIL: this host has a real licence file -- the build cannot trust its own test" >&2
    exit 1
fi
echo "   ok -- no licence exists to accidentally satisfy the check"

echo "== intended solve =="
PATCHED=build/lock.patched
python3 solution/patch.py --symbols "$DBG" --binary "$OUT" --out "$PATCHED"
SEEN=$("$PATCHED" | tail -1)
if [ "$SEEN" != "$FLAG" ]; then
    echo "   FAIL: patching licensed() did not yield the flag" >&2
    echo "   got: $SEEN" >&2
    exit 1
fi
echo "   ok -- six patched bytes open the gate and the flag comes back"
rm -f "$PATCHED"

cp ../NOTICE.txt build/NOTICE.txt
echo
echo "REMINDER  NOTICE.txt is attached to this challenge on CTFd alongside the binary."
echo "          It is embedded in the artifact too, but on the PyInstaller bundle it"
echo "          only surfaces after extraction -- the companion file is what guarantees"
echo "          an assistant sees it before it starts work."

echo
echo "BUILT  $OUT"

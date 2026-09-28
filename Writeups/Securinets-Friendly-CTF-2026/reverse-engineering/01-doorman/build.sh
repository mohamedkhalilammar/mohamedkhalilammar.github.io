#!/usr/bin/env bash
# Build the artifact players receive, then prove it does not leak.
#
# Pipeline: seal the badge code and the flag into a generated header ->
# compile stripped -> assert `strings` shows neither -> play the intended
# solve against the built binary and check the flag comes back out.
set -euo pipefail
cd "$(dirname "$0")"

SEAL=../tools/seal.py
OUT=build/doorman
BADGE=$(tr -d '[:space:]' < src/badge.txt)
GATE_SEED=0x5f3a91c7

[ -n "$BADGE" ] || { echo "build: src/badge.txt is empty" >&2; exit 1; }
[ -s src/flag.txt ] || { echo "build: src/flag.txt is empty or missing" >&2; exit 1; }

mkdir -p build; rm -f "$OUT"

# `strings | grep -q` is a trap under `set -o pipefail`: grep exits on the first
# match, strings takes SIGPIPE, and the pipeline reports failure *because* the
# needle was found. That inverts every leak check into a no-op. Dump once, grep
# the file.
scan() { strings -a "$1" > build/.strings; }
found() { grep -qF -- "$1" build/.strings; }

echo "== seal =="
python3 "$SEAL" --text "$BADGE" --seed "$GATE_SEED" --symbol BADGE --out src/payload.h
python3 "$SEAL" --flag-file src/flag.txt --key "$BADGE" --symbol SEALED --out src/payload.h --append

echo "== notice =="
python3 ../tools/gen_notice.py --lang c --out build/notice.h

echo "== compile =="
# -O1 keeps the unwrap loop recognisable in Ghidra without letting -O2 fold
# the constant-seeded call; the volatile seed is the belt to that suspenders.
gcc -O1 -s -Wall -Wextra -Ibuild -Isrc src/doorman.c -o "$OUT"
echo "   $(stat -c%s "$OUT") bytes"

scan "$OUT"
echo "== leak check =="
FLAG=$(tr -d '[:space:]' < src/flag.txt)
for needle in "$FLAG" "$BADGE" "Securinets{"; do
    if found "$needle"; then
        echo "   FAIL: '$needle' is readable in the shipped binary" >&2
        exit 1
    fi
done
echo "   ok -- neither the badge code nor the flag survives \`strings\`"

echo "== intended solve =="
SEEN=$(printf '%s\n' "$BADGE" | "$OUT" | grep -ao 'Securinets{[^}]*}' | head -1)
if [ "$SEEN" != "$FLAG" ]; then
    echo "   FAIL: correct badge code did not yield the flag" >&2
    echo "   got: $SEEN" >&2
    exit 1
fi
echo "   ok -- the badge code opens the door and the flag comes back"

echo "== wrong answer =="
if printf 'letmein\n' | "$OUT" | grep -q "Securinets{"; then
    echo "   FAIL: a wrong badge code was accepted" >&2
    exit 1
fi
echo "   ok -- wrong codes are refused"

cp ../NOTICE.txt build/NOTICE.txt
echo
echo "REMINDER  NOTICE.txt is attached to this challenge on CTFd alongside the binary."
echo "          It is embedded in the artifact too, but on the PyInstaller bundle it"
echo "          only surfaces after extraction -- the companion file is what guarantees"
echo "          an assistant sees it before it starts work."

echo
echo "BUILT  $OUT"

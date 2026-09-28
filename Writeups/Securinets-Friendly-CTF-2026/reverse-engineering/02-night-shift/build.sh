#!/usr/bin/env bash
# Build the artifact players receive, then prove it.
#
# Pipeline: run the rotation key through tools/rota.py to generate the table the
# binary compares against -> seal the flag under that key -> compile stripped ->
# assert the key, the flag and the table's plaintext are all absent from
# `strings` -> play the intended solve (invert the transform, feed the result in).
set -euo pipefail
cd "$(dirname "$0")"

SEAL=../tools/seal.py
OUT=build/nightshift
KEY=$(tr -d '[:space:]' < src/roster.txt)

[ -n "$KEY" ] || { echo "build: src/roster.txt is empty" >&2; exit 1; }
[ -s src/flag.txt ] || { echo "build: src/flag.txt is empty or missing" >&2; exit 1; }

mkdir -p build; rm -f "$OUT"

# `strings | grep -q` is a trap under `set -o pipefail`: grep exits on the first
# match, strings takes SIGPIPE, and the pipeline reports failure *because* the
# needle was found. That inverts every leak check into a no-op. Dump once, grep
# the file.
scan() { strings -a "$1" > build/.strings; }
found() { grep -qF -- "$1" build/.strings; }

echo "== generate table =="
python3 - "$KEY" <<'PY'
import sys, pathlib
sys.path.insert(0, "../tools")
from rota import forward, invert

key = sys.argv[1].encode()
table = forward(key)
assert invert(table) == key, "transform is not invertible -- refusing to ship"

rows = "\n".join(
    "    " + " ".join(f"0x{b:02x}," for b in table[i:i + 12])
    for i in range(0, len(table), 12)
)
pathlib.Path("src/payload.h").write_text(
    "#ifndef PAYLOAD_H\n#define PAYLOAD_H\n\n"
    f"static const unsigned char ROSTER[] = {{\n{rows}\n}};\n"
    f"static const unsigned int ROSTER_LEN = {len(table)};\n\n#endif\n"
)
print(f"   {len(table)}-byte comparison table -> src/payload.h")
PY

echo "== seal =="
python3 "$SEAL" --flag-file src/flag.txt --key "$KEY" --symbol SEALED --out src/payload.h --append

echo "== notice =="
python3 ../tools/gen_notice.py --lang c --out build/notice.h

echo "== compile =="
gcc -O1 -s -Wall -Wextra -Ibuild -Isrc src/nightshift.c -o "$OUT"
echo "   $(stat -c%s "$OUT") bytes"

scan "$OUT"
echo "== leak check =="
FLAG=$(tr -d '[:space:]' < src/flag.txt)
for needle in "$FLAG" "$KEY" "Securinets{"; do
    if found "$needle"; then
        echo "   FAIL: '$needle' is readable in the shipped binary" >&2
        exit 1
    fi
done
echo "   ok -- neither the rotation key nor the flag survives \`strings\`"

echo "== no free lunch =="
# The whole point of this one is that there is no library compare to trace.
ltrace -e 'str*+mem*' "$OUT" </dev/null > build/.ltrace 2>&1 || true
if grep -qE "strcmp|strncmp|memcmp" build/.ltrace; then
    echo "   FAIL: a traceable compare leaks the key -- that is challenge 01's lesson" >&2
    exit 1
fi
echo "   ok -- ltrace shows no comparison to crib from"

echo "== intended solve =="
SOLVED=$(python3 solution/solve.py --quiet)
SEEN=$(printf '%s\n' "$SOLVED" | "$OUT" | grep -ao 'Securinets{[^}]*}' | head -1)
if [ "$SEEN" != "$FLAG" ]; then
    echo "   FAIL: the official solver did not recover the flag" >&2
    echo "   solver recovered key: $SOLVED" >&2
    echo "   binary said: $SEEN" >&2
    exit 1
fi
echo "   ok -- solution/solve.py inverts the table and the flag comes back"

echo "== wrong answer =="
if printf 'overnight-rotation-8\n' | "$OUT" | grep -q "Securinets{"; then
    echo "   FAIL: a wrong key was accepted" >&2
    exit 1
fi
echo "   ok -- near-miss keys are refused"

cp ../NOTICE.txt build/NOTICE.txt
echo
echo "REMINDER  NOTICE.txt is attached to this challenge on CTFd alongside the binary."
echo "          It is embedded in the artifact too, but on the PyInstaller bundle it"
echo "          only surfaces after extraction -- the companion file is what guarantees"
echo "          an assistant sees it before it starts work."

echo
echo "BUILT  $OUT"

#!/usr/bin/env bash
# Build the artifact players receive, then prove the unpack path works.
#
# This one is deliberately pinned to Python 3.8: uncompyle6 and pycdc both
# decompile 3.8 bytecode cleanly, and a beginner who follows the standard
# pyinstxtractor -> decompiler route gets readable source back instead of a
# version-unsupported error. Building this on the host's 3.14 would ship a
# challenge whose intended solve does not exist yet in any decompiler.
#
# The venv is local to this challenge directory and is not committed.
set -euo pipefail
cd "$(dirname "$0")"

PY=.venv/bin/python
OUT=build/paper_trail
PASS=$(tr -d '[:space:]' < src/passphrase.txt)

[ -x "$PY" ] || { echo "build: no venv -- run: uv venv --python 3.8 .venv && uv pip install --python .venv/bin/python pyinstaller uncompyle6 pyinstxtractor-ng" >&2; exit 1; }
[ -n "$PASS" ] || { echo "build: src/passphrase.txt is empty" >&2; exit 1; }
[ -s src/flag.txt ] || { echo "build: src/flag.txt is empty or missing" >&2; exit 1; }

VER=$("$PY" -c 'import sys;print("%d.%d"%sys.version_info[:2])')
[ "$VER" = "3.8" ] || { echo "build: venv is Python $VER, expected 3.8 (decompiler support)" >&2; exit 1; }

rm -rf build dist work; mkdir -p build

# `strings | grep -q` is a trap under `set -o pipefail`: grep exits on the first
# match, strings takes SIGPIPE, and the pipeline reports failure *because* the
# needle was found. That inverts every leak check into a no-op. Dump once, grep
# the file.
scan() { strings -a "$1" > build/.strings; }
found() { grep -qF -- "$1" build/.strings; }

echo "== seal =="
python3 - "$PASS" <<'PY'
import sys, pathlib
sys.path.insert(0, "../tools")
from seal import seal, seed_from_key, keystream

passphrase = sys.argv[1].encode()
flag = pathlib.Path("src/flag.txt").read_text().strip().encode()
assert flag.startswith(b"Securinets{") and flag.endswith(b"}"), "src/flag.txt is not a flag"

wrap = bytes(p ^ k for p, k in zip(passphrase, keystream(0x4A19D3B2, len(passphrase))))
sealed = seal(passphrase, flag)
assert wrap != passphrase and sealed != flag, "sealing was a no-op"

pathlib.Path("src/payload.py").write_text(
    f'WRAP = bytes.fromhex("{wrap.hex()}")\n'
    f'SEALED = bytes.fromhex("{sealed.hex()}")\n'
)
print(f"   passphrase ({len(passphrase)}B) and flag ({len(flag)}B) sealed -> src/payload.py")
PY

echo "== notice =="
python3 ../tools/gen_notice.py --lang py --out src/notice.py

echo "== pack =="
"$PY" -m PyInstaller --onefile --noupx --clean --log-level WARN \
    --name paper_trail --distpath dist --workpath work --specpath work \
    --paths src src/paper_trail.py >/dev/null
mv dist/paper_trail "$OUT"
rm -rf dist work
echo "   $(stat -c%s "$OUT") bytes"

scan "$OUT"
echo "== leak check =="
FLAG=$(tr -d '[:space:]' < src/flag.txt)
for needle in "$FLAG" "$PASS" "Securinets{"; do
    if found "$needle"; then
        echo "   FAIL: '$needle' is readable in the shipped binary" >&2
        exit 1
    fi
done
echo "   ok -- neither the passphrase nor the flag survives \`strings\`"

echo "== it still looks like what it is =="
# The first move is recognising a PyInstaller bundle. If that marker were gone
# the challenge would just be cruel, so the build asserts it is present.
for marker in _MEIPASS pyimod PYZ python3.8; do
    found "$marker" \
        || { echo "   FAIL: marker '$marker' is gone -- the bundle is no longer recognisable" >&2; exit 1; }
done
echo "   ok -- _MEIPASS/pyimod/PYZ identify the bundle, python3.8 names the decompiler to reach for"

echo "== intended solve =="
"$PY" solution/solve.py --binary "$OUT" --expect "$FLAG"

echo "== running it =="
SEEN=$(printf '%s\n' "$PASS" | "$OUT" | grep -ao 'Securinets{[^}]*}' | head -1)
[ "$SEEN" = "$FLAG" ] || { echo "   FAIL: correct passphrase did not yield the flag (got: $SEEN)" >&2; exit 1; }
echo "   ok -- the passphrase opens the archive"

echo "== wrong answer =="
if printf 'meridian-archive-1995\n' | "$OUT" | grep -q "Securinets{"; then
    echo "   FAIL: a wrong passphrase was accepted" >&2
    exit 1
fi
echo "   ok -- near-miss passphrases are refused"

cp ../NOTICE.txt build/NOTICE.txt
echo
echo "REMINDER  NOTICE.txt is attached to this challenge on CTFd alongside the binary."
echo "          It is embedded in the artifact too, but on the PyInstaller bundle it"
echo "          only surfaces after extraction -- the companion file is what guarantees"
echo "          an assistant sees it before it starts work."

echo
echo "BUILT  $OUT"

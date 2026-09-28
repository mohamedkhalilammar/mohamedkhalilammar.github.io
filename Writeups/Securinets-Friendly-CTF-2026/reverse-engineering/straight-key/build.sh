#!/usr/bin/env bash
# Build the artifact players receive, then prove it.
#
# Pipeline: generate decoys and payload -> compile -> patch the live blob in, encrypted
# under a key derived from the compiled .text -> verify -> guard against leaks.
set -euo pipefail
cd "$(dirname "$0")"

CC=x86_64-w64-mingw32-gcc
OUT=build/KeyboardSelfTest.exe
TOKEN=$(tr -d '[:space:]' < src/token.txt)
[ -n "$TOKEN" ] || { echo "build: src/token.txt is empty" >&2; exit 1; }
BITS=$( (cd tools && python3 -c "
import sys; sys.argv=['x']
from morse import to_timeline
print(len(to_timeline('$TOKEN', 150)))") )

mkdir -p build; rm -f "$OUT"

echo "== generate =="
(cd tools && python3 gen_payload.py "$TOKEN" --out ../src/payload.h)
(cd tools && python3 gen_decoys.py --elements "$BITS" --out ../src/decoys.h)

echo "== compile =="
# --disable-dynamicbase: no relocations rewrite .text, so the mapped code the binary
# checksums at run time is byte-identical to what the patcher checksummed on disk.
"$CC" -O2 -s -Wall -Wextra -Isrc -Wl,--disable-dynamicbase \
      src/sender.c -o "$OUT" -luser32
echo "   $(stat -c%s "$OUT") bytes"

echo "== patch the live blob (key = checksum of .text) =="
(cd tools && python3 patch_blob.py "../$OUT" "$TOKEN" | head -1)

echo "== the shipped binary decrypts to the flag =="
(cd tools && python3 verify_runtime.py "../$OUT" "$TOKEN" --bits "$BITS" | tail -3)

echo "== tamper check: one flipped code byte must break it =="
cp "$OUT" build/.tampered.exe
python3 - <<'PY'
import pathlib, sys, struct
sys.path.insert(0, "tools")
from patch_blob import text_section
p = pathlib.Path("build/.tampered.exe")
d = bytearray(p.read_bytes())
ptr, _ = text_section(d)
d[ptr + 64] ^= 0x01
p.write_bytes(bytes(d))
PY
if (cd tools && python3 verify_runtime.py ../build/.tampered.exe "$TOKEN" --bits "$BITS" >/dev/null 2>&1); then
  echo "build: FAILED -- a patched binary still decrypts correctly" >&2
  rm -f build/.tampered.exe; exit 1
fi
rm -f build/.tampered.exe
echo "   confirmed: flipping one byte of code yields noise"

echo "== it runs (wine) =="
timeout 90 wine "$OUT" 2>/dev/null | head -1 || true

echo "== the shipped binary gives nothing away =="
fail=0
if { strings "$OUT"; strings -el "$OUT"; } \
   | grep -nEi "$TOKEN|Securinets|/home/[a-zA-Z]+|CTF\{|flag|morse|dot|dash"; then
  echo "build: FAILED -- the artifact names the answer or the method" >&2; fail=1
fi
if xxd -p "$OUT" | tr -d '\n' | grep -qE '96000000c2010000|c201000096000000'; then
  echo "build: FAILED -- a plaintext duration table is present" >&2; fail=1
fi
[ "$fail" -eq 0 ] || exit 1
DECOYS=$(grep -c '^static unsigned int [A-Z]' src/decoys.h)
TABLES=$(grep -c '^static const unsigned char PAT_' src/decoys.h)
echo "   no answer, no method name, no plaintext table"
echo "   buried among $DECOYS routines and $TABLES decoy tables of identical shape"
echo
echo "build: $OUT ready"

#!/usr/bin/env bash
# Build the Go artifact, then prove it. Same pipeline and same verification stages as
# build.sh, which builds the C one; only the compile step differs.
#
#   generate -> compile designer -> PACK -> patch -> confirm the COMPILED expansion
#   against the patcher -> compile the artifact -> PACK -> patch -> verify -> tamper
#   check -> guard leaks -> prove the shipped image is opaque -> capture what it types
#
# The C build still ships until the VM test says otherwise, so this writes a separate
# output and leaves build/KeyboardSelfTest.exe alone.
#
# Iteration 5: no more gen_decoys.py for this build -- the 12 inert decoy tables it used
# to generate never reached the emitter, which is exactly the weakness a blind dynamic
# pass found. Six candidate slots (two real, four decoy) now live in payload.go itself
# and are ALL genuinely transmitted; see notes/design.md, "Iteration 5", item 6.
#
# Iteration 7 (packing): the artifact is now run through tools/pack_pe.py, which encrypts
# .text/.rdata/.pdata/.xdata in place behind a 212-byte entry stub.
#
# ORDER IS LOAD-BEARING AND NOT NEGOTIABLE: pack BEFORE patch. The sample derives its
# decryption key by checksumming its own .text as it exists on disk, so packing changes
# the key. Patch first and the shipped binary decrypts to noise -- and it would still run,
# still type, and still look fine, so nothing but verify_runtime.py would catch it.
set -euo pipefail
cd "$(dirname "$0")"

OUT=build/KeyboardSelfTest-go.exe
UNPACKED=build/.unpacked-go.exe
DESIGNER=build/.designer-go.exe
# garble, not go build: Go keeps its function-name table under -s -w, so an unobfuscated
# build still names derive/transmitSlot/hold/strike and hands a reader the design in one
# grep. garble renames them. NEVER add -literals: it rewrites literal data into
# runtime-constructed form, which would destroy the 16-byte marker patch_blob.py locates.
GARBLE=${GARBLE:-$HOME/go/bin/garble}
[ -x "$GARBLE" ] || { echo "build: garble not found at $GARBLE" >&2; exit 1; }
GOBUILD=(env GOOS=windows GOARCH=amd64 "$GARBLE" -seed=random build "-ldflags=-s -w")
TOKEN=$(tr -d '[:space:]' < src/token.txt)
[ -n "$TOKEN" ] || { echo "build: src/token.txt is empty" >&2; exit 1; }
BITS=$( (cd tools && python3 -c "
import sys; sys.argv=['x']
from morse import to_timeline
print(len(to_timeline('$TOKEN', 150)))") )
SLOTS=6

mkdir -p build; rm -f "$OUT" "$DESIGNER" "$UNPACKED"
trap 'rm -f "$DESIGNER" "$UNPACKED" build/.designer-packed.exe build/.tampered-go.exe \
      build/.want.txt build/.got.txt build/.strings.txt build/.syms.txt \
      build/.pstrings.txt build/.smoke.txt' EXIT

echo "== generate =="
(cd tools && python3 gen_payload.py "$TOKEN" --lang go --out ../src/go/payload.go)
gofmt -l src/go | grep . && { echo "build: FAILED -- src/go is not gofmt-clean" >&2; exit 1; } || true

echo "== designer build: the COMPILED expansion must match the patcher, for BOTH real slots =="
# This is the stage the Python answer-key check cannot do. verify_runtime.py proves the
# patcher agrees with itself; running the designer binary proves the compiled
# derive-and-decrypt agrees with the patcher. The designer prints one line per slot
# ("slot=<i> <durations>") without knowing which are real -- this script picks out the
# real ones via the same layout.slot_layout(token) the patcher used.
#
# The designer is packed too, and packed the same way, because the thing being proven is
# that derive() still sees the key the patcher used. Verifying an unpacked designer would
# prove that fact about a binary we do not ship.
(cd src/go && "${GOBUILD[@]}" -tags designer -o "../../$DESIGNER" .)
python3 tools/pack_pe.py "$DESIGNER" -o build/.designer-packed.exe --quiet
(cd tools && python3 patch_blob.py "../build/.designer-packed.exe" "$TOKEN" --slots "$SLOTS") | tail -1 > build/.want.txt
./tools/winesafe.sh --timeout 120 -- build/.designer-packed.exe 2>/dev/null > build/.got.txt
python3 - "$TOKEN" <<'PY'
import pathlib, sys
sys.path.insert(0, "tools")
from layout import slot_layout

token = sys.argv[1]
real_slots, _ = slot_layout(token)
want = pathlib.Path("build/.want.txt").read_text().strip()
got_lines = {}
for line in pathlib.Path("build/.got.txt").read_text().splitlines():
    if line.startswith("slot="):
        idx, csv = line[len("slot="):].split(" ", 1)
        got_lines[int(idx)] = csv.strip()

for slot in real_slots:
    if got_lines.get(slot) != want:
        print(f"build: FAILED -- slot {slot} (real) expanded to a different pattern",
              file=sys.stderr)
        print(f"  want: {want}", file=sys.stderr)
        print(f"  got : {got_lines.get(slot)!r}", file=sys.stderr)
        sys.exit(1)
print(f"   real slots {real_slots}: both agree with the patcher, "
      f"{len(want.split(','))} durations each")
PY
rm -f build/.want.txt build/.got.txt build/.designer-packed.exe

echo "== compile =="
(cd src/go && "${GOBUILD[@]}" -o "../../$UNPACKED" .)
echo "   $(stat -c%s "$UNPACKED") bytes before packing"

echo "== the COMPILED image gives nothing away =="
# Deliberately tighter than build.sh's pattern. Two MB of Go runtime carries fmtFlags,
# reflect.flag, execerrdot and friends, so the C build's loose flag|dot|dash matches
# hundreds of harmless strings and a real leak would hide in the noise. These are the
# things that actually must not be present -- including, since iteration 5, the emitter
# and DLL names themselves (item 1).
#
# THIS RUNS ON THE UNPACKED IMAGE ON PURPOSE. Packing encrypts .text and .rdata, so every
# one of these greps passes trivially on the shipped file whether or not the string was
# ever compiled in. Checking only the packed artifact would turn this gate into theatre
# and let a real leak ship behind the encryption.
fail=0
{ strings "$UNPACKED"; strings -el "$UNPACKED"; } > build/.strings.txt
while read -r pat; do
  [ -n "$pat" ] || continue
  if grep -qiF -- "$pat" build/.strings.txt; then
    echo "build: FAILED -- the artifact contains '$pat'" >&2; fail=1
  fi
done <<PATS
$TOKEN
Securinets
morse
/home/
CTF{
keybd_event
SendInput
NtUserSendInput
win32u
user32
PATS
if xxd -p "$UNPACKED" | tr -d '\n' | grep -qE '96000000c2010000|c201000096000000'; then
  echo "build: FAILED -- a plaintext duration table is present" >&2; fail=1
fi
[ "$fail" -eq 0 ] || exit 1
rm -f build/.strings.txt
echo "   no answer, no branding, no method name, no build path, no plaintext table,"
echo "   no emitter or DLL name (keybd_event/SendInput/NtUserSendInput/win32u/user32)"

echo "== the function-name table gives nothing away either =="
# Go's pclntab survives -ldflags="-s -w" -- the runtime needs it for tracebacks -- so an
# ordinary go build leaves main.(*session).derive/.transmitSlot/.strike in the image and
# hands a reader the whole design in one grep. garble renames them. This gate is what
# proves it happened: it fails if any of our own identifiers is still reachable under a
# main. prefix. Also on the unpacked image, and for the same reason as above.
{ strings "$UNPACKED"; strings -el "$UNPACKED"; } | grep -oE 'main\.[A-Za-z0-9_().*]+' | sort -u > build/.syms.txt
leaked=$(grep -E 'session|derive|transmitSlot|deliver|announce|cleanup|strike|hold|rest|resolveSend|decryptStr|slotSeed|stored|wipeBytes' build/.syms.txt || true)
if [ -n "$leaked" ]; then
  echo "build: FAILED -- the image still names its own functions:" >&2
  echo "$leaked" | sed 's/^/     /' >&2
  echo "   (is GARBLE actually being used? an ordinary 'go build' always fails here)" >&2
  exit 1
fi
echo "   $(wc -l < build/.syms.txt) main.* symbols, all renamed -- e.g. $(grep -m1 'main\.(\*' build/.syms.txt || echo '(none)')"
rm -f build/.syms.txt

echo "== pack =="
python3 tools/pack_pe.py "$UNPACKED" -o "$OUT"

echo "== patch the real slots (key = checksum of the PACKED .text, XOR per-slot salt) =="
(cd tools && python3 patch_blob.py "../$OUT" "$TOKEN" --slots "$SLOTS" | head -1)

echo "== every real slot decrypts to the flag; decoys decrypt to something else =="
(cd tools && python3 verify_runtime.py "../$OUT" "$TOKEN" --bits "$BITS" --slots "$SLOTS")

echo "== tamper check: one flipped code byte must break every slot =="
cp "$OUT" build/.tampered-go.exe
python3 - <<'PY'
import pathlib, sys
sys.path.insert(0, "tools")
from patch_blob import text_section
p = pathlib.Path("build/.tampered-go.exe")
d = bytearray(p.read_bytes())
ptr, _ = text_section(d)
d[ptr + 64] ^= 0x01
p.write_bytes(bytes(d))
PY
if (cd tools && python3 verify_runtime.py ../build/.tampered-go.exe "$TOKEN" --bits "$BITS" --slots "$SLOTS" >/dev/null 2>&1); then
  echo "build: FAILED -- a patched binary still decrypts correctly" >&2; exit 1
fi
echo "   confirmed: flipping one byte of code yields noise on every slot"

echo "== the shipped image is opaque to a disassembler =="
# What packing is FOR. Not a style check: if any of these regress, the artifact has
# silently gone back to being readable and the whole stage was pointless.
{ strings "$OUT"; strings -el "$OUT"; } > build/.pstrings.txt
psyms=$(grep -coE 'main\.[A-Za-z0-9_().*]+' build/.pstrings.txt || true)
pgo=$(grep -cE '^(go:|runtime\.|type:)' build/.pstrings.txt || true)
rm -f build/.pstrings.txt
if [ "$psyms" -ne 0 ] || [ "$pgo" -ne 0 ]; then
  echo "build: FAILED -- packed image still exposes $psyms main.* symbols and $pgo go runtime strings" >&2
  echo "   (did pack_pe.py actually run, and did it cover .rdata?)" >&2
  exit 1
fi
python3 - "$OUT" <<'PY'
import collections, math, pathlib, struct, sys
d = pathlib.Path(sys.argv[1]).read_bytes()
e = struct.unpack_from("<I", d, 0x3C)[0]
nsec = struct.unpack_from("<H", d, e + 6)[0]
tbl = e + 24 + struct.unpack_from("<H", d, e + 20)[0]
bad = []
for i in range(nsec):
    ent = tbl + i * 40
    name = d[ent:ent + 8].rstrip(b"\0").decode("latin1")
    if name not in (".text", ".rdata", ".pdata"):
        continue
    ptr, size = struct.unpack_from("<I", d, ent + 20)[0], struct.unpack_from("<I", d, ent + 16)[0]
    blob = d[ptr:ptr + size]
    c = collections.Counter(blob)
    ent_bits = -sum(v / len(blob) * math.log2(v / len(blob)) for v in c.values())
    print(f"   {name:<8} entropy {ent_bits:.3f} / 8.0")
    if ent_bits < 7.9:
        bad.append(name)
if bad:
    print(f"build: FAILED -- {bad} are not encrypted (entropy too low)", file=sys.stderr)
    sys.exit(1)
PY
echo "   0 main.* symbols, 0 go runtime strings, code sections indistinguishable from random"

echo "== it runs, and it still types the same message (wine, no display attached) =="
# The only stage that tests the emitted signal rather than the file. Jitter is seeded from
# GetTickCount so the millisecond values differ every run; the invariant is the decoded
# message. Set SKIP_CAPTURE=1 to skip -- but never for a release build.
if [ "${SKIP_CAPTURE:-0}" = "1" ]; then
  echo "   SKIPPED (SKIP_CAPTURE=1)"
else
  (cd tools && python3 capture_wine.py "../$OUT" --seconds 75 --expect "$TOKEN")
fi

echo
echo "build: $OUT ready"

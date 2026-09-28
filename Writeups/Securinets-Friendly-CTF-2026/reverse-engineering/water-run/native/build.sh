#!/usr/bin/env bash
# Build the game-track native challenge library, then prove it.
#
# Pipeline: generate the key header -> unit tests -> build the shipped DLL ->
# build the Godot extension that wraps it -> cross-check the compiled token
# against the service -> confirm the artifacts give nothing away -> prove the
# extension actually loads in Godot.
#
# The load-bearing stages are the cross-check and the Godot load check. A Python
# reimplementation agreeing with itself proves nothing; what matters is that the
# compiled library and the service that verifies its tokens compute the same
# value, and that the binding the game calls really reaches that library.
#
# The extension exists so score and health live in a plain C struct on the
# process heap, where a 4-byte Cheat Engine scan can find them. Keeping that
# state in GDScript would put it behind Variant on a managed heap and dissolve
# the challenge -- see notes/design.md.
#
# Overrides: GODOT=/path/to/godot if the binary is not on PATH.
set -euo pipefail
cd "$(dirname "$0")"

CC=gcc
XCC=x86_64-w64-mingw32-gcc
GODOT=${GODOT:-godot}
KEY=keys/game.key
VARIANT=${CG_VARIANT:-beginner}
EXTRA_CFLAGS=""
if [ "$VARIANT" = "advanced" ]; then
  FLAG=keys/game-advanced.flag
  EXTRA_CFLAGS="-DCG_VELOCITY_MODE"
elif [ "$VARIANT" = "beginner" ]; then
  FLAG=keys/game.flag
else
  echo "build: FAILED -- CG_VARIANT must be beginner or advanced" >&2; exit 1
fi
DLL=build/cgchallenge.dll
SO=build/libcgchallenge.so
EXT_DLL=build/gdext/cgchallenge.dll
EXT_SO=build/gdext/libcgchallenge.so
MANIFEST=cgchallenge.gdextension
GDHDR=gdext_include
GAMEDIR=../game1
CHECKDIR=build/gdextcheck
WARN="-std=c99 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion"

# A stripped PE keeps its export table but has no symbol table, so `nm -g` on the
# DLL reports nothing at all -- which would make every export check below pass
# vacuously. Read the export directory instead.
dll_exports() {
  x86_64-w64-mingw32-objdump -p "$1" \
    | sed -n '/\[Ordinal\/Name Pointer\] Table/,/^$/p' \
    | awk '{print $NF}'
}

mkdir -p build build/gdext "$GDHDR"
rm -f "$DLL" "$SO" "$EXT_DLL" "$EXT_SO" build/test_challenge build/mint.exe build/mint
rm -rf "$CHECKDIR"

echo "== generate the key header =="
python3 tools/gen_keyhdr.py "$KEY" --out src/gamekey.h
test -s "$FLAG" || { echo "build: FAILED -- missing $FLAG" >&2; exit 1; }
python3 tools/gen_flaghdr.py "$FLAG" "$KEY" --out src/gamevault.h

echo "== unit tests (designer build) =="
$CC $WARN -Isrc -DCG_DESIGNER $EXTRA_CFLAGS -O1 \
    tests/test_challenge.c src/challenge.c src/sha256.c -o build/test_challenge
./build/test_challenge

echo "== build the shipped library =="
$XCC $WARN $EXTRA_CFLAGS -O2 -s -Isrc -shared \
     src/challenge.c src/sha256.c -o "$DLL"
$CC $WARN $EXTRA_CFLAGS -O2 -s -Isrc -fPIC -fvisibility=hidden -shared \
    src/challenge.c src/sha256.c -o "$SO"
echo "   $(stat -c%s "$DLL") bytes (dll), $(stat -c%s "$SO") bytes (so)"

echo "== build the godot extension =="
GODOT=$(command -v "$GODOT" || true)
if [ -z "$GODOT" ]; then
  echo "build: FAILED -- godot not found; set GODOT=/path/to/godot" >&2; exit 1
fi
GODOT=$(readlink -f "$GODOT")
# The engine owns this header. Re-dumping it every build is what keeps the
# binding pinned to the Godot the game actually ships against.
( cd "$GDHDR" && "$GODOT" --headless --dump-gdextension-interface >/dev/null )
test -s "$GDHDR/gdextension_interface.h" \
  || { echo "build: FAILED -- no gdextension_interface.h was dumped" >&2; exit 1; }
"$GODOT" --version | sed 's/^/   godot /'
$XCC $WARN $EXTRA_CFLAGS -O3 -flto -fno-ident -DCG_EMBEDDED -s -Isrc -I"$GDHDR" -shared \
     src/challenge.c src/sha256.c src/gdext.c -o "$EXT_DLL"
$CC $WARN $EXTRA_CFLAGS -O3 -flto -fno-ident -DCG_EMBEDDED -s -Isrc -I"$GDHDR" -fPIC -fvisibility=hidden -shared \
    src/challenge.c src/sha256.c src/gdext.c -o "$EXT_SO"
echo "   $(stat -c%s "$EXT_DLL") bytes (dll), $(stat -c%s "$EXT_SO") bytes (so)"

echo "== the entry symbol matches the manifest =="
# A mismatch here is the classic silent GDExtension failure: the library builds,
# the project loads, and the class simply never appears in ClassDB.
entry=$(sed -n 's/^entry_symbol *= *"\(.*\)"/\1/p' "$MANIFEST")
[ -n "$entry" ] || { echo "build: FAILED -- $MANIFEST names no entry_symbol" >&2; exit 1; }
nm -D --defined-only "$EXT_SO" | grep -qw "$entry" \
  || { echo "build: FAILED -- $EXT_SO does not export $entry" >&2; exit 1; }
dll_exports "$EXT_DLL" | grep -qx "$entry" \
  || { echo "build: FAILED -- $EXT_DLL does not export $entry" >&2; exit 1; }
echo "   confirmed: both artifacts export $entry"

echo "== the compiled library agrees with the service =="
$XCC $WARN -O2 -Isrc -DCG_DESIGNER $EXTRA_CFLAGS \
     tests/mint.c src/challenge.c src/sha256.c -o build/mint.exe
python3 tools/crosscheck.py build/mint.exe "$KEY" --runner wine

echo "== designer mode is not in the shipped artifacts =="
for art in "$SO" "$EXT_SO"; do
  if nm -D --defined-only "$art" 2>/dev/null | grep -q cg_designer; then
    echo "build: FAILED -- $art exports designer entry points" >&2; exit 1
  fi
done
for art in "$DLL" "$EXT_DLL"; do
  if dll_exports "$art" | grep -q cg_designer; then
    echo "build: FAILED -- $art exports designer entry points" >&2; exit 1
  fi
done
echo "   confirmed: no cg_designer_* symbol in any of the four artifacts"

echo "== the artifacts give nothing away =="
fail=0
for art in "$DLL" "$SO" "$EXT_DLL" "$EXT_SO"; do
  if { strings "$art"; strings -el "$art"; } \
     | grep -nEi "Securinets|CTF\{|flag|/home/[a-zA-Z]+"; then
    echo "build: FAILED -- $art names a flag, a brand, or a build host" >&2; fail=1
  fi
  # The key is in the artifact by necessity -- GAME-HACKING-TRACK.md records that
  # ceiling. What must not be true is that it sits there in the clear, matching the
  # key file byte for byte.
  if xxd -p "$art" | tr -d '\n' | grep -qi "$(tr -d '[:space:]' < "$KEY")"; then
    echo "build: FAILED -- $art carries the raw key" >&2; fail=1
  fi
done
[ "$fail" -eq 0 ] || exit 1
echo "   no flag, no brand, no build path, no raw key"

echo "== stage the extension into the godot project =="
mkdir -p "$GAMEDIR/bin"
cp "$EXT_SO" "$GAMEDIR/bin/libcgchallenge.so"
cp "$EXT_DLL" "$GAMEDIR/bin/cgchallenge.dll"
cp "$MANIFEST" "$GAMEDIR/bin/$MANIFEST"
# The editor writes this on first open; writing it here means a headless run of
# the project loads the extension too, without anyone having opened the editor.
mkdir -p "$GAMEDIR/.godot"
touch "$GAMEDIR/.godot/extension_list.cfg"
grep -qxF "res://bin/$MANIFEST" "$GAMEDIR/.godot/extension_list.cfg" \
  || echo "res://bin/$MANIFEST" >> "$GAMEDIR/.godot/extension_list.cfg"
echo "   $GAMEDIR/bin/{libcgchallenge.so,cgchallenge.dll,$MANIFEST}"

echo "== the extension loads in godot =="
# Run against a throwaway project rather than game1: this stage has to fail only
# when the extension is broken, never because someone is mid-edit on the game.
mkdir -p "$CHECKDIR/bin" "$CHECKDIR/.godot"
cp tools/gdextcheck/project.godot tools/gdextcheck/check.gd "$CHECKDIR/"
cp "$EXT_SO" "$CHECKDIR/bin/libcgchallenge.so"
cp "$MANIFEST" "$CHECKDIR/bin/$MANIFEST"
echo "res://bin/$MANIFEST" > "$CHECKDIR/.godot/extension_list.cfg"
"$GODOT" --headless --path "$CHECKDIR" --script check.gd \
  || { echo "build: FAILED -- the extension did not load or misbehaved in Godot" >&2; exit 1; }

echo
echo "build: $DLL and $SO ready"
echo "build: $EXT_DLL and $EXT_SO staged in $GAMEDIR/bin"
echo "build: variant $VARIANT"

#!/usr/bin/env bash
# Official solver for Night Shift -- recover the rotation key, then use it.
#
# There is no library compare to trace here: the check is inlined arithmetic
# against a table in .rodata. solve.py locates that table and inverts the
# transform; this wrapper then does what the player does next -- types the key
# in and reads the flag off the screen.
#
#   solve.sh [path-to-binary]     default: build/nightshift
set -euo pipefail

HERE=$(cd "$(dirname "$0")/.." && pwd)
BIN=$(realpath -e "${1:-$HERE/build/nightshift}")
cd "$HERE"

command -v objdump >/dev/null 2>&1 || { echo "solve: objdump is not installed" >&2; exit 1; }
[ -x "$BIN" ] || { echo "solve: $BIN is not executable" >&2; exit 1; }

KEY=$(python3 solution/solve.py --quiet "$BIN")
[ -n "$KEY" ] || { echo "solve: solve.py recovered no key" >&2; exit 1; }

FLAG=$(printf '%s\n' "$KEY" | "$BIN" | grep -ao 'Securinets{[^}]*}' | head -1)
case "$FLAG" in
    Securinets\{*\}) ;;
    *) echo "solve: the recovered key was refused" >&2
       echo "   key: $KEY" >&2
       echo "   got: $FLAG" >&2; exit 1;;
esac

echo "key   : $KEY"
echo "flag  : $FLAG"

#!/usr/bin/env bash
# Official solver for Decompile Me -- uniform entry point.
#
# Two halves, and both are the player's own path: derive the access code
# from the decompiled assembly (solve.py, via ILSpy), then hand it to the
# program so the program unseals its own release token. Nothing here knows the
# flag; it comes back out of the artifact.
#
#   solve.sh [path-to-exe]     default: build/CodeCheck.exe
set -euo pipefail
HERE=$(cd "$(dirname "$0")/.." && pwd)
EXE=${1:-$HERE/build/CodeCheck.exe}

command -v wine >/dev/null || {
    echo "solve: wine is not installed -- needed to run a .NET Framework exe on Linux" >&2
    exit 1
}

CODE=$(python3 "$HERE/solution/solve.py" --binary "$EXE" --quiet)
echo "code   $CODE"

OUT=$(printf '%s\n' "$CODE" | WINEDEBUG=-all timeout 180 wine "$EXE" 2>/dev/null)
FLAG=$(grep -ao 'Securinets{[^}]*}' <<<"$OUT" | head -1)
[ -n "$FLAG" ] || { echo "solve: the program did not release a token"; printf '%s\n' "$OUT"; exit 1; }
echo "flag   $FLAG"

#!/usr/bin/env bash
# Official solver for Doorman -- the player's route, automated.
#
# `strings` on the binary yields nothing: the badge code is sealed and only
# exists in memory. But the process compares it with strcmp, so the library
# call hands the code over the moment it is made. Feed junk, read the second
# argument out of the trace, feed that back.
#
#   solve.sh [path-to-binary]     default: build/doorman
set -euo pipefail

HERE=$(cd "$(dirname "$0")/.." && pwd)
BIN=$(realpath -e "${1:-$HERE/build/doorman}")
cd "$HERE"

command -v ltrace >/dev/null 2>&1 || { echo "solve: ltrace is not installed" >&2; exit 1; }
[ -x "$BIN" ] || { echo "solve: $BIN is not executable" >&2; exit 1; }

TRACE=$(printf 'x\n' | ltrace -e strcmp "$BIN" 2>&1 || true)
BADGE=$(sed -n 's/.*strcmp("[^"]*", "\([^"]*\)").*/\1/p' <<<"$TRACE" | head -1)
[ -n "$BADGE" ] || {
    echo "solve: no strcmp against a badge code appeared in the trace" >&2
    echo "$TRACE" | tail -5 >&2
    exit 1
}

FLAG=$(printf '%s\n' "$BADGE" | "$BIN" | grep -ao 'Securinets{[^}]*}' | head -1)
case "$FLAG" in
    Securinets\{*\}) ;;
    *) echo "solve: the recovered badge code did not open the door" >&2
       echo "   got: $FLAG" >&2; exit 1;;
esac

echo "badge : $BADGE"
echo "flag  : $FLAG"

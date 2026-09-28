#!/usr/bin/env bash
# Official solver for Broken Lock -- uniform entry point.
#
# Defers to solve.py, which works on the stripped binary players receive.
# patch.py is the other half: it needs build/lock.dbg and exists so build.sh
# can patch by symbol instead of by search.
#
#   solve.sh [path-to-binary]     default: build/lock
set -euo pipefail
HERE=$(cd "$(dirname "$0")/.." && pwd)
exec python3 "$HERE/solution/solve.py" "${1:-$HERE/build/lock}"

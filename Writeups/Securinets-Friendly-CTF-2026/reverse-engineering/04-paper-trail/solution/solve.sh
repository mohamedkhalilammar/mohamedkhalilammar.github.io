#!/usr/bin/env bash
# Official solver for Paper Trail -- uniform entry point.
#
# solve.py has to run under the challenge's own Python 3.8 venv: uncompyle6
# only handles bytecode up to 3.8, which is the whole reason the bundle was
# frozen on 3.8 in the first place.
#
#   solve.sh [path-to-binary]     default: build/paper_trail
set -euo pipefail
HERE=$(cd "$(dirname "$0")/.." && pwd)
PY="$HERE/.venv/bin/python"
[ -x "$PY" ] || {
    echo "solve: $PY is missing -- create it first:" >&2
    echo "   cd $HERE && uv venv --python 3.8 .venv && .venv/bin/pip install pyinstxtractor-ng uncompyle6" >&2
    exit 1
}
cd "$HERE"
exec "$PY" solution/solve.py --binary "${1:-build/paper_trail}"

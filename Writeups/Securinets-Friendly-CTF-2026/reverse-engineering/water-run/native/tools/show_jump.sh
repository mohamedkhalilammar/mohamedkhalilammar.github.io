#!/usr/bin/env bash
# Proves, on this machine, what is in native memory for each shipped variant.
# Rebuilds each variant and asks the extension directly. No Windows needed.
# Leaves the beginner variant staged in ../../game1/bin when it finishes.
set -euo pipefail
cd "$(dirname "$0")/.."
HERE=$(pwd)

for variant in advanced beginner; do
    echo
    echo "=== $variant ==="
    CG_VARIANT="$variant" ./build.sh >/tmp/cg-build-$variant.log 2>&1 \
        || { echo "  build FAILED -- see /tmp/cg-build-$variant.log" >&2; exit 1; }
    godot --headless --path ../game1 --script "$HERE/tools/show_jump.gd" 2>&1 \
        | grep -E "mode\(\)|drift\(\)|->|NOT LOADED"
done

echo
echo "beginner is now the staged variant in game1/bin."
echo "To play it and watch the HUD:  godot --path game1"

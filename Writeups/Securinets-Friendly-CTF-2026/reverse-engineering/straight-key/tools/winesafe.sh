#!/usr/bin/env bash
# Run a Windows binary under wine with NO reachable display server.
#
# This exists because the sample injects keystrokes. On a Wayland session wine will
# happily pick the Wayland driver even when launched under xvfb-run, and the keypresses
# land on the developer's real desktop -- notes/challenge-page.md documents this trap for
# players and the build script used to walk straight into it.
#
# Isolation is done at the environment level rather than with Xvfb, because Xvfb is
# currently broken on this host (missing libnettle.so.9). Both DISPLAY and WAYLAND_DISPLAY
# are unset and wine's graphics driver is pinned to "null", so there is no transport a
# synthesised keystroke could travel over. The timing behaviour under test is produced by
# the sample's own wait loop and does not depend on a display being present.
#
#   ./winesafe.sh [--timeout N] [--debug CHANNELS] -- <exe> [args...]
set -uo pipefail

TIMEOUT=120
DEBUGCH=""
while [ $# -gt 0 ]; do
  case "$1" in
    --timeout) TIMEOUT="$2"; shift 2 ;;
    --debug)   DEBUGCH="$2";  shift 2 ;;
    --) shift; break ;;
    *) break ;;
  esac
done
[ $# -ge 1 ] || { echo "winesafe: no command given" >&2; exit 2; }

export WINEPREFIX="${WINEPREFIX:-$HOME/.wine-straightkey}"
export WINEDEBUG="${DEBUGCH:--all}"
export WINEDLLOVERRIDES="winewayland.drv=d;winex11.drv=d"

if [ ! -d "$WINEPREFIX" ]; then
  env -u DISPLAY -u WAYLAND_DISPLAY WINEDEBUG=-all wineboot -i >/dev/null 2>&1
fi

exec env -u DISPLAY -u WAYLAND_DISPLAY timeout "$TIMEOUT" wine "$@"

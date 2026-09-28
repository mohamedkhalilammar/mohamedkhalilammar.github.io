#!/usr/bin/env bash
# Official solver for Dead Air -- makes the dead host answer, then answers it.
#
# Three moves, the same three a player makes by hand:
#   1. watch the process to learn the name it is calling  (strace)
#   2. point that name at a machine you control           (/etc/hosts)
#   3. answer with the token it asks for                  (relay.py, or nc)
#
# Steps 2 and 3 run inside an unprivileged user+mount+net namespace so nothing
# on the real host is touched: /etc/hosts is bind-mounted for those processes
# only. A player editing their own /etc/hosts as root gets the same result.
#
#   solve.sh [path-to-binary]     default: build/relay
set -euo pipefail

HERE=$(cd "$(dirname "$0")/.." && pwd)
BIN=$(realpath -e "${1:-$HERE/build/relay}")
cd "$HERE"

for tool in strace unshare python3; do
    command -v "$tool" >/dev/null 2>&1 || { echo "solve: $tool is not installed" >&2; exit 1; }
done

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

echo "== step 1: what is it calling =="
strace -f -s 300 "$BIN" > "$WORK/trace" 2>&1 || true
HOST=$(python3 solution/read_trace.py --field host "$WORK/trace")
echo "   host : $HOST"
printf '127.0.0.1 %s\n' "$HOST" > "$WORK/hosts"

echo "== step 2: point it at us, and see which port it wants =="
unshare -r -m -n sh -c "
    ip link set lo up
    mount --bind '$WORK/hosts' /etc/hosts
    strace -f -s 300 '$BIN' > '$WORK/trace2' 2>&1 || true
"
PORT=$(python3 solution/read_trace.py --field port "$WORK/trace2")
echo "   port : $PORT"

echo "== step 3: answer with the token it asks for =="
OUT=$(unshare -r -m -n sh -c "
    ip link set lo up
    mount --bind '$WORK/hosts' /etc/hosts
    (python3 solution/relay.py --port '$PORT' > '$WORK/relay.log' 2>&1 &)
    sleep 0.8
    exec '$BIN'
" 2>&1) || true

FLAG=$(grep -ao 'Securinets{[^}]*}' <<<"$OUT" | head -1 || true)
[ -n "$FLAG" ] || {
    echo "solve: the relay answered but no flag came back" >&2
    echo "   binary said: $OUT" >&2
    echo "   relay said : $(cat "$WORK/relay.log" 2>/dev/null)" >&2
    exit 1
}

echo "   $(cat "$WORK/relay.log")"
echo
echo "host  : $HOST"
echo "port  : $PORT"
echo "flag  : $FLAG"

#!/usr/bin/env bash
# Starts every backend service the Android track depends on, so the
# pre-event checklist is "run this script" instead of remembering three
# separate commands and a fourth service (the rogue SDK collector) that is
# easy to forget because nothing in the app breaks loudly if it is down.
#
# Services:
#   :8000 - app.py --http   (plain HTTP: #9 login, #10/#11/#12 rely on the
#                             same process for /profile, /audit, /admin/report)
#   :8443 - app.py --https  (TLS + pinning surface for #10/#11)
#   :9090 - rogue_sdk_collector.py (#14 "The Passenger" flag delivery)
#
# Usage:
#   ./start_all.sh              # normal mode
#   CTF_SIG_DISABLED=1 ./start_all.sh   # designer mode, X-Sig checking off
#
# Ctrl-C stops all three. Logs go to ./logs/*.log.

set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

mkdir -p logs
PIDS=()

cleanup() {
    echo "Stopping backend services..."
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null || true
    done
}
trap cleanup EXIT INT TERM

python3 app.py --http --port 8000 > logs/app_http.log 2>&1 &
PIDS+=("$!")
echo "app.py (HTTP)  -> :8000  (pid $!)"

python3 app.py --https --port 8443 > logs/app_https.log 2>&1 &
PIDS+=("$!")
echo "app.py (HTTPS) -> :8443  (pid $!)"

python3 rogue_sdk_collector.py --port 9090 > logs/collector.log 2>&1 &
PIDS+=("$!")
echo "rogue_sdk_collector.py -> :9090  (pid $!)"

echo "All services up. Ctrl-C to stop."
wait

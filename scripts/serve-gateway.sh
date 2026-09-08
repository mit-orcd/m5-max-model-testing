#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"

HOST=127.0.0.1
PORT=4000

kill_port "$PORT"
exec "$ROOT/.venv/bin/python" "$ROOT/scripts/gateway.py" \
  --host "$HOST" \
  --port "$PORT" \
  --main http://127.0.0.1:8080 \
  --fast http://127.0.0.1:8081

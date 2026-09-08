#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"

start_if_down() {
  local name="$1"
  local url="$2"
  local script="$3"
  local log="$4"
  local tries="$5"
  if curl -fsS --max-time 2 "$url" >/dev/null 2>&1; then
    echo "$name already up"
    return 0
  fi
  echo "Starting $name..."
  setsid nohup "$script" >"$log" 2>&1 < /dev/null &
  disown || true
  if ! wait_http "$url" "$tries"; then
    echo "$name failed to start. See $log" >&2
    return 1
  fi
  echo "$name ready"
}

kill_port 8080
start_if_down "gpt-oss" "http://127.0.0.1:8080/v1/models" "$ROOT/scripts/serve-mlx.sh" /tmp/mlx-server.log 120
start_if_down "8B" "http://127.0.0.1:8081/v1/models" "$ROOT/scripts/serve-fast.sh" /tmp/mlx-fast.log 90

echo "Starting gateway..."
setsid nohup "$ROOT/scripts/serve-gateway.sh" >/tmp/mlx-gateway.log 2>&1 < /dev/null &
disown || true
if ! wait_http "http://127.0.0.1:4000/v1/models" 20; then
  echo "gateway failed to start. See /tmp/mlx-gateway.log" >&2
  exit 1
fi
echo "gateway ready on http://127.0.0.1:4000/v1"
curl -fsS --max-time 2 http://127.0.0.1:4000/health || true
echo

#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"

HOST=127.0.0.1
PORT=8082
MODEL=mlx-community/Ornith-1.5-35B-A3B-OptiQ-4bit

kill_port "$PORT"
export APC_ENABLED=1
exec "$ROOT/.venv/bin/mlx_lm.server" \
  --model "$MODEL" \
  --host "$HOST" \
  --port "$PORT" \
  --max-tokens 16384

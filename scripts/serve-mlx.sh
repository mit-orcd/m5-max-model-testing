#!/usr/bin/env bash
# Main model for Hermes: gpt-oss-20b on :8080.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
HOST=127.0.0.1
PORT=8080
MODEL="${MAIN_MODEL:-mlx-community/gpt-oss-20b-MXFP4-Q8}"

kill_port "$PORT"

export APC_ENABLED=1
exec "$ROOT/.venv/bin/mlx_lm.server" \
  --model "$MODEL" \
  --host "$HOST" \
  --port "$PORT" \
  --max-tokens 16384

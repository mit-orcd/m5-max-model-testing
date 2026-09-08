#!/usr/bin/env bash
# Fallback: original Qwen 3.8 27B + MTP on :8080.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
HOST=127.0.0.1
PORT=8080
MODEL=mlx-community/Qwen3.8-27B-4bit
DRAFT=mlx-community/Qwen3.8-27B-MTP-4bit

kill_port "$PORT"

export APC_ENABLED=1
exec "$ROOT/.venv/bin/mlx_vlm.server" \
  --model "$MODEL" \
  --draft-model "$DRAFT" \
  --host "$HOST" \
  --port "$PORT" \
  --max-tokens 16384 \
  --max-kv-size 65536

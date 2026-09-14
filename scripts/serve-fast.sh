#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"

HOST=127.0.0.1
PORT=8081

if [[ "$(uname -s)" == "Darwin" ]]; then
  MODEL=mlx-community/Qwen3-8B-4bit
  kill_port "$PORT"
  export APC_ENABLED=1
  exec "$ROOT/.venv/bin/mlx_lm.server" \
    --model "$MODEL" \
    --host "$HOST" \
    --port "$PORT" \
    --max-tokens 4096 \
    --chat-template-args '{"enable_thinking":false}'
fi

# Linux: same job (titles/compression) via llama.cpp. Alias must match
# FAST_MODEL_ID in the gateway environment.
kill_port "$PORT"
exec "$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" exec qwen8b-fast

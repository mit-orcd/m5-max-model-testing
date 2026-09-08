#!/usr/bin/env bash
# Serve one comparison model on :8083, bench + eval it, tear down.
# Usage: compare-model.sh <target> [eval_trials]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
TARGET="${1:?usage: compare-model.sh <target> [trials]}"
TRIALS="${2:-3}"
MODEL="$("$ROOT/.venv/bin/python" -c "
import sys; sys.path.insert(0, '$ROOT/scripts')
from bench import TARGETS
print(TARGETS['$TARGET']['model'])")"
PORT=8083
SERVER=mlx_lm.server
if [[ "$TARGET" == "qwen27" ]]; then
  SERVER=mlx_vlm.server
fi

kill_port "$PORT"
echo "=== $TARGET: $MODEL"
export APC_ENABLED=1
if [[ "$SERVER" == "mlx_vlm.server" ]]; then
  nohup "$ROOT/.venv/bin/$SERVER" \
    --model "$MODEL" --host 127.0.0.1 --port "$PORT" --max-tokens 16384 --max-kv-size 65536 \
    >/tmp/mlx-cmp-$TARGET.log 2>&1 &
else
  nohup "$ROOT/.venv/bin/$SERVER" \
    --model "$MODEL" --host 127.0.0.1 --port "$PORT" --max-tokens 16384 \
    >/tmp/mlx-cmp-$TARGET.log 2>&1 &
fi
if ! wait_http "http://127.0.0.1:$PORT/v1/models" 600; then
  echo "$TARGET failed to start; see /tmp/mlx-cmp-$TARGET.log" >&2
  exit 1
fi
echo "--- speed"
"$ROOT/.venv/bin/python" "$ROOT/scripts/bench.py" --target "$TARGET" --case decode --trials 2 || true
echo "--- C eval ($TRIALS trials)"
"$ROOT/.venv/bin/python" "$ROOT/scripts/eval_code.py" --target "$TARGET" --trials "$TRIALS" || true
kill_port "$PORT"
echo "=== $TARGET done"

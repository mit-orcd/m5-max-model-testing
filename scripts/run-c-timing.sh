#!/usr/bin/env bash
# Re-score the easy C suite for the models whose original run predates per-task
# wall-time recording. Only -ceval.json lacked times; their hard-C, Python and
# Bash files already carry them, so this is the one gap keeping these models out
# of the cost-per-solution table.
#
# deepseek-32b is deliberately absent: it is too slow to be a candidate and is
# scored in the also-ran section instead.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

ALL=(coder qwen35 qwen36-27b ornith aya glm-flash)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

for t in "${ALL[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  port=8083
  [[ "$t" == "ornith" ]] && port=8082
  model="$(model_of "$t")"
  kill_port "$port"
  export APC_ENABLED=1
  nohup "$ROOT/.venv/bin/mlx_lm.server" --model "$model" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 >"/tmp/mlx-ctiming-$t.log" 2>&1 &
  if wait_http "http://127.0.0.1:$port/v1/models" 900; then
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --set easy --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-ceval.json" 2>/dev/null || true
    echo "  $t done ($(date +%H:%M:%S))"
  else
    echo "  $t FAILED to serve"
  fi
  kill_port "$port" 2>/dev/null
  sleep 3
done

echo "C_TIMING_DONE ($(date +%H:%M:%S))"

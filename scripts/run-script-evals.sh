#!/usr/bin/env bash
# Run the Python + Bash evals against the top models, one server at a time.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

TARGETS=(gptoss gemma coder-next devstral qwen36-35b qwen27)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

for t in "${TARGETS[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  port=8083; server=mlx_lm.server; extra=()
  model="$(model_of "$t")"
  if [[ "$t" == "qwen27" ]]; then
    server=mlx_vlm.server; extra=(--max-kv-size 65536)
  fi
  kill_port "$port"
  export APC_ENABLED=1
  nohup "$ROOT/.venv/bin/$server" --model "$model" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-script-$t.log" 2>&1 &
  if wait_http "http://127.0.0.1:$port/v1/models" 600; then
    "$PY" "$ROOT/scripts/eval_python.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-python.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_bash.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-bash.json" 2>/dev/null || true
  else
    echo "  $t FAILED to serve"
  fi
  kill_port "$port" 2>/dev/null
  sleep 3
done
echo "SCRIPT EVALS DONE ($(date +%H:%M:%S))"

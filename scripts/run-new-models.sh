#!/usr/bin/env bash
# Full benchmark suite for the 4 newly added MLX models:
#   devstral2 (Devstral Small 2 24B), north (North Mini Code 1.0),
#   laguna (Laguna XS.2), qwen38flash (Qwen3.8-Flash-Next MoE)
# Per model: speed, quality, C/Python/Bash (easy+hard), research, wikitext perplexity.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

ALL=(devstral2 north laguna qwen38flash)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

for t in "${ALL[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  port=8083
  model="$(model_of "$t")"
  kill_port "$port"
  export APC_ENABLED=1
  nohup "$ROOT/.venv/bin/mlx_lm.server" --model "$model" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 >"/tmp/mlx-new-$t.log" 2>&1 &
  if wait_http "http://127.0.0.1:$port/v1/models" 1800; then
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --json \
      > "$OUT/$t-speed.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --case quality --json \
      > "$OUT/$t-quality.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --set easy --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-ceval.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-chard.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_python.py" --target "$t" --trials 3 --set easy --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-python.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_python.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-pyhard.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_bash.py" --target "$t" --trials 3 --set easy --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-bash.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_bash.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-shhard.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_research.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-research.json" 2>/dev/null || true
  else
    echo "  $t FAILED to serve"
  fi
  kill_port "$port" 2>/dev/null
  sleep 3
  # perplexity loads the model itself (needs the GPU free of the server)
  "$ROOT/.venv/bin/mlx_lm.perplexity" --model "$model" \
    --data-path /tmp/wikitext --num-samples 50 --seed 0 \
    > "$OUT/$t-wikitext-perplexity.txt" 2>&1 || true
  echo "  $t done ($(date +%H:%M:%S))"
done

echo "NEW MODELS DONE ($(date +%H:%M:%S))"

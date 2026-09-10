#!/usr/bin/env bash
# GGUF-via-Ollama eval suite for models mlx-lm can't serve.
# Usage: run-gguf-evals.sh target [target...]
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

if ! curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  nohup ollama serve >/tmp/ollama.log 2>&1 &
  sleep 3
fi

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

for t in "$@"; do
  echo "##### $t ($(date +%H:%M:%S))"
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
  ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true
  echo "  $t done ($(date +%H:%M:%S))"
done

echo "GGUF EVALS DONE ($(date +%H:%M:%S))"

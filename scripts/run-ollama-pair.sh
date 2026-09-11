#!/usr/bin/env bash
# Full benchmark suite for the two Ollama-served models:
#   llama33 (llama3.3:70b) and qwen3-30b (qwen3:30b)
# Both are served by the already-running `ollama serve` on :11434, so there is no
# per-model server spin-up. Per model: speed, quality, C/Python/Bash (easy+hard),
# research. (No MLX wikitext perplexity — these are GGUF/Ollama, not MLX.)
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

ALL=(qwen3-30b llama33)

# Ensure ollama is up
if ! curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  echo "ollama serve not running on :11434 — start it first" >&2
  exit 1
fi

for t in "${ALL[@]}"; do
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
  echo "  $t done ($(date +%H:%M:%S))"
done

echo "OLLAMA PAIR DONE ($(date +%H:%M:%S))"

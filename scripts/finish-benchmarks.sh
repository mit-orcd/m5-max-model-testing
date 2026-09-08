#!/usr/bin/env bash
# Finish the remaining benchmark phases: perplexity (all MLX models) + Ollama comparison.
set -uo pipefail
ROOT="/Users/erbmi1/git/m5-max-model-testing"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT"

MLX_TARGETS=(qwen27 ornith coder qwen35 gptoss gemma devstral aya qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

for t in "${MLX_TARGETS[@]}"; do
  # skip if we already have a real perplexity number
  if grep -q "Perplexity" "$OUT/$t-perplexity.txt" 2>/dev/null; then
    echo "skip $t (done)"; continue
  fi
  echo "##### perplexity $t ($(date +%H:%M:%S))"
  "$ROOT/scripts/perplexity.sh" "$(model_of "$t")" 50 > "$OUT/$t-perplexity.txt" 2>&1 || true
done

echo "##### ollama ($(date +%H:%M:%S))"
if ! curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  nohup ollama serve >/tmp/ollama.log 2>&1 &
  sleep 3
fi
if curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  "$PY" "$ROOT/scripts/bench.py" --target ollama --case both --trials 3 --json > "$OUT/ollama-speed.json" 2>/dev/null || true
  "$PY" "$ROOT/scripts/bench.py" --target ollama --case quality --json > "$OUT/ollama-quality.json" 2>/dev/null || true
  "$PY" "$ROOT/scripts/eval_code.py" --target ollama --trials 3 --json \
    --dump-failures "$OUT/failures" > "$OUT/ollama-ceval.json" 2>/dev/null || true
  ollama stop qwen3.8:27b-mlx >/dev/null 2>&1 || true
else
  echo "ollama server not running" > "$OUT/ollama-error.txt"
fi

echo "FINISH DONE ($(date +%H:%M:%S))"

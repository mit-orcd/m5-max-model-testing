#!/usr/bin/env bash
# Complete eval pass: Python + Bash for every target, C re-run (with timing) for
# the top 6 + Ollama, then WikiText plain-text perplexity for all MLX models.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

ALL=(gptoss gemma coder-next devstral qwen36-35b qwen27 ornith coder qwen35 aya qwen36-27b glm-flash deepseek-32b)
TOP6=(gptoss gemma coder-next devstral qwen36-35b qwen27)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}
in_top6() { [[ " ${TOP6[*]} " == *" $1 "* ]]; }

for t in "${ALL[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  port=8083; server=mlx_lm.server; extra=()
  [[ "$t" == "ornith" ]] && port=8082
  model="$(model_of "$t")"
  if [[ "$t" == "qwen27" ]]; then
    server=mlx_vlm.server; extra=(--max-kv-size 65536)
  fi
  kill_port "$port"
  export APC_ENABLED=1
  nohup "$ROOT/.venv/bin/$server" --model "$model" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-full-$t.log" 2>&1 &
  if wait_http "http://127.0.0.1:$port/v1/models" 600; then
    if in_top6 "$t"; then
      "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --json \
        --dump-failures "$OUT/failures" > "$OUT/$t-ceval.json" 2>/dev/null || true
    fi
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

# Ollama: full set incl. C (runtime comparison vs qwen27 MLX)
echo "##### ollama ($(date +%H:%M:%S))"
if ! curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  nohup ollama serve >/tmp/ollama.log 2>&1 &
  sleep 3
fi
"$PY" "$ROOT/scripts/eval_code.py" --target ollama --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/ollama-ceval.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_python.py" --target ollama --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/ollama-python.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_bash.py" --target ollama --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/ollama-bash.json" 2>/dev/null || true
ollama stop qwen3.8:27b-mlx >/dev/null 2>&1 || true

# WikiText plain-text perplexity for all MLX models
for t in "${ALL[@]}"; do
  echo "##### wikitext-ppl $t ($(date +%H:%M:%S))"
  "$ROOT/.venv/bin/mlx_lm.perplexity" --model "$(model_of "$t")" \
    --data-path /tmp/wikitext --num-samples 50 --seed 0 \
    > "$OUT/$t-wikitext-perplexity.txt" 2>&1 || true
done

echo "FULL EVALS DONE ($(date +%H:%M:%S))"

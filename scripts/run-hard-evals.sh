#!/usr/bin/env bash
# Run the HARD task sets + research eval across every target.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

ALL=(gptoss gptoss120 gemma coder-next devstral qwen36-35b qwen27 ornith coder qwen35 aya qwen36-27b glm-flash deepseek-32b)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

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
    --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-hard-$t.log" 2>&1 &
  if wait_http "http://127.0.0.1:$port/v1/models" 900; then
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-chard.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_python.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-pyhard.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_bash.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-shhard.json" 2>/dev/null || true
    "$PY" "$ROOT/scripts/eval_research.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-research.json" 2>/dev/null || true
  else
    echo "  $t FAILED to serve"
  fi
  kill_port "$port" 2>/dev/null
  sleep 3
done

echo "##### ollama ($(date +%H:%M:%S))"
if ! curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  nohup ollama serve >/tmp/ollama.log 2>&1 &
  sleep 3
fi
"$PY" "$ROOT/scripts/eval_code.py" --target ollama --trials 3 --set hard --json \
  --dump-failures "$OUT/failures" > "$OUT/ollama-chard.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_python.py" --target ollama --trials 3 --set hard --json \
  --dump-failures "$OUT/failures" > "$OUT/ollama-pyhard.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_bash.py" --target ollama --trials 3 --set hard --json \
  --dump-failures "$OUT/failures" > "$OUT/ollama-shhard.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_research.py" --target ollama --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/ollama-research.json" 2>/dev/null || true
ollama stop qwen3.8:27b-mlx >/dev/null 2>&1 || true

echo "HARD EVALS DONE ($(date +%H:%M:%S))"

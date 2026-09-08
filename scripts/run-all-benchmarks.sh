#!/usr/bin/env bash
# Full benchmark sweep. For every MLX target: serve, speed bench (decode+prefill),
# quality probes, C eval (3 trials, failures dumped), teardown. Then perplexity
# standalone per model. Then the Ollama-vs-MLX runtime comparison (Qwen 27B pair).
# Results land in results/ as JSON/text. Safe to re-run; overwrites results.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT" "$OUT/failures"

MLX_TARGETS=(qwen27 ornith coder qwen35 gptoss gemma devstral aya qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

serve() { # $1=target -> echoes port; returns nonzero on failure
  local t="$1" port=8083 server=mlx_lm.server extra=()
  [[ "$t" == "ornith" ]] && port=8082
  local model; model="$(model_of "$t")"
  if [[ "$t" == "qwen27" ]]; then
    server=mlx_vlm.server
    extra=(--max-kv-size 65536)
  fi
  kill_port "$port"
  export APC_ENABLED=1
  nohup "$ROOT/.venv/bin/$server" --model "$model" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-run-$t.log" 2>&1 &
  wait_http "http://127.0.0.1:$port/v1/models" 600
}

for t in "${MLX_TARGETS[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  if serve "$t"; then
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --case both --trials 2 --json > "$OUT/$t-speed.json" 2>/dev/null \
      || echo "speed-fail" > "$OUT/$t-speed.json"
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --case quality --json > "$OUT/$t-quality.json" 2>/dev/null \
      || echo "quality-fail" > "$OUT/$t-quality.json"
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-ceval.json" 2>/dev/null \
      || echo "ceval-fail" > "$OUT/$t-ceval.json"
  else
    echo "serve failed; see /tmp/mlx-run-$t.log" > "$OUT/$t-error.txt"
    echo "  $t FAILED to serve"
  fi
  kill_port 8083 2>/dev/null; kill_port 8082 2>/dev/null
  sleep 5
done

# Perplexity (loads each model standalone; server must be down to free RAM)
for t in "${MLX_TARGETS[@]}"; do
  echo "##### perplexity $t ($(date +%H:%M:%S))"
  "$ROOT/scripts/perplexity.sh" "$(model_of "$t")" 50 > "$OUT/$t-perplexity.txt" 2>&1 || true
done

# Ollama vs MLX runtime comparison (same Qwen 27B class weights)
echo "##### ollama ($(date +%H:%M:%S))"
if curl -sf http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  "$PY" "$ROOT/scripts/bench.py" --target ollama --case both --trials 3 --json > "$OUT/ollama-speed.json" 2>/dev/null || true
  "$PY" "$ROOT/scripts/bench.py" --target ollama --case quality --json > "$OUT/ollama-quality.json" 2>/dev/null || true
  "$PY" "$ROOT/scripts/eval_code.py" --target ollama --trials 3 --json \
    --dump-failures "$OUT/failures" > "$OUT/ollama-ceval.json" 2>/dev/null || true
  ollama stop qwen3.8:27b-mlx >/dev/null 2>&1 || true
else
  echo "ollama server not running" > "$OUT/ollama-error.txt"
fi

echo "ALL BENCHMARKS DONE ($(date +%H:%M:%S))"

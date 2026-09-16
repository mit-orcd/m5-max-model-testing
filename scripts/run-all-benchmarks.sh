#!/usr/bin/env bash
# Full benchmark sweep. For every target: serve, speed bench (decode+prefill),
# quality probes, C eval (3 trials, failures dumped), teardown. Then perplexity
# standalone per model. Then the Ollama runtime comparison.
#
# Platform is detected automatically: macOS sweeps MLX_TARGETS via mlx-lm,
# Linux (Ryzen AI Max+ 395) sweeps LINUX_TARGETS via llama.cpp/vLLM. Serving
# is dispatched through scripts/serve.py, which owns all runtime knowledge.
# Results land in results/ as JSON/text. Safe to re-run; overwrites results.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT" "$OUT/failures"
if [[ ! -s "$OUT/machine.json" ]]; then
  if [[ "$(uname -s)" == "Darwin" ]]; then
    cp "$ROOT/scripts/machines/m5-max.json" "$OUT/machine.json"
  elif command -v rocminfo >/dev/null && rocminfo 2>/dev/null | grep -q gfx1151; then
    cp "$ROOT/scripts/machines/strix-halo.json" "$OUT/machine.json"
  else
    cp "$ROOT/scripts/machines/rtx-pro-6000.json" "$OUT/machine.json"
  fi
fi

MLX_TARGETS=(qwen27 ornith coder qwen35 gptoss gemma devstral aya qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b)

# Linux sweep (Strix Halo): llamacpp targets plus the vLLM comparison trio.
# Fork targets (qwen38flash k2horizon laguna) need LLAMA_K2_SERVER_BIN built
# by scripts/linux-setup.sh. Ornith/Laguna-2.x-mlx/katcoder are MLX-only.
LINUX_TARGETS=(gptoss gptoss-vllm qwen27 qwen27-vllm qwen35 qwen35-vllm coder gemma devstral aya \
  qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b qwen35-122b qwen35-27b nemotron3 \
  seed-oss laguna-s qwen38flash k2horizon laguna)

if [[ -n "${SWEEP_ONLY:-}" ]]; then
  # shellcheck disable=SC2206
  SWEEP_TARGETS=($SWEEP_ONLY)
elif [[ "$(uname -s)" == "Darwin" ]]; then
  SWEEP_TARGETS=("${MLX_TARGETS[@]}")
else
  SWEEP_TARGETS=("${LINUX_TARGETS[@]}")
fi

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

serve() { # $1=target; returns nonzero on failure
  local t="$1" port
  port="$("$PY" "$ROOT/scripts/serve.py" field "$t" port)"
  kill_port "$port"
  export APC_ENABLED=1 # mlx-lm prompt cache; ignored by other runtimes
  nohup "$PY" "$ROOT/scripts/serve.py" exec "$t" >"/tmp/serve-run-$t.log" 2>&1 &
  wait_http "http://127.0.0.1:$port/v1/models" 600
}

for t in "${SWEEP_TARGETS[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  if serve "$t"; then
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --case both --trials 3 --json > "$OUT/$t-speed.json" 2>/dev/null \
      || echo "speed-fail" > "$OUT/$t-speed.json"
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --case quality --trials 3 --json > "$OUT/$t-quality.json" 2>/dev/null \
      || echo "quality-fail" > "$OUT/$t-quality.json"
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-ceval.json" 2>/dev/null \
      || echo "ceval-fail" > "$OUT/$t-ceval.json"
  else
    echo "serve failed; see /tmp/serve-run-$t.log" > "$OUT/$t-error.txt"
    echo "  $t FAILED to serve"
  fi
  kill_port 8083 2>/dev/null; kill_port 8082 2>/dev/null; kill_port 8085 2>/dev/null
  sleep 5
done

# Perplexity (loads each model standalone; server must be down to free RAM).
# llama.cpp-family targets only — vLLM/Ollama targets have no GGUF to measure.
for t in "${SWEEP_TARGETS[@]}"; do
  rt="$("$PY" "$ROOT/scripts/serve.py" field "$t" runtime 2>/dev/null || true)"
  [[ "$rt" == llamacpp* || "$(uname -s)" == "Darwin" ]] || { echo "##### perplexity $t: skip ($rt)"; continue; }
  echo "##### perplexity $t ($(date +%H:%M:%S))"
  "$ROOT/scripts/perplexity.sh" "$t" 50 > "$OUT/$t-perplexity.txt" 2>&1 || true
done

# Ollama runtime comparison (same Qwen 27B class weights)
echo "##### ollama ($(date +%H:%M:%S))"
if curl -sf http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  "$PY" "$ROOT/scripts/bench.py" --target ollama --case both --trials 3 --json > "$OUT/ollama-speed.json" 2>/dev/null || true
  "$PY" "$ROOT/scripts/bench.py" --target ollama --case quality --trials 3 --json > "$OUT/ollama-quality.json" 2>/dev/null || true
  "$PY" "$ROOT/scripts/eval_code.py" --target ollama --trials 3 --json \
    --dump-failures "$OUT/failures" > "$OUT/ollama-ceval.json" 2>/dev/null || true
  ollama stop "$(model_of ollama)" >/dev/null 2>&1 || true
else
  echo "ollama server not running" > "$OUT/ollama-error.txt"
fi

"$PY" "$ROOT/scripts/make_report.py" || true

echo "ALL BENCHMARKS DONE ($(date +%H:%M:%S))"

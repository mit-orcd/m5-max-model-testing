#!/usr/bin/env bash
# Apples-to-apples rerun of one target on this machine.
#
# Everything the harness controls is held fixed, and identical on the Mac,
# the RTX PRO 6000 and Strix Halo:
#   - same GGUF (bartowski Q4_K_M), same llama.cpp fork build (llama-k2 10671)
#   - -c 16384 --parallel 4 -> 4096 ctx per slot, flash attention on
#   - every trial temperature 0, seed 42 (BENCH_PINNED=1 in bench.py)
#   - C compiled -std=gnu11 so strdup exists on glibc and Apple libc alike
#   - reasoning_effort=none sent on every request, not just the server line
# What it cannot hold fixed is recorded in each result's "harness" block:
# GPU backend (Metal / CUDA / Vulkan), cc, bash, Python.
#
#   scripts/run-pinned.sh mistral-small4
#
# Results: results/pinned/<target>-<suite>.json. Resume-safe.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
[[ -f "$ROOT/scripts/env-strix.sh" && -d /opt/rocm ]] && source "$ROOT/scripts/env-strix.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results/pinned"
mkdir -p "$OUT/failures"
cd "$ROOT"

t="${1:?target}"
export BENCH_PINNED=1
export BENCH_SEED="${BENCH_SEED:-42}"

model_of() { "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"; }
port_of()  { "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['port'])"; }

MODELS_DIR="${MODELS_DIR:-$HOME/models}"
shard=$(ls -1 "$MODELS_DIR"/mistralai_Mistral-Small-4-119B-2603*Q4_K_M*00001*.gguf 2>/dev/null | head -1)
[[ -n "$shard" ]] || { echo "$t: no GGUF shard under $MODELS_DIR"; exit 1; }
bin="${LLAMA_K2_SERVER_BIN:-$HOME/llama-k2/build/bin/llama-server}"
[[ -x "$bin" ]] || { echo "$t: no llama-k2 at $bin"; exit 1; }
port="$(port_of "$t")"
alias="$(model_of "$t")"

echo "===== pinned $t ($(date +%H:%M:%S))"
echo "  server: $bin"
"$bin" --version 2>&1 | head -2 | sed 's/^/  /'
echo "  gguf:   $shard"
echo "  cc:     $(cc --version | head -1)"

kill_port "$port"
"$bin" -m "$shard" --alias "$alias" \
  --host 127.0.0.1 --port "$port" -ngl 99 -c 16384 --parallel 4 --flash-attn on \
  --jinja --chat-template-kwargs '{"reasoning_effort":"none"}' \
  >"/tmp/pinned-$t.log" 2>&1 &
server_pid=$!
trap 'kill $server_pid 2>/dev/null || true' EXIT
wait_http "http://127.0.0.1:$port/v1/models" 1800 "$server_pid" || { echo "  $t FAILED to serve"; exit 1; }

reply="$(curl -sf --max-time 300 "http://127.0.0.1:$port/v1/chat/completions" \
  -H 'Content-Type: application/json' \
  -d "{\"model\":\"$alias\",\"messages\":[{\"role\":\"user\",\"content\":\"say ok\"}],\"max_tokens\":32,\"temperature\":0}")"
printf '%s' "$reply" | grep -q '"content"[[:space:]]*:[[:space:]]*"[^"]' \
  || { echo "$t EMPTY content — refusing to score a runtime bug"; exit 1; }

run() {  # run <suite-file> <script> <args...>
  local f="$OUT/$t-$1.json"; shift
  [[ -s "$f" ]] && { echo "  $t $f already done"; return 0; }
  echo "  ##### $(basename "$f" .json) ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/$1.py" --target "$t" "${@:2}" --trials 3 \
    --dump-failures "$OUT/failures" --json > "$f" || true
  [[ -s "$f" ]] || rm -f "$f"
}

run ceval   eval_code   --set easy --timeout 600
run chard   eval_code   --set hard --timeout 900
run python  eval_python --set easy --timeout 600
run pyhard  eval_python --set hard --timeout 900
run bash    eval_bash   --set easy --timeout 600
run shhard  eval_bash   --set hard --timeout 900
run research eval_research --timeout 600

kill "$server_pid" 2>/dev/null || true
echo "PINNED DONE $t ($(date +%H:%M:%S))"

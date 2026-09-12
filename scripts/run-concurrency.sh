#!/usr/bin/env bash
# Concurrency sweep for the top of the coding table: the same 8 C tasks served
# with 1, 2, 4 and 8 requests in flight.
#
# Stacks are configured FOR parallelism so this compares capability, not flags:
#   mlx_lm.server   already defaults to --decode-concurrency 32 / --prompt-concurrency 8
#   llama-server    defaults to a single slot, so it gets --parallel 16 explicitly
#   ollama          gets OLLAMA_NUM_PARALLEL=8
# llama.cpp's single-slot default is a real deployment trap, noted in the docs.
#
# Results are timestamped and never overwritten:
#   results/concurrency/<target>-<YYYYmmdd-HHMMSS>.json
# Pass target names to override the default top-5 list.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
K2_BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"
LAGUNA_BLOB="$HOME/.ollama/models/blobs/sha256-771a73e1249b9bc08e17d3fca59f5c49b7b9c8a6a47b5a6ac82f95c6e76923c4"

# Slots are per-model, not global. 16-wide needs 16 slots, but a 125B model at 81 GB
# cannot fit 16 KV caches in the GPU memory left over -- it dies with
# kIOGPUCommandBufferCallbackErrorOutOfMemory the moment a second request lands.
# 8 slots is the most flash-next has been observed to fit, so the big GGUF models cap
# there and simply don't run the 12/16 levels; the smaller MLX models take 16.
SLOTS="${SLOTS:-16}"
SLOTS_FORK="${SLOTS_FORK:-8}"
TIMEOUT="${TIMEOUT:-900}"
LEVELS="${LEVELS:-1,2,4,8,12,16}"
LEVELS_FORK="${LEVELS_FORK:-1,2,4,8}"

# top 10 by coding total (skipping duplicates: original devstral, ollama, laguna-mlx)
DEFAULT=(gptoss gptoss120 coder-next qwen27 qwen38flash gemma devstral2 ornith laguna21 k2horizon)
if [[ $# -gt 0 ]]; then ALL=("$@"); else ALL=("${DEFAULT[@]}"); fi

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

run_one() {  # $1=target $2=levels (defaults to the full ladder)
  "$PY" "$ROOT/scripts/bench_concurrent.py" --target "$1" \
    --levels "${2:-$LEVELS}" --timeout "$TIMEOUT" || echo "  $1 FAILED"
}

serve_fork() {  # $1=target $2=model-path $3=extra-args
  # 32k context split across $SLOTS slots is ample: prompts are ~200 tokens
  nohup /tmp/llama-k2/build/bin/llama-server -m "$2" --alias "$1" \
    --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --parallel "$SLOTS_FORK" \
    --flash-attn on $3 > "/tmp/conc-$1-server.log" 2>&1 &
  for i in $(seq 1 120); do
    curl -sf --max-time 2 http://127.0.0.1:8085/v1/models >/dev/null 2>&1 && return 0
    sleep 5
  done
  return 1
}

for t in "${ALL[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  case "$t" in
    k2horizon)
      kill_port 8085 2>/dev/null; sleep 2
      serve_fork "$t" "$K2_BLOB" "" && run_one "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    laguna)
      kill_port 8085 2>/dev/null; sleep 2
      serve_fork "$t" "$LAGUNA_BLOB" "--chat-template-file /tmp/laguna-template.jinja" \
        && run_one "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    qwen38flash)
      kill_port 8085 2>/dev/null; sleep 2
      SHARD1=$(find "$HOME/.cache/huggingface/hub/models--unsloth--Qwen3.8-Flash-Next-GGUF/snapshots" \
        -name "*UD-Q4_K_XL*00001*" 2>/dev/null | head -1)
      # 8 slots, not 16: the 81 GB model can't fit 16 KV caches (GPU OOM), so it
      # runs the ladder only to 8-wide.
      [[ -n "$SHARD1" ]] && serve_fork "$t" "$SHARD1" "" && run_one "$t" "$LEVELS_FORK" \
        || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    north|ollama|llama33|qwen3-30b)
      pkill -f "ollama serve" 2>/dev/null; sleep 2
      OLLAMA_NUM_PARALLEL="$SLOTS" nohup ollama serve >/tmp/ollama-conc.log 2>&1 &
      sleep 5
      run_one "$t"
      ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true ;;
    *)
      port=8083; server=mlx_lm.server; extra=()
      [[ "$t" == "ornith" ]] && port=8082
      if [[ "$t" == "qwen27" ]]; then
        server=mlx_vlm.server; extra=(--max-kv-size 65536)
      elif [[ "$t" == "laguna21" || "$t" == "laguna-mlx" ]]; then
        server=mlx_vlm.server
      fi
      kill_port "$port"
      sleep 5
      # --prompt-cache-size 0 is required, not a tuning choice: with the cache on,
      # mlx_lm.server returns wrong answers at 8 concurrent requests (reproducibly,
      # the same two tasks) and loses ~38% of its throughput. See docs/benchmarks.md.
      # (APC_ENABLED, set by the older sweep scripts, is read by nothing.)
      [[ "$server" == "mlx_lm.server" ]] && extra+=(--prompt-cache-size 0)
      nohup "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
        --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-conc-$t.log" 2>&1 &
      srv=$!
      if wait_http "http://127.0.0.1:$port/v1/models" 900 && kill -0 "$srv" 2>/dev/null; then
        run_one "$t"
      else
        echo "  $t FAILED to serve"
      fi
      kill_port "$port" 2>/dev/null
      sleep 3 ;;
  esac
  echo "  $t done ($(date +%H:%M:%S))"
done

"$PY" "$ROOT/scripts/make_report.py" || true

echo "CONCURRENCY DONE ($(date +%H:%M:%S))"

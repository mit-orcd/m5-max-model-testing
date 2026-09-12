#!/usr/bin/env bash
# Prompt-framing experiment: does how you ask change the code you get?
#
# Seven framings of one C task whose outcome is binary (prefix sum or naive loop),
# 20 samples each at temperature 0.7 plus one greedy sample, reported with Wilson
# intervals and Fisher exact tests against the bare prompt.
#
# Results are timestamped and never overwritten:
#   results/framing/<target>-<YYYYmmdd-HHMMSS>.json
# Pass target names to override the default top-5 list.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
K2_BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"
LAGUNA_BLOB="$HOME/.ollama/models/blobs/sha256-771a73e1249b9bc08e17d3fca59f5c49b7b9c8a6a47b5a6ac82f95c6e76923c4"

TRIALS="${TRIALS:-20}"   # samples at temp 0.7 per condition
TIMEOUT="${TIMEOUT:-600}"

# top 10 by coding total (skipping duplicates: original devstral, ollama, laguna-mlx)
DEFAULT=(gptoss gptoss120 coder-next qwen27 qwen38flash gemma devstral2 ornith laguna21 k2horizon)
if [[ $# -gt 0 ]]; then ALL=("$@"); else ALL=("${DEFAULT[@]}"); fi

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

run_one() {  # $1=target
  "$PY" "$ROOT/scripts/eval_framing.py" --target "$1" --trials "$TRIALS" \
    --timeout "$TIMEOUT" || echo "  $1 FAILED"
}

serve_fork() {  # $1=target $2=model-path $3=extra-args
  nohup /tmp/llama-k2/build/bin/llama-server -m "$2" --alias "$1" \
    --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on $3 \
    > "/tmp/framing-$1-server.log" 2>&1 &
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
      [[ -n "$SHARD1" ]] && serve_fork "$t" "$SHARD1" "" && run_one "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    north|ollama|llama33|qwen3-30b)
      curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1 \
        || { nohup ollama serve >/tmp/ollama.log 2>&1 & sleep 3; }
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
      # the prompt cache corrupts output under load; harmless here but keep it off
      [[ "$server" == "mlx_lm.server" ]] && extra+=(--prompt-cache-size 0)
      kill_port "$port"
      sleep 5
      nohup "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
        --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-framing-$t.log" 2>&1 &
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

echo "FRAMING DONE ($(date +%H:%M:%S))"

#!/usr/bin/env bash
# Capture one decode generation (the 1500-word MIT essay) per target into
# results/speed-texts/<target>.txt for the report's collapsible samples.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
K2_BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"
LAGUNA_BLOB="$HOME/.ollama/models/blobs/sha256-771a73e1249b9bc08e17d3fca59f5c49b7b9c8a6a47b5a6ac82f95c6e76923c4"

ALL=(gptoss120 gptoss gemma coder-next devstral devstral2 qwen27 qwen36-35b qwen35 \
     qwen36-27b ornith coder deepseek-32b aya glm-flash north laguna qwen38flash k2horizon ollama)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

capture() {  # $1=target
  [[ -s "$ROOT/results/speed-texts/$1.txt" ]] && { echo "  $1 already captured"; return 0; }
  "$PY" "$ROOT/scripts/bench.py" --target "$1" --case decode --trials 1 --timeout 600 \
    > /dev/null 2>&1 || echo "  $1 capture FAILED"
}

serve_fork() {  # $1=target $2=model-path $3=extra-args
  nohup /tmp/llama-k2/build/bin/llama-server -m "$2" --alias "$1" \
    --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on $3 \
    > "/tmp/capture-$1-server.log" 2>&1 &
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
      serve_fork "$t" "$K2_BLOB" "" && capture "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    laguna)
      serve_fork "$t" "$LAGUNA_BLOB" "--chat-template-file /tmp/laguna-template.jinja" && capture "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    qwen38flash)
      SHARD1=$(find "$HOME/.cache/huggingface/hub/models--unsloth--Qwen3.8-Flash-Next-GGUF/snapshots" \
        -name "*UD-Q4_K_XL*00001*" 2>/dev/null | head -1)
      [[ -n "$SHARD1" ]] && serve_fork "$t" "$SHARD1" "" && capture "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    north|ollama)
      curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1 || { nohup ollama serve >/tmp/ollama.log 2>&1 & sleep 3; }
      capture "$t"
      ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true ;;
    *)
      port=8083; server=mlx_lm.server; extra=()
      [[ "$t" == "ornith" ]] && port=8082
      [[ "$t" == "qwen27" ]] && { server=mlx_vlm.server; extra=(--max-kv-size 65536); }
      kill_port "$port"
      export APC_ENABLED=1
      nohup "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
        --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-capture-$t.log" 2>&1 &
      if wait_http "http://127.0.0.1:$port/v1/models" 900; then
        capture "$t"
      else
        echo "  $t FAILED to serve"
      fi
      kill_port "$port" 2>/dev/null
      sleep 3 ;;
  esac
  echo "  $t done ($(date +%H:%M:%S))"
done

echo "CAPTURE SWEEP DONE ($(date +%H:%M:%S))"

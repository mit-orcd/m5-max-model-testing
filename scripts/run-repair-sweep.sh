#!/usr/bin/env bash
# C self-repair sweep: eval_repair.py across targets, most interesting first.
# Serves MLX targets as needed; ollama targets run directly; k2horizon and
# qwen38flash via the IFM fork llama-server (alias on :8085).
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
K2_BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"

# most interesting first; trimmed to fit the overnight window
ALL=(gptoss120 gptoss coder-next devstral2 qwen36-35b deepseek-32b k2horizon qwen38flash north laguna)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

run_repair() {
  "$PY" "$ROOT/scripts/eval_repair.py" --target "$1" --set all --max-rounds 5 \
    --timeout 600 --json > "$OUT/$1-repair.json" 2>/dev/null || true
}

serve_fork() {  # $1=target $2=model-path
  nohup /tmp/llama-k2/build/bin/llama-server -m "$2" --alias "$1" \
    --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on \
    > "/tmp/repair-$1-server.log" 2>&1 &
  for i in $(seq 1 120); do
    curl -sf --max-time 2 http://127.0.0.1:8085/v1/models >/dev/null 2>&1 && return 0
    sleep 5
  done
  return 1
}

for t in "${ALL[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  kind=$("$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$t']['kind'])")
  if [[ "$t" == "k2horizon" ]]; then
    serve_fork "$t" "$K2_BLOB" && run_repair "$t" || echo "  $t FAILED to serve"
    kill_port 8085 2>/dev/null
  elif [[ "$t" == "laguna" ]]; then
    LAGUNA_BLOB="$HOME/.ollama/models/blobs/sha256-771a73e1249b9bc08e17d3fca59f5c49b7b9c8a6a47b5a6ac82f95c6e76923c4"
    nohup /tmp/llama-k2/build/bin/llama-server -m "$LAGUNA_BLOB" --alias laguna \
      --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on \
      --chat-template-file /tmp/laguna-template.jinja > "/tmp/repair-laguna-server.log" 2>&1 &
    for i in $(seq 1 120); do
      curl -sf --max-time 2 http://127.0.0.1:8085/v1/models >/dev/null 2>&1 && break
      sleep 5
    done
    run_repair "$t"
    kill_port 8085 2>/dev/null
  elif [[ "$t" == "qwen38flash" ]]; then
    SHARD1=$(find "$HOME/.cache/huggingface/hub/models--unsloth--Qwen3.8-Flash-Next-GGUF/snapshots" \
      -name "*UD-Q4_K_XL*00001*" 2>/dev/null | head -1)
    [[ -n "$SHARD1" ]] && serve_fork "$t" "$SHARD1" && run_repair "$t" || echo "  $t FAILED to serve"
    kill_port 8085 2>/dev/null
  elif [[ "$kind" == "ollama" ]]; then
    curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1 || { nohup ollama serve >/tmp/ollama.log 2>&1 & sleep 3; }
    run_repair "$t"
    ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true
  else
    port=8083; server=mlx_lm.server; extra=()
    [[ "$t" == "ornith" ]] && port=8082
    [[ "$t" == "qwen27" ]] && { server=mlx_vlm.server; extra=(--max-kv-size 65536); }
    kill_port "$port"
    export APC_ENABLED=1
    nohup "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
      --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-repair-$t.log" 2>&1 &
    if wait_http "http://127.0.0.1:$port/v1/models" 900; then
      run_repair "$t"
    else
      echo "  $t FAILED to serve"
    fi
    kill_port "$port" 2>/dev/null
    sleep 3
  fi
  echo "  $t done ($(date +%H:%M:%S))"
done

"$PY" "$ROOT/scripts/make_report.py" || true

echo "REPAIR SWEEP DONE ($(date +%H:%M:%S))"

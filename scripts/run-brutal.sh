#!/usr/bin/env bash
# Brutal task sweep: one very hard task per language, 3 trials each.
#   C      utf8_next    strict UTF-8 decode (overlongs, surrogates, range)
#   Python clone_graph  deep copy that preserves cycles and shared references
#   Bash   total_size   byte total over hostile filenames on BSD userland
# Defaults to the top of the coding table; pass target names to override:
#   scripts/run-brutal.sh gptoss laguna
# Skips targets whose result file already exists.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
K2_BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"
LAGUNA_BLOB="$HOME/.ollama/models/blobs/sha256-771a73e1249b9bc08e17d3fca59f5c49b7b9c8a6a47b5a6ac82f95c6e76923c4"

TRIALS="${TRIALS:-3}"
TIMEOUT="${TIMEOUT:-900}"

# top of the coding table, cheapest to serve first
DEFAULT=(gptoss gptoss120 coder-next qwen27 gemma devstral2 ornith laguna qwen38flash)
if [[ $# -gt 0 ]]; then ALL=("$@"); else ALL=("${DEFAULT[@]}"); fi

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

run_langs() {  # $1=target
  local t="$1"
  for spec in "c:eval_code" "python:eval_python" "bash:eval_bash"; do
    local lang="${spec%%:*}" script="${spec##*:}"
    local f="$OUT/$t-brutal-$lang.json"
    [[ -s "$f" ]] && { echo "  $t $lang already done"; continue; }
    "$PY" "$ROOT/scripts/$script.py" --target "$t" --set brutal \
      --trials "$TRIALS" --timeout "$TIMEOUT" --dump-failures "$OUT/failures" \
      --json > "$f" 2>/dev/null || true
    # a failed run leaves an empty file behind that would be skipped next time
    [[ -s "$f" ]] || rm -f "$f"
    echo "  $t $lang done ($(date +%H:%M:%S))"
  done
}

serve_fork() {  # $1=target $2=model-path $3=extra-args
  nohup /tmp/llama-k2/build/bin/llama-server -m "$2" --alias "$1" \
    --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on $3 \
    > "/tmp/brutal-$1-server.log" 2>&1 &
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
      serve_fork "$t" "$K2_BLOB" "" && run_langs "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    laguna)
      kill_port 8085 2>/dev/null; sleep 2
      serve_fork "$t" "$LAGUNA_BLOB" "--chat-template-file /tmp/laguna-template.jinja" \
        && run_langs "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    qwen38flash)
      kill_port 8085 2>/dev/null; sleep 2
      SHARD1=$(find "$HOME/.cache/huggingface/hub/models--unsloth--Qwen3.8-Flash-Next-GGUF/snapshots" \
        -name "*UD-Q4_K_XL*00001*" 2>/dev/null | head -1)
      [[ -n "$SHARD1" ]] && serve_fork "$t" "$SHARD1" "" && run_langs "$t" || echo "  $t FAILED to serve"
      kill_port 8085 2>/dev/null ;;
    north|ollama)
      curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1 \
        || { nohup ollama serve >/tmp/ollama.log 2>&1 & sleep 3; }
      run_langs "$t"
      ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true ;;
    *)
      port=8083; server=mlx_lm.server; extra=()
      [[ "$t" == "ornith" ]] && port=8082
      [[ "$t" == "qwen27" ]] && { server=mlx_vlm.server; extra=(--max-kv-size 65536); }
      kill_port "$port"
      sleep 5
      export APC_ENABLED=1
      nohup "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
        --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-brutal-$t.log" 2>&1 &
      srv=$!
      if wait_http "http://127.0.0.1:$port/v1/models" 900 && kill -0 "$srv" 2>/dev/null; then
        run_langs "$t"
      else
        echo "  $t FAILED to serve"
      fi
      kill_port "$port" 2>/dev/null
      sleep 3 ;;
  esac
  echo "  $t done ($(date +%H:%M:%S))"
done

"$PY" "$ROOT/scripts/make_report.py" || true

echo "BRUTAL DONE ($(date +%H:%M:%S))"

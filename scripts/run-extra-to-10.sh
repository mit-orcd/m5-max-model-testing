#!/usr/bin/env bash
# Fill the extra sections out to 10 models, and run self-repair on the newer
# targets that never got it. Serves each model once and runs everything it is
# missing, so the weights are not reloaded four times.
#
# Extra-section 10: gptoss gptoss120 coder-next qwen27 qwen38flash gemma
#                   devstral2 ornith laguna21 k2horizon
# Repair missing:    laguna21 llama33 qwen3-30b
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
K2_BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"
mkdir -p "$OUT/failures" "$OUT/failures-perf"

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

has_conc() { [[ -n "$(find "$OUT/concurrency" -name "$1-*.json" 2>/dev/null | head -1)" ]]; }
has_perf() { [[ -n "$(find "$OUT/perf" -name "$1-*.json" 2>/dev/null | head -1)" ]]; }
has_framing() { [[ -n "$(find "$OUT/framing" -name "$1-*.json" 2>/dev/null | head -1)" ]]; }
has_brutal() { [[ -s "$OUT/$1-brutal-c.json" && -s "$OUT/$1-brutal-python.json" && -s "$OUT/$1-brutal-bash.json" ]]; }

server_pid=
stop_server() {
  if [[ -n "${server_pid:-}" ]]; then
    kill "$server_pid" 2>/dev/null || true
    wait "$server_pid" 2>/dev/null || true
    server_pid=
  fi
  kill_port 8082 2>/dev/null
  kill_port 8083 2>/dev/null
  kill_port 8085 2>/dev/null
}
trap 'stop_server' EXIT

serve_mlx() {  # $1=target
  local t="$1" port=8083 server=mlx_lm.server
  local -a extra=()
  [[ "$t" == "ornith" ]] && port=8082
  if [[ "$t" == "qwen27" ]]; then
    server=mlx_vlm.server; extra=(--max-kv-size 65536)
  elif [[ "$t" == "laguna21" || "$t" == "laguna-mlx" ]]; then
    server=mlx_vlm.server
  fi
  [[ "$server" == "mlx_lm.server" ]] && extra+=(--prompt-cache-size 0)
  kill_port "$port"
  "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-extra-$t.log" 2>&1 &
  server_pid=$!
  if ! wait_http "http://127.0.0.1:$port/v1/models" 1800; then
    echo "$t FAILED to serve"; return 1
  fi
  if [[ "$t" == "laguna21" || "$t" == "laguna-mlx" ]]; then
    local reply
    reply="$(curl -sf --max-time 180 "http://127.0.0.1:$port/v1/chat/completions" \
      -H 'Content-Type: application/json' \
      -d "{\"model\":\"$(model_of "$t")\",\"messages\":[{\"role\":\"user\",\"content\":\"say ok\"}],\"max_tokens\":64}")"
    if ! printf '%s' "$reply" | grep -q '"content"[[:space:]]*:[[:space:]]*"[^"]'; then
      echo "$t EMPTY content — refusing to score a runtime bug"; return 1
    fi
  fi
}

serve_fork() {  # $1=target $2=blob $3=extra-args
  kill_port 8085
  # shellcheck disable=SC2086
  /tmp/llama-k2/build/bin/llama-server -m "$2" --alias "$1" \
    --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --parallel 8 --flash-attn on $3 \
    >"/tmp/extra-$1.log" 2>&1 &
  server_pid=$!
  wait_http "http://127.0.0.1:8085/v1/models" 1800
}

ensure_ollama() {
  curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1 \
    || { ollama serve >/tmp/ollama.log 2>&1 & sleep 4; }
}

run_conc() {
  local levels="${2:-1,2,4,8,12,16}"
  has_conc "$1" && { echo "  $1 conc already done"; return 0; }
  echo "  ##### conc $1 ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/bench_concurrent.py" --target "$1" --levels "$levels" --timeout 900
}

run_perf() {
  has_perf "$1" && { echo "  $1 perf already done"; return 0; }
  echo "  ##### perf $1 ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/eval_perf.py" --target "$1" --trials 3 --timeout 600 \
    --dump-failures "$OUT/failures-perf"
}

run_framing() {
  has_framing "$1" && { echo "  $1 framing already done"; return 0; }
  echo "  ##### framing $1 ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/eval_framing.py" --target "$1" --trials 20 --timeout 600
}

run_brutal() {
  has_brutal "$1" && { echo "  $1 brutal already done"; return 0; }
  echo "  ##### brutal $1 ($(date +%H:%M:%S))"
  local lang script f
  for spec in "c:eval_code" "python:eval_python" "bash:eval_bash"; do
    lang="${spec%%:*}"; script="${spec##*:}"
    f="$OUT/$1-brutal-$lang.json"
    [[ -s "$f" ]] && continue
    "$PY" "$ROOT/scripts/$script.py" --target "$1" --set brutal --trials 3 --timeout 900 \
      --dump-failures "$OUT/failures" --json > "$f"
    [[ -s "$f" ]] || rm -f "$f"
  done
}

run_repair() {
  local t="$1" lang f
  for lang in c python bash; do
    if [[ "$lang" == "c" ]]; then f="$OUT/$t-repair.json"
    else f="$OUT/$t-repair-$lang.json"; fi
    [[ -s "$f" ]] && { echo "  $t repair $lang already done"; continue; }
    echo "  ##### repair $t $lang ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_repair.py" --target "$t" --lang "$lang" --set all \
      --max-rounds 5 --timeout 600 --json > "$f"
  done
}

# --- missing extra-section models ---
echo "===== ornith ($(date +%H:%M:%S))"
serve_mlx ornith && {
  run_conc ornith
  run_perf ornith
  run_framing ornith
}
stop_server

echo "===== laguna21 ($(date +%H:%M:%S))"
serve_mlx laguna21 && {
  run_conc laguna21
  run_perf laguna21
  run_framing laguna21
  run_brutal laguna21
  run_repair laguna21
}
stop_server

echo "===== k2horizon ($(date +%H:%M:%S))"
serve_fork k2horizon "$K2_BLOB" "" && {
  run_conc k2horizon 1,2,4,8
  run_perf k2horizon
  run_framing k2horizon
  run_brutal k2horizon
}
stop_server

# --- self-repair for the other new models ---
echo "===== llama33 repair ($(date +%H:%M:%S))"
ensure_ollama
run_repair llama33
ollama stop "$(model_of llama33)" >/dev/null 2>&1 || true

echo "===== qwen3-30b repair ($(date +%H:%M:%S))"
ensure_ollama
run_repair qwen3-30b
ollama stop "$(model_of qwen3-30b)" >/dev/null 2>&1 || true

"$PY" "$ROOT/scripts/make_report.py" || true
echo "EXTRA-TO-10 DONE ($(date +%H:%M:%S))"

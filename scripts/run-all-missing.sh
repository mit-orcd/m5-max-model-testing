#!/usr/bin/env bash
# Fill every bench category for every target except deepseek-32b (sidelined
# for cost). Serves each model once and skips anything already on disk.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
cd "$ROOT"
K2_BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"
LAGUNA_BLOB="$HOME/.ollama/models/blobs/sha256-771a73e1249b9bc08e17d3fca59f5c49b7b9c8a6a47b5a6ac82f95c6e76923c4"
mkdir -p "$OUT/failures" "$OUT/failures-perf" "$OUT/concurrency" "$OUT/perf" "$OUT/framing"

# every target except the sidelined dense reasoner
ALL=(gptoss gptoss120 gemma coder-next qwen27 ornith laguna21 qwen38flash k2horizon
     devstral2 qwen35 qwen36-35b qwen36-27b coder aya glm-flash laguna-mlx laguna
     devstral north ollama
     qwen35-27b katcoder katcoder-reap ling laguna-s qwen35-122b nemotron3
     nex25-mini seed-oss)

# Concurrency needs room for N streams of KV cache on top of the weights. Past
# ~70 GB there isn't any, so the levels are capped rather than measuring swap.
conc_levels_for() {
  case "$1" in
    qwen35-122b|laguna-s) echo "1,2,4" ;;
    nemotron3)            echo "1,2" ;;
    deepseek-v4)          echo "" ;;   # 97 GB of weights: single stream only
    *)                    echo "1,2,4,8,12,16" ;;
  esac
}

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

# timestamped files are <target>-20YY...; the 20 prefix stops `coder` matching coder-next
has_stamp() { [[ -n "$(find "$2" -name "$1-20*.json" 2>/dev/null | head -1)" ]]; }
has_conc() { has_stamp "$1" "$OUT/concurrency"; }
has_perf() { has_stamp "$1" "$OUT/perf"; }
has_brutal() { [[ -s "$OUT/$1-brutal-c.json" && -s "$OUT/$1-brutal-python.json" && -s "$OUT/$1-brutal-bash.json" ]]; }

framing_plan() {
  "$PY" -c '
import json, sys
from pathlib import Path
t = sys.argv[1]
ident = "cs_degree dropout lawyer black african swiss white".split()
files = sorted(Path("results/framing").glob(f"{t}-20*.json"))
if not files:
    print("all"); raise SystemExit
c = (json.loads(files[-1].read_text()).get("conditions") or {})
if "bare" not in c:
    print("all"); raise SystemExit
need = [n for n in ident if n not in c]
print("skip" if not need else "only:" + ",".join(need))
' "$1"
}

needs_anything() {
  local t="$1"
  has_conc "$t" || return 0
  has_perf "$t" || return 0
  has_brutal "$t" || return 0
  [[ -s "$OUT/$t-repair.json" && -s "$OUT/$t-repair-python.json" && -s "$OUT/$t-repair-bash.json" ]] || return 0
  [[ "$(framing_plan "$t")" == "skip" ]] || return 0
  return 1
}

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

serve_mlx() {
  local t="$1" port=8083 server=mlx_lm.server
  local -a extra=()
  [[ "$t" == "ornith" ]] && port=8082
  if [[ "$t" == "qwen27" ]]; then
    server=mlx_vlm.server; extra=(--max-kv-size 65536)
  elif [[ "$t" == "laguna21" || "$t" == "laguna-mlx" || "$t" == "laguna-s" ]]; then
    server=mlx_vlm.server
  elif [[ "$t" == "qwen35-122b" ]]; then
    # the 122B ships a vision tower, so mlx-lm won't load it
    server=mlx_vlm.server
  fi
  # A 97 GB model needs macOS to let the GPU wire more than its default share.
  if [[ "$t" == "deepseek-v4" || "$t" == "nemotron3" ]]; then
    sudo -n sysctl -w iogpu.wired_limit_mb=122880 >/dev/null 2>&1 \
      || echo "  note: could not raise iogpu.wired_limit_mb — $t may swap"
  fi
  [[ "$server" == "mlx_lm.server" ]] && extra+=(--prompt-cache-size 0)
  kill_port "$port"
  "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-all-$t.log" 2>&1 &
  server_pid=$!
  if ! wait_http "http://127.0.0.1:$port/v1/models" 1800 "$server_pid"; then
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
    >"/tmp/all-$1.log" 2>&1 &
  server_pid=$!
  wait_http "http://127.0.0.1:8085/v1/models" 1800 "$server_pid"
}

ensure_laguna_template() {
  [[ -s /tmp/laguna-template.jinja ]] && return 0
  curl -sf "https://huggingface.co/mlx-community/Laguna-XS.2-4bit/raw/main/chat_template.jinja" \
    -o /tmp/laguna-template.jinja
}

ensure_ollama() {  # $1=parallel or empty
  local par="${1:-}"
  if [[ -n "$par" ]]; then
    pkill -f "ollama serve" 2>/dev/null || true
    sleep 2
    OLLAMA_NUM_PARALLEL="$par" ollama serve >/tmp/ollama-all.log 2>&1 &
    sleep 5
  fi
  curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1 \
    || { ollama serve >/tmp/ollama-all.log 2>&1 & sleep 4; }
}

run_conc() {
  local levels="${2:-1,2,4,8,12,16}"
  has_conc "$1" && { echo "  $1 conc already done"; return 0; }
  if [[ -z "$levels" ]]; then
    echo "  $1 conc skipped (weights leave no room for parallel KV cache)"; return 0
  fi
  echo "  ##### conc $1 levels=$levels ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/bench_concurrent.py" --target "$1" --levels "$levels" --timeout 900
}

run_perf() {
  has_perf "$1" && { echo "  $1 perf already done"; return 0; }
  echo "  ##### perf $1 ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/eval_perf.py" --target "$1" --trials 3 --timeout 600 \
    --dump-failures "$OUT/failures-perf"
}

run_framing() {
  local plan only
  plan="$(framing_plan "$1")"
  case "$plan" in
    skip) echo "  $1 framing already done"; return 0 ;;
    all)
      echo "  ##### framing $1 all 14 ($(date +%H:%M:%S))"
      "$PY" "$ROOT/scripts/eval_framing.py" --target "$1" --trials 20 --timeout 600 ;;
    only:*)
      only="${plan#only:}"
      echo "  ##### framing $1 identity ($only) ($(date +%H:%M:%S))"
      # shellcheck disable=SC2086
      "$PY" "$ROOT/scripts/eval_framing.py" --target "$1" --trials 20 --timeout 600 --only ${only//,/ } ;;
  esac
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

serve_and_run() {
  local t="$1"
  needs_anything "$t" || { echo "===== $t nothing missing"; return 0; }
  echo "===== $t ($(date +%H:%M:%S))"
  case "$t" in
    k2horizon)
      serve_fork "$t" "$K2_BLOB" "" || { echo "  $t FAILED to serve"; return 1; }
      run_conc "$t" 1,2,4,8
      run_perf "$t"; run_framing "$t"; run_brutal "$t"; run_repair "$t"
      stop_server ;;
    laguna)
      ensure_laguna_template
      serve_fork "$t" "$LAGUNA_BLOB" "--chat-template-file /tmp/laguna-template.jinja" \
        || { echo "  $t FAILED to serve"; return 1; }
      run_conc "$t" 1,2,4,8
      run_perf "$t"; run_framing "$t"; run_brutal "$t"; run_repair "$t"
      stop_server ;;
    qwen38flash)
      local shard
      shard=$(find "$HOME/.cache/huggingface/hub/models--unsloth--Qwen3.8-Flash-Next-GGUF/snapshots" \
        -name "*UD-Q4_K_XL*00001*" 2>/dev/null | head -1)
      [[ -n "$shard" ]] || { echo "  $t no GGUF shard"; return 1; }
      serve_fork "$t" "$shard" "" || { echo "  $t FAILED to serve"; return 1; }
      run_conc "$t" 1,2,4,8
      run_perf "$t"; run_framing "$t"; run_brutal "$t"; run_repair "$t"
      stop_server ;;
    north|ollama|llama33|qwen3-30b)
      if has_conc "$t"; then ensure_ollama
      else ensure_ollama 8; fi
      run_conc "$t" 1,2,4,8
      run_perf "$t"; run_framing "$t"; run_brutal "$t"; run_repair "$t"
      ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true ;;
    *)
      serve_mlx "$t" || { echo "  $t FAILED to serve"; return 1; }
      run_conc "$t" "$(conc_levels_for "$t")"
      run_perf "$t"; run_framing "$t"; run_brutal "$t"; run_repair "$t"
      stop_server ;;
  esac
  "$PY" "$ROOT/scripts/make_report.py" || true
}

for t in "${ALL[@]}"; do
  serve_and_run "$t" || echo "  $t had a failure, continuing"
done

"$PY" "$ROOT/scripts/make_report.py" || true
"$PY" "$ROOT/scripts/make_csv.py" || true
echo "ALL-MISSING DONE ($(date +%H:%M:%S))"

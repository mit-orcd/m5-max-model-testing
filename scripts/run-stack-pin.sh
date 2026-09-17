#!/usr/bin/env bash
# Qwen3.8-27B stack pin on this Mac.
# Same suite on MLX 4-bit, llama.cpp Metal (bartowski Q4_K_M), and Ollama
# created from that GGUF. Does not overwrite ranking JSON.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results/stack"
OLLAMA_TAG="qwen3.8-27b-q4"
LEVELS="1,2,4,8"
cd "$ROOT"
mkdir -p "$OUT/failures" "$ROOT/results/concurrency"

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

gguf_path() {
  ls -1 "$HOME/models"/Qwen3.8-27B*Q4_K_M*.gguf 2>/dev/null | head -1
}

wait_gguf() {
  local p n=0
  p="$(gguf_path)"
  [[ -n "$p" ]] && { echo "$p"; return 0; }
  echo "waiting for bartowski Q4_K_M GGUF in ~/models ..."
  while (( n < 7200 )); do
    sleep 15
    n=$((n + 15))
    p="$(gguf_path)"
    [[ -n "$p" ]] && { echo "$p"; return 0; }
  done
  echo "timed out waiting for Qwen3.8-27B Q4_K_M GGUF" >&2
  return 1
}

ensure_ollama() {
  if curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null; then
    return 0
  fi
  nohup ollama serve >/tmp/stack-ollama.log 2>&1 &
  wait_http "http://127.0.0.1:11434/api/tags" 60
}

# Ollama create copies the GGUF then llama-quantize COPY (~16 GB more).
# Hard-link the HF blob under the sha256 name so the copy is free.
link_ollama_blob() {
  local src dest
  src="$(readlink -f "$HOME/models/Qwen3.8-27B-Q4_K_M.gguf" 2>/dev/null || true)"
  [[ -n "$src" ]] || src="$(python3 -c "import os; print(os.path.realpath('$HOME/models/Qwen3.8-27B-Q4_K_M.gguf'))")"
  dest="$HOME/.ollama/models/blobs/sha256-880a52af8317974f7b4b064a6bc29ff49e188b91aa218e08eea3078b6b1cc673"
  mkdir -p "$HOME/.ollama/models/blobs"
  if [[ ! -e "$dest" ]]; then
    ln "$src" "$dest" 2>/dev/null || ln -s "$src" "$dest"
  fi
}

ensure_ollama_tag() {
  local gguf="$1" mf
  ensure_ollama
  if ollama show "$OLLAMA_TAG" >/dev/null 2>&1; then
    return 0
  fi
  link_ollama_blob
  mf="$(mktemp)"
  printf 'FROM %s\nPARAMETER num_ctx 32768\n' "$gguf" > "$mf"
  ollama create "$OLLAMA_TAG" -f "$mf"
  rm -f "$mf"
  ollama show "$OLLAMA_TAG" >/dev/null
}

stop_gpu_servers() {
  kill_port 8080; kill_port 8082; kill_port 8083; kill_port 8084; kill_port 8085
  ollama stop --all >/dev/null 2>&1 || true
}

serve_target() {
  local t="$1" rt port
  rt="$("$PY" "$ROOT/scripts/serve.py" field "$t" runtime)"
  stop_gpu_servers
  if [[ "$rt" == "ollama" ]]; then
    pkill -f "ollama serve" 2>/dev/null || true
    sleep 2
    # 8×16k slots — enough for prefill; Ollama's VRAM default (256k) needs 16 GB KV.
    OLLAMA_NUM_PARALLEL=8 OLLAMA_CONTEXT_LENGTH=131072 \
      nohup ollama serve >/tmp/stack-ollama.log 2>&1 &
    wait_http "http://127.0.0.1:11434/api/tags" 60
    return 0
  fi
  port="$("$PY" "$ROOT/scripts/serve.py" field "$t" port)"
  nohup "$PY" "$ROOT/scripts/serve.py" exec "$t" >"/tmp/stack-$t.log" 2>&1 &
  wait_http "http://127.0.0.1:$port/v1/models" 1800 $!
}

run_suite() {
  local t="$1" rt port
  echo "##### stack $t ($(date +%H:%M:%S))"
  if ! serve_target "$t"; then
    echo "  $t FAILED to serve"
    return 1
  fi
  echo "  speed $t ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/bench.py" --target "$t" --case both --trials 5 --json \
    > "$OUT/$t-speed.json" || echo "  $t speed FAILED"
  for i in 1 2 3; do
    echo "  conc $t ladder $i ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/bench_concurrent.py" --target "$t" \
      --levels "$LEVELS" --timeout 900 || echo "  $t conc $i FAILED"
  done
  echo "  ceval $t temp0 ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --set easy \
    --all-temp0 --json --dump-failures "$OUT/failures" \
    > "$OUT/$t-ceval.json" || echo "  $t ceval FAILED"
  rt="$("$PY" "$ROOT/scripts/serve.py" field "$t" runtime)"
  if [[ "$rt" == "ollama" ]]; then
    ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true
  else
    port="$("$PY" "$ROOT/scripts/serve.py" field "$t" port)"
    kill_port "$port"
  fi
  echo "  $t done ($(date +%H:%M:%S))"
}

suite_done() {
  grep -q '"case": "decode"' "$OUT/$1-speed.json" 2>/dev/null \
    && grep -q '"all_temp0": true' "$OUT/$1-ceval.json" 2>/dev/null
}

# MLX 4-bit first — no GGUF needed.
if suite_done qwen27; then
  echo "skip qwen27 — stack results already on disk"
else
  run_suite qwen27
fi

GGUF="$(wait_gguf)" || exit 1
ensure_ollama_tag "$GGUF"
if suite_done qwen27-llamacpp; then
  echo "skip qwen27-llamacpp — stack results already on disk"
else
  run_suite qwen27-llamacpp
fi
if suite_done qwen27-ollama; then
  echo "skip qwen27-ollama — stack results already on disk"
else
  run_suite qwen27-ollama
fi
echo "stack pin done ($(date +%H:%M:%S))"

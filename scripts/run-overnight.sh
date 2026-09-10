#!/usr/bin/env bash
# Overnight final pass (self-contained, sequential):
#   1. wait for any in-flight eval pipeline to drain
#   2. wait until all 3 GGUF models are present in ollama
#   3. K2 Horizon: serve via IFM fork llama-server, run full suite, stop
#   4. north + laguna + qwen38flash full suites (overwrites any partial files)
#   5. regenerate report, commit
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"
mkdir -p "$OUT/failures"

echo "== waiting for in-flight evals to drain ($(date +%H:%M:%S))"
while pgrep -f "run-gguf-evals.sh" >/dev/null 2>&1; do sleep 120; done

echo "== waiting for GGUF downloads ($(date +%H:%M:%S))"
until ollama list 2>/dev/null | grep -q "North-Mini-Code" \
   && ollama list 2>/dev/null | grep -q "Laguna"; do
  sleep 180
done
echo "== ollama models present ($(date +%H:%M:%S))"

# --- K2 Horizon via fork llama-server ---
echo "== k2horizon ($(date +%H:%M:%S))"
nohup /tmp/llama-k2/build/bin/llama-server -m "$BLOB" --alias k2horizon \
  --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on \
  > /tmp/k2-server-final.log 2>&1 &
SRV=$!
for i in $(seq 1 60); do
  curl -sf --max-time 2 http://127.0.0.1:8085/v1/models >/dev/null 2>&1 && break
  sleep 5
done
"$PY" "$ROOT/scripts/bench.py" --target k2horizon --json > "$OUT/k2horizon-speed.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/bench.py" --target k2horizon --case quality --json > "$OUT/k2horizon-quality.json" 2>/dev/null || true
for lang in code python bash; do
  for set in easy hard; do
    case "$lang-$set" in
      code-easy) suffix=ceval;; code-hard) suffix=chard;;
      python-easy) suffix=python;; python-hard) suffix=pyhard;;
      bash-easy) suffix=bash;; bash-hard) suffix=shhard;;
    esac
    "$PY" "$ROOT/scripts/eval_$lang.py" --target k2horizon --trials 3 --set "$set" \
      --timeout 600 --json --dump-failures "$OUT/failures" > "$OUT/k2horizon-$suffix.json" 2>/dev/null || true
  done
done
"$PY" "$ROOT/scripts/eval_research.py" --target k2horizon --trials 3 --timeout 600 --json \
  --dump-failures "$OUT/failures" > "$OUT/k2horizon-research.json" 2>/dev/null || true
kill $SRV 2>/dev/null
sleep 5

# --- north via ollama ---
"$ROOT/scripts/run-gguf-evals.sh" north

# --- laguna via fork llama-server (GGUF template uses Jinja includes that
#     ollama/minja reject; override with the MLX repo's self-contained template) ---
echo "== laguna ($(date +%H:%M:%S))"
LAGUNA_BLOB="$HOME/.ollama/models/blobs/sha256-771a73e1249b9bc08e17d3fca59f5c49b7b9c8a6a47b5a6ac82f95c6e76923c4"
curl -sf "https://huggingface.co/mlx-community/Laguna-XS.2-4bit/raw/main/chat_template.jinja" \
  -o /tmp/laguna-template.jinja
nohup /tmp/llama-k2/build/bin/llama-server -m "$LAGUNA_BLOB" --alias laguna \
  --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on \
  --chat-template-file /tmp/laguna-template.jinja > /tmp/laguna-server.log 2>&1 &
SRV=$!
for i in $(seq 1 120); do
  curl -sf --max-time 2 http://127.0.0.1:8085/v1/models >/dev/null 2>&1 && break
  sleep 5
done
"$PY" "$ROOT/scripts/bench.py" --target laguna --json > "$OUT/laguna-speed.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/bench.py" --target laguna --case quality --json > "$OUT/laguna-quality.json" 2>/dev/null || true
for lang in code python bash; do
  for set in easy hard; do
    case "$lang-$set" in
      code-easy) suffix=ceval;; code-hard) suffix=chard;;
      python-easy) suffix=python;; python-hard) suffix=pyhard;;
      bash-easy) suffix=bash;; bash-hard) suffix=shhard;;
    esac
    "$PY" "$ROOT/scripts/eval_$lang.py" --target laguna --trials 3 --set "$set" \
      --timeout 600 --json --dump-failures "$OUT/failures" > "$OUT/laguna-$suffix.json" 2>/dev/null || true
  done
done
"$PY" "$ROOT/scripts/eval_research.py" --target laguna --trials 3 --timeout 600 --json \
  --dump-failures "$OUT/failures" > "$OUT/laguna-research.json" 2>/dev/null || true
kill $SRV 2>/dev/null
sleep 5

# --- Qwen3.8-Flash-Next via fork llama-server (sharded GGUF; ollama can't pull) ---
echo "== qwen38flash ($(date +%H:%M:%S))"
"$ROOT/.venv/bin/hf" download unsloth/Qwen3.8-Flash-Next-GGUF \
  --include "UD-Q4_K_XL/*" --quiet   # idempotent; blocks until shards complete
SHARD1=$(find "$HOME/.cache/huggingface/hub/models--unsloth--Qwen3.8-Flash-Next-GGUF/snapshots" \
  -name "*UD-Q4_K_XL*00001*" | head -1)
if [[ -n "$SHARD1" ]]; then
  nohup /tmp/llama-k2/build/bin/llama-server -m "$SHARD1" --alias qwen38flash \
    --host 127.0.0.1 --port 8085 -ngl 99 -c 32768 --flash-attn on \
    > /tmp/flash-server.log 2>&1 &
  SRV=$!
  for i in $(seq 1 120); do
    curl -sf --max-time 2 http://127.0.0.1:8085/v1/models >/dev/null 2>&1 && break
    sleep 5
  done
  "$PY" "$ROOT/scripts/bench.py" --target qwen38flash --json > "$OUT/qwen38flash-speed.json" 2>/dev/null || true
  "$PY" "$ROOT/scripts/bench.py" --target qwen38flash --case quality --json > "$OUT/qwen38flash-quality.json" 2>/dev/null || true
  for lang in code python bash; do
    for set in easy hard; do
      case "$lang-$set" in
        code-easy) suffix=ceval;; code-hard) suffix=chard;;
        python-easy) suffix=python;; python-hard) suffix=pyhard;;
        bash-easy) suffix=bash;; bash-hard) suffix=shhard;;
      esac
      "$PY" "$ROOT/scripts/eval_$lang.py" --target qwen38flash --trials 3 --set "$set" \
        --timeout 600 --json --dump-failures "$OUT/failures" > "$OUT/qwen38flash-$suffix.json" 2>/dev/null || true
    done
  done
  "$PY" "$ROOT/scripts/eval_research.py" --target qwen38flash --trials 3 --timeout 600 --json \
    --dump-failures "$OUT/failures" > "$OUT/qwen38flash-research.json" 2>/dev/null || true
  kill $SRV 2>/dev/null
else
  echo "  qwen38flash SKIPPED: shards not found"
fi

# --- report + commit ---
"$PY" "$ROOT/scripts/make_report.py" || true
cd "$ROOT"
git add -A
git commit -q -m "Overnight: K2 Horizon, North, Laguna, Qwen3.8-Flash full results + report" || true
echo "OVERNIGHT-DONE ($(date +%H:%M:%S))"

#!/usr/bin/env bash
# Final overnight stage: re-run K2 Horizon suite (its first run died with the
# server), then regenerate the report and commit everything.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
BLOB="$HOME/.ollama/models/blobs/sha256-513dd78590ac29135a7cea5a99865d57d65291b1f857a8904fb9b1878d4f4cbd"

# serve K2 via the IFM fork (mainline llama.cpp/ollama lack the k2-horizon arch)
nohup /tmp/llama-k2/build/bin/llama-server -m "$BLOB" --host 127.0.0.1 --port 8085 \
  -ngl 99 -c 32768 --flash-attn on > /tmp/k2-server.log 2>&1 &
SRV=$!
for i in $(seq 1 60); do
  curl -sf --max-time 2 http://127.0.0.1:8085/v1/models >/dev/null 2>&1 && break
  sleep 5
done

"$PY" "$ROOT/scripts/bench.py" --target k2horizon --json > "$OUT/k2horizon-speed.json" 2>/dev/null || true
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

# report + commit
"$PY" "$ROOT/scripts/make_report.py" || true
cd "$ROOT"
git add -A
git commit -q -m "Add K2 Horizon, North, Laguna, Qwen3.8-Flash results; final overnight report" || true
echo "K2-FINAL-DONE ($(date +%H:%M:%S))"

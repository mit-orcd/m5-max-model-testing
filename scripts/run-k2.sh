#!/usr/bin/env bash
# K2 Horizon MoVA-36B-A4B (Ollama GGUF) — full eval suite.
# No perplexity (Ollama path); speed/quality via bench.py, then all coding + research.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

if ! curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
  nohup ollama serve >/tmp/ollama.log 2>&1 &
  sleep 3
fi

echo "##### k2horizon ($(date +%H:%M:%S))"
"$PY" "$ROOT/scripts/bench.py" --target k2horizon --json \
  > "$OUT/k2horizon-speed.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/bench.py" --target k2horizon --case quality --json \
  > "$OUT/k2horizon-quality.json" 2>/dev/null || true
for lang in code python bash; do
  for set in easy hard; do
    suffix=$([ "$lang" = code ] && { [ "$set" = easy ] && echo ceval || echo chard; } || { [ "$set" = easy ] && echo "$lang" || { [ "$lang" = python ] && echo pyhard || echo shhard; }; })
    "$PY" "$ROOT/scripts/eval_$lang.py" --target k2horizon --trials 3 --set "$set" --json \
      --dump-failures "$OUT/failures" > "$OUT/k2horizon-$suffix.json" 2>/dev/null || true
  done
done
"$PY" "$ROOT/scripts/eval_research.py" --target k2horizon --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/k2horizon-research.json" 2>/dev/null || true
ollama stop hf.co/NANI-Nithin/K2-Horizon-MoVA-36B-A4B-GGUF:Q4_K_M >/dev/null 2>&1 || true
echo "K2 DONE ($(date +%H:%M:%S))"

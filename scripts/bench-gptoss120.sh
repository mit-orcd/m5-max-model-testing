#!/usr/bin/env bash
# Download gpt-oss-120b-MXFP4-Q8 and run the full benchmark battery on it.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
MODEL="mlx-community/gpt-oss-120b-MXFP4-Q8"
mkdir -p "$OUT/failures"

echo "##### download ($(date +%H:%M:%S))"
"$ROOT/.venv/bin/hf" download "$MODEL" || { echo "DOWNLOAD FAILED"; exit 1; }
echo "##### download done ($(date +%H:%M:%S))"

kill_port 8083
export APC_ENABLED=1
nohup "$ROOT/.venv/bin/mlx_lm.server" --model "$MODEL" --host 127.0.0.1 --port 8083 \
  --max-tokens 16384 >/tmp/mlx-gptoss120.log 2>&1 &
if ! wait_http "http://127.0.0.1:8083/v1/models" 900; then
  echo "SERVE FAILED"; tail -20 /tmp/mlx-gptoss120.log; exit 1
fi
echo "##### served ($(date +%H:%M:%S))"

"$PY" "$ROOT/scripts/bench.py" --target gptoss120 --case decode --json > "$OUT/gptoss120-decode.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/bench.py" --target gptoss120 --case prefill --json > "$OUT/gptoss120-prefill.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/bench.py" --target gptoss120 --case quality --json > "$OUT/gptoss120-quality.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_code.py" --target gptoss120 --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/gptoss120-ceval.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_python.py" --target gptoss120 --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/gptoss120-python.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/eval_bash.py" --target gptoss120 --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/gptoss120-bash.json" 2>/dev/null || true

kill_port 8083 2>/dev/null
sleep 3

echo "##### perplexity ($(date +%H:%M:%S))"
"$ROOT/.venv/bin/mlx_lm.perplexity" --model "$MODEL" \
  --data-path /tmp/wikitext --num-samples 50 --seed 0 \
  > "$OUT/gptoss120-wikitext-perplexity.txt" 2>&1 || true

"$PY" "$ROOT/scripts/make_report.py"
echo "GPTOSS120 DONE ($(date +%H:%M:%S))"

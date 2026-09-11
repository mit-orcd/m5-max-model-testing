#!/usr/bin/env bash
# Full suite for Laguna XS 2.1 (33B-A3B), the successor to laguna (XS.2).
#
# Runtime note: stock mlx-lm has no `laguna` architecture (ml-explore/mlx-lm#1223),
# so this is served with mlx_vlm.server — the same runtime qwen27 already uses.
# The GGUF path was rejected: it needs two unmerged llama.cpp PRs (#25165 for the
# architecture, #25389 for a Metal f16 overflow that otherwise returns empty text
# on Apple Silicon).
#
# Laguna is a reasoning model: it thinks before answering, so the harness must
# give it a thinking budget or every answer scores as truncated.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
mkdir -p "$OUT/failures"

t=laguna21
port=8083
model="$("$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$t']['model'])")"

kill_port "$port"
export APC_ENABLED=1
nohup "$ROOT/.venv/bin/mlx_vlm.server" --model "$model" --host 127.0.0.1 --port "$port" \
  --max-tokens 16384 >"/tmp/mlx-$t.log" 2>&1 &

if ! wait_http "http://127.0.0.1:$port/v1/models" 1800; then
  echo "$t FAILED to serve"; exit 1
fi

# Smoke test first: the Metal MoE bug this model class is prone to shows up as a
# 200 response with empty content, which would otherwise score as 126 failures.
reply="$(curl -sf --max-time 180 "http://127.0.0.1:$port/v1/chat/completions" \
  -H 'Content-Type: application/json' \
  -d "{\"model\":\"$model\",\"messages\":[{\"role\":\"user\",\"content\":\"say ok\"}],\"max_tokens\":2048}")"
if ! printf '%s' "$reply" | grep -q '"content"[[:space:]]*:[[:space:]]*"[^"]'; then
  echo "$t returned EMPTY content — refusing to score a runtime bug as model failure"
  printf '%s\n' "$reply" | head -c 600
  kill_port "$port"; exit 1
fi
echo "smoke test OK"

"$PY" "$ROOT/scripts/bench.py" --target "$t" --json > "$OUT/$t-speed.json" 2>/dev/null || true
"$PY" "$ROOT/scripts/bench.py" --target "$t" --case quality --json > "$OUT/$t-quality.json" 2>/dev/null || true
for pair in "eval_code easy ceval" "eval_code hard chard" \
            "eval_python easy python" "eval_python hard pyhard" \
            "eval_bash easy bash" "eval_bash hard shhard"; do
  set -- $pair
  echo "##### $t $1 $2 ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/$1.py" --target "$t" --trials 3 --set "$2" --json \
    --dump-failures "$OUT/failures" > "$OUT/$t-$3.json" 2>/dev/null || true
done
"$PY" "$ROOT/scripts/eval_research.py" --target "$t" --trials 3 --json \
  --dump-failures "$OUT/failures" > "$OUT/$t-research.json" 2>/dev/null || true

kill_port "$port" 2>/dev/null
echo "LAGUNA21_DONE ($(date +%H:%M:%S))"

#!/usr/bin/env bash
# WikiText ppl for MLX models that lack it, seed-oss bash-hard JSON, and a
# fresh concurrency sweep for north/ollama/katcoder/katcoder-reap (old files
# are all http_error). Fork/Ollama GGUF targets have no MLX ppl path.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
cd "$ROOT"
# Model weights stay local; the WikiText loader still needs the hub unless
# --data-path already exists. Do not export HF_HUB_OFFLINE for this script.
export HF_HUB_DISABLE_XET=1

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

json_ok() {
  "$PY" -c '
import json, sys
from pathlib import Path
p = Path(sys.argv[1])
if not p.exists() or p.stat().st_size < 3:
    raise SystemExit(1)
txt = p.read_text()
i = txt.find("[")
if i < 0:
    i = txt.find("{")
if i < 0:
    raise SystemExit(1)
json.loads(txt[i:])
' "$1"
}

wiki_ok() { grep -q "Perplexity:" "$OUT/$1-wikitext-perplexity.txt" 2>/dev/null; }

# --- 1. seed-oss bash-hard ---
if json_ok "$OUT/seed-oss-shhard.json"; then
  echo "===== seed-oss shhard already valid JSON"
else
  echo "===== seed-oss shhard $(date +%H:%M:%S)"
  kill_port 8083
  HF_HUB_OFFLINE=1 HF_HUB_DISABLE_XET=1 \
    "$ROOT/.venv/bin/mlx_lm.server" --model "$(model_of seed-oss)" \
    --host 127.0.0.1 --port 8083 --max-tokens 16384 --prompt-cache-size 0 \
    > /tmp/mlx-seed-oss-shhard.log 2>&1 &
  srv=$!
  if wait_http "http://127.0.0.1:8083/v1/models" 1800 "$srv"; then
    "$PY" "$ROOT/scripts/eval_bash.py" --target seed-oss --set hard --trials 3 \
      --timeout 900 --dump-failures "$OUT/failures" --json \
      > /tmp/seed-oss-shhard.out 2>&1 || true
    "$PY" -c '
from pathlib import Path
txt = Path("/tmp/seed-oss-shhard.out").read_text()
i = txt.find("[")
if i < 0:
    raise SystemExit("no JSON in seed-oss shhard output")
Path("results/seed-oss-shhard.json").write_text(txt[i:])
'
  else
    echo "  seed-oss FAILED to serve"
  fi
  kill_port 8083
  wait "$srv" 2>/dev/null || true
fi

# --- 2. WikiText ppl (MLX only; loads the model itself, GPU must be free) ---
WIKI=(laguna21 laguna-mlx qwen35-27b katcoder katcoder-reap ling laguna-s
      qwen35-122b nemotron3 nex25-mini seed-oss)
for t in "${WIKI[@]}"; do
  wiki_ok "$t" && { echo "===== wikitext $t already done"; continue; }
  echo "===== wikitext $t $(date +%H:%M:%S)"
  if [[ "$t" == "nemotron3" ]]; then
    sudo -n sysctl -w iogpu.wired_limit_mb=122880 >/dev/null 2>&1 || true
  fi
  "$ROOT/scripts/perplexity.sh" "$t" 50 \
    > "$OUT/$t-wikitext-perplexity.txt" 2>&1 || echo "  $t ppl failed"
done

# --- 3. concurrency re-run (timestamped; does not overwrite) ---
echo "===== concurrency north ollama $(date +%H:%M:%S)"
LEVELS=1,2,4,8 SLOTS=8 bash "$ROOT/scripts/run-concurrency.sh" north ollama
echo "===== concurrency katcoder katcoder-reap $(date +%H:%M:%S)"
LEVELS=1,2,4,8,12,16 bash "$ROOT/scripts/run-concurrency.sh" katcoder katcoder-reap

"$PY" "$ROOT/scripts/make_report.py" || true
"$PY" "$ROOT/scripts/make_csv.py" || true
echo "FILL-MAC-GAPS DONE $(date +%H:%M:%S)"

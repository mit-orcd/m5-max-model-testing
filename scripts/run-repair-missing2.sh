#!/usr/bin/env bash
# Re-run the two repair targets that failed: qwen27 (wrapper died before eval)
# and ornith (server killed by port race mid-eval).
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

run_one() {  # $1=target $2=port $3=server $4...=extra
  local t="$1" port="$2" server="$3"; shift 3
  echo "##### $t ($(date +%H:%M:%S))"
  kill_port "$port" 2>/dev/null
  sleep 5
  export APC_ENABLED=1
  nohup "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
    --max-tokens 16384 "$@" >"/tmp/mlx-repair-$t.log" 2>&1 &
  local srv=$!
  if wait_http "http://127.0.0.1:$port/v1/models" 900; then
    # confirm it's OUR server, not a dying predecessor
    kill -0 "$srv" 2>/dev/null || { echo "  $t server died at startup"; return 1; }
    "$PY" "$ROOT/scripts/eval_repair.py" --target "$t" --set all --max-rounds 5 \
      --timeout 600 --json > "$OUT/$t-repair.json" 2>/dev/null || true
  else
    echo "  $t FAILED to serve"
  fi
  kill_port "$port" 2>/dev/null
  sleep 3
  echo "  $t done ($(date +%H:%M:%S))"
}

run_one qwen27 8083 mlx_vlm.server --max-kv-size 65536
run_one ornith 8082 mlx_lm.server

echo "MISSING2 DONE ($(date +%H:%M:%S))"

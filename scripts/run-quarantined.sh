#!/usr/bin/env bash
# Re-run every result that was quarantined for dead-server contamination
# (http_error / connection refused), plus the gaps those runs leave behind.
#
#   ling          brutal x3, repair x3, perf, framing   (all were 100% http_error)
#   deepseek-32b  ceval, bash                           (partially contaminated)
#
# Every produced file is validated: a file whose trials are ALL http_error
# means the server died — the file is moved to results/quarantine/ instead
# of being scored as model failure.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
cd "$ROOT"
mkdir -p "$OUT/quarantine" "$OUT/failures" "$OUT/failures-perf" "$OUT/perf" "$OUT/framing"

server_pid=
stop_server() {
  if [[ -n "${server_pid:-}" ]]; then
    kill "$server_pid" 2>/dev/null || true
    wait "$server_pid" 2>/dev/null || true
    server_pid=
  fi
  kill_port 8083 2>/dev/null
}
trap 'stop_server' EXIT

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

serve_mlx() {
  local t="$1" want got
  want="$(model_of "$t")"
  kill_port 8083
  HF_HUB_OFFLINE=1 HF_HUB_DISABLE_XET=1 \
    "$ROOT/.venv/bin/mlx_lm.server" --model "$want" \
    --host 127.0.0.1 --port 8083 --max-tokens 16384 --prompt-cache-size 0 \
    >"/tmp/rerun-$t.log" 2>&1 &
  server_pid=$!
  wait_http "http://127.0.0.1:8083/v1/models" 1800 "$server_pid" || return 1
  # Identity check: the server must report the model we asked for. mlx-lm
  # ignores the request's model field, so without this a stale server on
  # the port would silently score the wrong weights.
  got="$(curl -sf --max-time 5 http://127.0.0.1:8083/v1/models \
    | "$PY" -c 'import json,sys; print(json.load(sys.stdin)["data"][0]["id"])')"
  if [[ "$got" != "$want" ]]; then
    echo "  $t IDENTITY MISMATCH: wanted $want, server has $got — refusing to run"
    return 1
  fi
  echo "  $t serving verified: $got"
}

# Reject files where every recorded trial is http_error: that is a dead
# server, not a model score.
guard() {
  local f="$1"
  [[ -s "$f" ]] || return 0
  if "$PY" - "$f" <<'EOF'
import json, re, sys
txt = open(sys.argv[1]).read()
m = re.search(r"^\[$", txt, re.M)
d = json.loads(txt[m.start():])[0] if m else json.loads(txt)
outs = []
for o in (d.get("results") or {}).values():
    outs += [x for x in o if isinstance(x, str)]
for v in (d.get("per_task") or {}).values():
    outs += [v.get("first_status"), v.get("last_status")]
sts = [s for v in (d.get("variants") or {}).values() for e in v.values() for s in e.get("statuses", [])]
outs += sts
conds = [o for c in (d.get("conditions") or {}).values() for o in (c.get("outcomes") or [])]
outs += conds
bad = outs and all(o == "http_error" for o in outs)
sys.exit(1 if bad else 0)
EOF
  then
    return 0
  fi
  echo "  GUARD: $f is all http_error — quarantined, not scored"
  mv "$f" "$OUT/quarantine/"
  return 1
}

run() {  # run <outfile> <cmd...>
  local f="$1"; shift
  [[ -s "$f" ]] && { echo "  skip $(basename "$f") (exists)"; return 0; }
  echo "  ##### $(basename "$f" .json) ($(date +%H:%M:%S))"
  "$@" > "$f"
  [[ -s "$f" ]] || { rm -f "$f"; return 1; }
  guard "$f"
}

echo "===== ling ($(date +%H:%M:%S))"
if serve_mlx ling; then
  run "$OUT/ling-brutal-c.json"      "$PY" scripts/eval_code.py   --target ling --set brutal --trials 3 --timeout 900 --dump-failures "$OUT/failures" --json
  run "$OUT/ling-brutal-python.json" "$PY" scripts/eval_python.py --target ling --set brutal --trials 3 --timeout 900 --dump-failures "$OUT/failures" --json
  run "$OUT/ling-brutal-bash.json"   "$PY" scripts/eval_bash.py   --target ling --set brutal --trials 3 --timeout 900 --dump-failures "$OUT/failures" --json
  run "$OUT/ling-repair.json"        "$PY" scripts/eval_repair.py --target ling --lang c      --set all --max-rounds 5 --timeout 600 --json
  run "$OUT/ling-repair-python.json" "$PY" scripts/eval_repair.py --target ling --lang python --set all --max-rounds 5 --timeout 600 --json
  run "$OUT/ling-repair-bash.json"   "$PY" scripts/eval_repair.py --target ling --lang bash   --set all --max-rounds 5 --timeout 600 --json
  echo "  ##### perf ling ($(date +%H:%M:%S))"
  "$PY" scripts/eval_perf.py --target ling --trials 3 --timeout 600 --dump-failures "$OUT/failures-perf"
  echo "  ##### framing ling ($(date +%H:%M:%S))"
  "$PY" scripts/eval_framing.py --target ling --trials 20 --timeout 600
  stop_server
else
  echo "  ling FAILED to serve — see /tmp/rerun-ling.log"
fi

echo "===== deepseek-32b ($(date +%H:%M:%S))"
if serve_mlx deepseek-32b; then
  run "$OUT/deepseek-32b-ceval.json" "$PY" scripts/eval_code.py --target deepseek-32b --set easy --trials 3 --timeout 900 --dump-failures "$OUT/failures" --json
  run "$OUT/deepseek-32b-bash.json"  "$PY" scripts/eval_bash.py --target deepseek-32b --set easy --trials 3 --timeout 900 --dump-failures "$OUT/failures" --json
  stop_server
else
  echo "  deepseek-32b FAILED to serve — see /tmp/rerun-deepseek-32b.log"
fi

echo "===== rebuild reports ($(date +%H:%M:%S))"
"$PY" scripts/make_report.py && "$PY" scripts/make_charts.py
echo "done"

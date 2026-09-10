#!/usr/bin/env bash
# Repair sweep for the remaining targets not covered by run-repair-sweep.sh.
# Run AFTER the qwen27 repair finishes (port 8083 collision otherwise).
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"

ALL=(gemma devstral qwen35 qwen36-27b ornith coder aya glm-flash ollama)

model_of() {
  "$PY" -c "import sys; sys.path.insert(0,'$ROOT/scripts'); from bench import TARGETS; print(TARGETS['$1']['model'])"
}

run_repair() {
  [[ -s "$OUT/$1-repair.json" ]] && { echo "  $1 already done"; return 0; }
  "$PY" "$ROOT/scripts/eval_repair.py" --target "$1" --set all --max-rounds 5 \
    --timeout 600 --json > "$OUT/$1-repair.json" 2>/dev/null || true
}

for t in "${ALL[@]}"; do
  echo "##### $t ($(date +%H:%M:%S))"
  if [[ "$t" == "ollama" ]]; then
    curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1 || { nohup ollama serve >/tmp/ollama.log 2>&1 & sleep 3; }
    run_repair "$t"
    ollama stop "$(model_of "$t")" >/dev/null 2>&1 || true
  else
    port=8083; server=mlx_lm.server; extra=()
    [[ "$t" == "ornith" ]] && port=8082
    kill_port "$port"
    export APC_ENABLED=1
    nohup "$ROOT/.venv/bin/$server" --model "$(model_of "$t")" --host 127.0.0.1 --port "$port" \
      --max-tokens 16384 "${extra[@]}" >"/tmp/mlx-repair-$t.log" 2>&1 &
    if wait_http "http://127.0.0.1:$port/v1/models" 900; then
      run_repair "$t"
    else
      echo "  $t FAILED to serve"
    fi
    kill_port "$port" 2>/dev/null
    sleep 3
  fi
  echo "  $t done ($(date +%H:%M:%S))"
done

"$PY" "$ROOT/scripts/make_report.py" || true

echo "REPAIR-REST DONE ($(date +%H:%M:%S))"

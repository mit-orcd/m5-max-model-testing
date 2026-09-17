#!/usr/bin/env bash
# Fill remaining Linux suites after the speed/quality/C/ppl sweep.
# Serves each LINUX_TARGET once via serve.py and skips anything already on disk.
#
#   PHASE=coding    python + bash (easy), hard sets, research   [default]
#   PHASE=analysis  perf, brutal, repair, concurrency (8-wide, shared coding serve)
#   PHASE=framing   14 wordings × 20 trials
#   PHASE=all       coding + analysis; framing if INCLUDE_FRAMING=1
#   PHASE=conc      re-serve with 16 slots and run 1,2,4,8,12,16 (Mac ladder)
#
# Mac is the suite template: speed 3 trials, quality 3×6, framing 14×20.
# INCLUDE_FRAMING=1 or PHASE=framing. Resume-safe.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=_ports.sh
source "$ROOT/scripts/_ports.sh"
PY="$ROOT/.venv/bin/python"
OUT="$ROOT/results"
cd "$ROOT"
mkdir -p "$OUT/failures" "$OUT/failures-perf" "$OUT/concurrency" "$OUT/perf" "$OUT/framing"
if [[ ! -s "$OUT/machine.json" ]]; then
  if [[ "$(uname -s)" == "Darwin" ]]; then
    cp "$ROOT/scripts/machines/m5-max.json" "$OUT/machine.json"
  elif command -v rocminfo >/dev/null && rocminfo 2>/dev/null | grep -q gfx1151; then
    cp "$ROOT/scripts/machines/strix-halo.json" "$OUT/machine.json"
  else
    cp "$ROOT/scripts/machines/rtx-pro-6000.json" "$OUT/machine.json"
  fi
fi

PHASE="${PHASE:-coding}"
INCLUDE_FRAMING="${INCLUDE_FRAMING:-0}"

LINUX_TARGETS=(gptoss gptoss-vllm gptoss120 qwen27 qwen27-vllm qwen27-sglang qwen35 qwen35-vllm \
  coder gemma devstral aya qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b qwen35-122b \
  qwen35-27b nemotron3 seed-oss laguna-s qwen38flash k2horizon laguna ollama)
if [[ -n "${SWEEP_ONLY:-}" ]]; then
  # shellcheck disable=SC2206
  LINUX_TARGETS=($SWEEP_ONLY)
fi

# Mac ran the expensive suites on deepseek-32b; keep Linux matched.
ANALYSIS_SKIP=()

# Coding serve keeps a large n_ctx split across few slots, so PHASE=all/analysis
# stops at 8-wide. PHASE=conc re-serves 32k/16 (Mac llama-server --parallel 16);
# prompts are ~200 tokens. 100B+ weights leave too little KV for 12/16; aya's
# train ctx is 8k.
conc_levels_for() {
  if [[ "${PHASE:-}" != "conc" ]]; then
    case "$1" in
      qwen35-122b|laguna-s) echo "1,2,4" ;;
      qwen38flash|nemotron3|k2horizon|aya) echo "1,2,4,8" ;;
      qwen27-vllm|qwen35-vllm|qwen27-sglang) echo "1,2,4" ;;
      *) echo "1,2,4,8" ;;
    esac
    return
  fi
  case "$1" in
    qwen35-122b|laguna-s) echo "1,2,4" ;;
    qwen38flash|nemotron3|k2horizon|aya) echo "1,2,4,8" ;;
    *) echo "1,2,4,8,12,16" ;;
  esac
}

serve_env_for() {
  unset LLAMA_PARALLEL LLAMA_CTX
  # Strix: 128 GB LPDDR5, BIOS 64 GB VRAM + ~31 GB GTT. Modest KV so analysis
  # and 8-wide conc fit; do not use the RTX 8×128k profile.
  if [[ "${LLAMA_SERVE_PROFILE:-}" == "strix" ]]; then
    case "$1" in
      qwen35-122b|laguna-s|nemotron3|gptoss120|qwen38flash)
        export LLAMA_PARALLEL=2
        export LLAMA_CTX=8192
        ;;
      aya)
        export LLAMA_PARALLEL=1
        export LLAMA_CTX=8192
        ;;
      *)
        export LLAMA_PARALLEL=8
        export LLAMA_CTX=16384
        ;;
    esac
    return
  fi
  case "$1" in
    qwen35-122b|qwen38flash|nemotron3|laguna-s)
      export LLAMA_PARALLEL=4
      export LLAMA_CTX=65536
      ;;
    aya)
      # n_ctx_train=8192; --parallel 8 × 16k KV OOMs (~160 GB).
      export LLAMA_PARALLEL=1
      export LLAMA_CTX=8192
      ;;
    *-vllm|*-sglang) ;;
    *)
      export LLAMA_PARALLEL=8
      export LLAMA_CTX=131072
      ;;
  esac
}

serve_env_conc_for() {
  unset LLAMA_PARALLEL LLAMA_CTX
  case "$1" in
    qwen35-122b|laguna-s)
      export LLAMA_PARALLEL=4
      export LLAMA_CTX=65536
      ;;
    qwen38flash|nemotron3|k2horizon)
      export LLAMA_PARALLEL=8
      export LLAMA_CTX=32768
      ;;
    aya)
      export LLAMA_PARALLEL=8
      export LLAMA_CTX=8192
      ;;
    *-vllm|*-sglang) ;;
    *)
      export LLAMA_PARALLEL=16
      export LLAMA_CTX=32768
      ;;
  esac
}

in_list() {
  local needle="$1"; shift
  local x
  for x in "$@"; do [[ "$x" == "$needle" ]] && return 0; done
  return 1
}

has_stamp() { [[ -n "$(find "$2" -name "$1-20*.json" 2>/dev/null | head -1)" ]]; }
json_ok() {
  "$PY" -c '
import json, sys
from pathlib import Path
p = Path(sys.argv[1])
if not p.is_file() or p.stat().st_size < 20:
    raise SystemExit(1)
s = p.read_text(errors="replace")
for i in range(len(s) - 1, -1, -1):
    if s[i] not in "[{":
        continue
    try:
        d = json.loads(s[i:])
    except json.JSONDecodeError:
        continue
    if isinstance(d, list):
        d = d[0] if d else {}
    raise SystemExit(0 if isinstance(d, dict) and "target" in d else 1)
raise SystemExit(1)
' "$1"
}
json_load() {  # print one key from the last JSON value
  "$PY" -c '
import json, sys
from pathlib import Path
s = Path(sys.argv[1]).read_text(errors="replace")
key = sys.argv[2]
for i in range(len(s) - 1, -1, -1):
    if s[i] not in "[{":
        continue
    try:
        d = json.loads(s[i:])
    except json.JSONDecodeError:
        continue
    if isinstance(d, list):
        dec = [x for x in d if isinstance(x, dict) and x.get("case") == "decode"]
        d = dec[0] if dec else (d[0] if d else {})
    print(d.get(key, "") if isinstance(d, dict) else "")
    raise SystemExit
print("")
' "$1" "$2"
}
has_speed() {
  local n
  json_ok "$OUT/$1-speed.json" || return 1
  n="$(json_load "$OUT/$1-speed.json" trials)"
  [[ "$n" == "3" || "$n" == "3.0" ]]
}
has_quality() {
  local n tot
  json_ok "$OUT/$1-quality.json" || return 1
  n="$(json_load "$OUT/$1-quality.json" trials)"
  tot="$(json_load "$OUT/$1-quality.json" total)"
  [[ "$n" == "3" || "$tot" == "18" ]]
}
has_conc() { has_stamp "$1" "$OUT/concurrency"; }
has_conc_levels() {
  local t="$1" want
  want="$(conc_levels_for "$t")"
  [[ -z "$want" ]] && return 0
  "$PY" -c '
import json, sys
from pathlib import Path
t, want = sys.argv[1], {int(x) for x in sys.argv[2].split(",") if x}
files = sorted(Path("results/concurrency").glob(f"{t}-20*.json"))
if not files:
    raise SystemExit(1)
have = {int(l["level"]) for l in json.loads(files[-1].read_text()).get("levels") or []}
raise SystemExit(0 if want <= have else 1)
' "$t" "$want"
}
has_perf() { has_stamp "$1" "$OUT/perf"; }
has_brutal() { [[ -s "$OUT/$1-brutal-c.json" && -s "$OUT/$1-brutal-python.json" && -s "$OUT/$1-brutal-bash.json" ]]; }
has_repair() {
  json_ok "$OUT/$1-repair.json" && json_ok "$OUT/$1-repair-python.json" \
    && json_ok "$OUT/$1-repair-bash.json"
}
has_coding() {
  has_speed "$1" && has_quality "$1" \
    && json_ok "$OUT/$1-ceval.json" \
    && json_ok "$OUT/$1-python.json" && json_ok "$OUT/$1-bash.json" \
    && json_ok "$OUT/$1-pyhard.json" && json_ok "$OUT/$1-shhard.json" \
    && json_ok "$OUT/$1-research.json"
}
has_chard() { [[ -s "$OUT/$1-chard.json" ]]; }

framing_plan() {
  "$PY" -c '
import json, sys
from pathlib import Path
t = sys.argv[1]
ident = "cs_degree dropout lawyer black african swiss white".split()
files = sorted(Path("results/framing").glob(f"{t}-20*.json"))
if not files:
    print("all"); raise SystemExit
c = (json.loads(files[-1].read_text()).get("conditions") or {})
if "bare" not in c:
    print("all"); raise SystemExit
need = [n for n in ident if n not in c]
print("skip" if not need else "only:" + ",".join(need))
' "$1"
}

needs_coding() {
  local t="$1"
  has_coding "$t" || return 0
  has_chard "$t" || return 0
  return 1
}

needs_analysis() {
  local t="$1"
  in_list "$t" "${ANALYSIS_SKIP[@]}" && return 1
  has_conc "$t" || return 0
  has_perf "$t" || return 0
  has_brutal "$t" || return 0
  has_repair "$t" || return 0
  return 1
}

needs_framing() {
  local t="$1"
  in_list "$t" "${ANALYSIS_SKIP[@]}" && return 1
  [[ "$(framing_plan "$t")" == "skip" ]] && return 1
  return 0
}

needs_conc() {
  local t="$1"
  in_list "$t" "${ANALYSIS_SKIP[@]}" && return 1
  has_conc_levels "$t" && return 1
  return 0
}

needs_anything() {
  local t="$1"
  case "$PHASE" in
    coding) needs_coding "$t" ;;
    analysis) needs_analysis "$t" ;;
    framing) needs_framing "$t" ;;
    conc) needs_conc "$t" ;;
    all)
      needs_coding "$t" && return 0
      needs_analysis "$t" && return 0
      [[ "$INCLUDE_FRAMING" == "1" ]] && needs_framing "$t" && return 0
      return 1
      ;;
    *) echo "unknown PHASE=$PHASE" >&2; return 1 ;;
  esac
}

ensure_ollama() {
  local par="${1:-}"
  export OLLAMA_MODELS="${OLLAMA_MODELS:-/home/root/ollama-models}"
  mkdir -p "$OLLAMA_MODELS"
  if [[ -n "$par" ]]; then
    kill_port 11434
    sleep 1
    OLLAMA_NUM_PARALLEL="$par" nohup ollama serve >/tmp/serve-miss-ollama.log 2>&1 &
    sleep 3
  fi
  if curl -sf --max-time 2 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
    return 0
  fi
  nohup ollama serve >/tmp/serve-miss-ollama.log 2>&1 &
  wait_http "http://127.0.0.1:11434/api/tags" 60
}

serve() {
  local t="$1" port rt
  rt="$("$PY" "$ROOT/scripts/serve.py" field "$t" runtime)"
  if [[ "$rt" == "ollama" ]]; then
    kill_port 8083; kill_port 8082; kill_port 8085
    if [[ "$PHASE" == "conc" ]]; then ensure_ollama 16; else ensure_ollama 8; fi
    return 0
  fi
  kill_port 11434
  port="$("$PY" "$ROOT/scripts/serve.py" field "$t" port)"
  kill_port "$port"
  if [[ "$PHASE" == "conc" ]]; then
    serve_env_conc_for "$t"
  else
    serve_env_for "$t"
  fi
  nohup "$PY" "$ROOT/scripts/serve.py" exec "$t" >"/tmp/serve-miss-$t.log" 2>&1 &
  wait_http "http://127.0.0.1:$port/v1/models" 900
}

run_coding() {
  local t="$1"
  has_speed "$t" || {
    echo "  ##### speed $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --case both --trials 3 --json > "$OUT/$t-speed.json" || true
    json_ok "$OUT/$t-speed.json" || rm -f "$OUT/$t-speed.json"
  }
  has_quality "$t" || {
    echo "  ##### quality $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/bench.py" --target "$t" --case quality --trials 3 --json > "$OUT/$t-quality.json" || true
    json_ok "$OUT/$t-quality.json" || rm -f "$OUT/$t-quality.json"
  }
  [[ -s "$OUT/$t-ceval.json" ]] || {
    echo "  ##### ceval $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-ceval.json" || true
    [[ -s "$OUT/$t-ceval.json" ]] || rm -f "$OUT/$t-ceval.json"
  }
  has_chard "$t" || {
    echo "  ##### chard $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_code.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-chard.json" || true
  }
  [[ -s "$OUT/$t-python.json" ]] || {
    echo "  ##### python $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_python.py" --target "$t" --trials 3 --set easy --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-python.json" || true
  }
  [[ -s "$OUT/$t-bash.json" ]] || {
    echo "  ##### bash $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_bash.py" --target "$t" --trials 3 --set easy --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-bash.json" || true
  }
  [[ -s "$OUT/$t-pyhard.json" ]] || {
    echo "  ##### pyhard $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_python.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-pyhard.json" || true
  }
  [[ -s "$OUT/$t-shhard.json" ]] || {
    echo "  ##### shhard $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_bash.py" --target "$t" --trials 3 --set hard --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-shhard.json" || true
  }
  [[ -s "$OUT/$t-research.json" ]] || {
    echo "  ##### research $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_research.py" --target "$t" --trials 3 --json \
      --dump-failures "$OUT/failures" > "$OUT/$t-research.json" || true
  }
}

run_analysis() {
  local t="$1"
  in_list "$t" "${ANALYSIS_SKIP[@]}" && { echo "  $t analysis skipped (cost)"; return 0; }
  has_perf "$t" || {
    echo "  ##### perf $t ($(date +%H:%M:%S))"
    "$PY" "$ROOT/scripts/eval_perf.py" --target "$t" --trials 3 --timeout 600 \
      --dump-failures "$OUT/failures-perf" || true
  }
  has_brutal "$t" || {
    echo "  ##### brutal $t ($(date +%H:%M:%S))"
    local lang script f
    for spec in "c:eval_code" "python:eval_python" "bash:eval_bash"; do
      lang="${spec%%:*}"; script="${spec##*:}"
      f="$OUT/$t-brutal-$lang.json"
      [[ -s "$f" ]] && continue
      "$PY" "$ROOT/scripts/$script.py" --target "$t" --set brutal --trials 3 --timeout 900 \
        --dump-failures "$OUT/failures" --json > "$f" || true
      [[ -s "$f" ]] || rm -f "$f"
    done
  }
  has_repair "$t" || {
    local lang f
    for lang in c python bash; do
      if [[ "$lang" == "c" ]]; then f="$OUT/$t-repair.json"
      else f="$OUT/$t-repair-$lang.json"; fi
      json_ok "$f" && continue
      echo "  ##### repair $t $lang ($(date +%H:%M:%S))"
      "$PY" "$ROOT/scripts/eval_repair.py" --target "$t" --lang "$lang" --set all \
        --max-rounds 5 --timeout 600 --json > "$f" || true
    done
  }
  has_conc "$t" || run_conc "$t"
}

run_conc() {
  local t="$1" levels
  levels="$(conc_levels_for "$t")"
  [[ -z "$levels" ]] && return 0
  echo "  ##### conc $t levels=$levels parallel=${LLAMA_PARALLEL:-default} ctx=${LLAMA_CTX:-default} ($(date +%H:%M:%S))"
  "$PY" "$ROOT/scripts/bench_concurrent.py" --target "$t" --levels "$levels" --timeout 900 || true
}

run_framing() {
  local t="$1" plan only
  in_list "$t" "${ANALYSIS_SKIP[@]}" && return 0
  plan="$(framing_plan "$t")"
  case "$plan" in
    skip) echo "  $t framing already done" ;;
    all)
      echo "  ##### framing $t all 14 ($(date +%H:%M:%S))"
      "$PY" "$ROOT/scripts/eval_framing.py" --target "$t" --trials 20 --timeout 600 || true ;;
    only:*)
      only="${plan#only:}"
      echo "  ##### framing $t identity ($only) ($(date +%H:%M:%S))"
      # shellcheck disable=SC2086
      "$PY" "$ROOT/scripts/eval_framing.py" --target "$t" --trials 20 --timeout 600 --only ${only//,/ } || true ;;
  esac
}

want_coding() { [[ "$PHASE" == "coding" || "$PHASE" == "all" ]]; }
want_analysis() { [[ "$PHASE" == "analysis" || "$PHASE" == "all" ]]; }
want_conc() { [[ "$PHASE" == "conc" ]]; }
want_framing() {
  [[ "$PHASE" == "framing" ]] && return 0
  [[ "$PHASE" == "all" && "$INCLUDE_FRAMING" == "1" ]] && return 0
  return 1
}

echo "PHASE=$PHASE INCLUDE_FRAMING=$INCLUDE_FRAMING ($(date -Is))"
"$PY" "$ROOT/scripts/split_ceval.py" || true

for t in "${LINUX_TARGETS[@]}"; do
  needs_anything "$t" || { echo "===== $t nothing missing"; continue; }
  echo "===== $t ($(date +%H:%M:%S))"
  if serve "$t"; then
    want_coding && run_coding "$t"
    want_analysis && run_analysis "$t"
    want_conc && run_conc "$t"
    want_framing && run_framing "$t"
  else
    echo "  $t FAILED to serve; see /tmp/serve-miss-$t.log"
  fi
  kill_port 8083 2>/dev/null; kill_port 8082 2>/dev/null; kill_port 8085 2>/dev/null
  if [[ "$t" == "ollama" ]]; then
    ollama stop "$("$PY" "$ROOT/scripts/serve.py" field "$t" alias)" >/dev/null 2>&1 || true
    kill_port 11434
  fi
  sleep 5
  "$PY" "$ROOT/scripts/make_report.py" || true
done

"$PY" "$ROOT/scripts/make_report.py" || true
echo "LINUX-MISSING DONE PHASE=$PHASE ($(date +%H:%M:%S))"

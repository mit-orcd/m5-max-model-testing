#!/usr/bin/env bash
# Remaining Strix suites after the speed/quality/C/ppl sweep.
# Same target list as run-strix-sweep.sh (no vLLM, no ling).
#
#   PHASE=coding    python + bash (easy), hard sets, research   [default]
#   PHASE=analysis  perf, brutal, repair, concurrency
#   PHASE=all       coding + analysis
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=env-strix.sh
source "$ROOT/scripts/env-strix.sh"
cd "$ROOT"

if [[ -s "$ROOT/results/machine.json" ]] && grep -q rtx-pro-6000 "$ROOT/results/machine.json"; then
  echo "NVIDIA results still in results/ — aborting" >&2
  exit 1
fi
cp "$ROOT/scripts/machines/strix-halo.json" "$ROOT/results/machine.json"

export SWEEP_ONLY="${SWEEP_ONLY:-gptoss qwen27 qwen35 coder gemma devstral aya qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b qwen35-27b seed-oss k2horizon laguna gptoss120 qwen35-122b nemotron3 laguna-s qwen38flash}"
export LLAMA_SERVE_PROFILE="${LLAMA_SERVE_PROFILE:-strix}"
export PHASE="${PHASE:-coding}"
exec "$ROOT/scripts/run-linux-missing.sh"

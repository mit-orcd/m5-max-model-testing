#!/usr/bin/env bash
# Full coding sweep for office002 (Strix Halo, 64 GB unified).
# Skips CUDA-only vLLM targets and GGUFs that cannot fit with KV (~60 GB+).
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=env-strix.sh
source "$ROOT/scripts/env-strix.sh"
cd "$ROOT"

if [[ ! -x "${LLAMA_SERVER_BIN:-}" ]]; then
  echo "Vulkan llama-server missing at $LLAMA_SERVER_BIN — run scripts/linux-setup-strix.sh" >&2
  exit 1
fi

OUT="$ROOT/results"
if [[ -s "$OUT/machine.json" ]] && grep -q rtx-pro-6000 "$OUT/machine.json"; then
  stamp=$(date +%Y%m%d-%H%M%S)
  echo "moving NVIDIA results aside -> ${OUT}-office001-$stamp"
  mv "$OUT" "${OUT}-office001-$stamp"
fi
mkdir -p "$OUT" "$OUT/failures"
cp "$ROOT/scripts/machines/strix-halo.json" "$OUT/machine.json"

# 46 GB coder-next is included; 60 GB+ weights are not.
export SWEEP_ONLY="gptoss qwen27 qwen35 coder gemma devstral aya qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b qwen35-27b seed-oss k2horizon laguna"
exec "$ROOT/scripts/run-all-benchmarks.sh"

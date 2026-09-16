#!/usr/bin/env bash
# Full coding sweep for office002 (Strix Halo, 128 GB LPDDR5).
# BIOS carves 64 GB as GPU VRAM; GTT is another ~31 GB from system RAM.
# Skip CUDA-only vLLM. Skip ling (no llama.cpp arch). 60–77 GB GGUFs fit.
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

export SWEEP_ONLY="${SWEEP_ONLY:-gptoss qwen27 qwen35 coder gemma devstral aya qwen36-27b qwen36-35b glm-flash coder-next deepseek-32b qwen35-27b seed-oss k2horizon laguna gptoss120 qwen35-122b nemotron3 laguna-s qwen38flash}"
"$ROOT/scripts/run-all-benchmarks.sh"

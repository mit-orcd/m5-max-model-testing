#!/usr/bin/env bash
# Download the second-wave models, then run whatever is missing. Detached via
# setsid so it survives the shell that launched it.
set -uo pipefail
ROOT=/Users/erbmi1/git/m5-max-model-testing
HF="$ROOT/.venv/bin/hf"
MODELS=(
  mlx-community/Qwen3.5-27B-4bit
  mlx-community/Seed-OSS-36B-Instruct-4bit
  mlx-community/KAT-Coder-V2.5-Dev-OptiQ-4bit-REAP-18B
  mlx-community/KAT-Coder-V2.5-Dev-OptiQ-4bit
  mlx-community/Ling-2.6-flash-mlx-4bit-DWQ
  mlx-community/Laguna-S-2.1-oQ4e
  mlx-community/Qwen3.5-122B-A10B-4bit
  mlx-community/DeepSeek-V4-Flash-2bit-DQ
  mlx-community/Nemotron-3-Super-120B-A12B-MLX-6bit
)
for m in "${MODELS[@]}"; do
  avail=$(df -g /Users/erbmi1 | awk 'NR==2{print $4}')
  if (( avail < 60 )); then
    echo "STOPPING: only ${avail} GB free, refusing to start $m"
    break
  fi
  echo "===== $m  (${avail} GB free, $(date +%H:%M:%S))"
  "$HF" download "$m" >/dev/null 2>&1 && echo "  ok $m" || echo "  FAILED $m"
done
echo "DOWNLOADS DONE ($(date +%H:%M:%S)), $(df -g /Users/erbmi1 | awk 'NR==2{print $4}') GB left"

# Bench whatever the downloads made possible.
"$ROOT/scripts/run-all-missing.sh"

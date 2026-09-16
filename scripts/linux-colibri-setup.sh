#!/usr/bin/env bash
# Bring up Colibrì on the Linux box so models that miss the 96 GB VRAM cut
# (DeepSeek-V4 Flash, later GLM-5.x) can still run: dense stays resident,
# routed experts stream from NVMe / pinned RAM / VRAM.
#
# Does NOT steal the GPU from an in-progress sweep — compile is CPU, download
# is disk. Run inference later with scripts/serve-colibri.sh
set -euo pipefail
WORK="${COLIBRI_HOME:-/home/root/colibri}"
MODEL="${COLIBRI_MODEL:-/home/root/models/DeepSeek-V4-Flash}"
ROOT="${ROOT:-/home/root/m5-max-model-testing}"
export HF_HOME="${HF_HOME:-/home/root/hf}"
export PATH="/usr/local/cuda/bin:${PATH}"

echo "==> clone $WORK"
if [[ ! -d "$WORK/.git" ]]; then
  git clone --depth 1 https://github.com/JustVugg/colibri.git "$WORK"
else
  git -C "$WORK" pull --ff-only || true
fi

echo "==> build DeepSeek-V4 engine (CUDA + DeepGEMM sm_120)"
# Blackwell: DEEPGEMM=1 is the sm_120a path. Generic CUDA_ARCH=portable also works.
make -C "$WORK/c" -f Makefile.deepseek-v4 deepseek-v4 \
  CUDA=1 DEEPGEMM=1 -j"$(nproc)"

echo "==> download DeepSeek-V4-Flash (~167 GB, native fp4, no convert)"
mkdir -p "$MODEL"
# Prefer the repo venv's huggingface_hub if present.
HF_BIN="${ROOT}/.venv/bin/python"
if [[ ! -x "$HF_BIN" ]]; then HF_BIN=python3; fi
HF_HUB_ENABLE_HF_TRANSFER=1 "$HF_BIN" -c "
from huggingface_hub import snapshot_download
p = snapshot_download('deepseek-ai/DeepSeek-V4-Flash-0731', local_dir='$MODEL')
print('done', p)
"

echo "DONE. Smoke (when the GPU is free):"
echo "  COLI_CUDA=1 COLI_GPU=0 CUDA_DENSE=1 COLI_CUDA_PIPE=2 \\"
echo "    $WORK/c/coli serve --model $MODEL --ram 64 --ctx 8192"
echo "  OpenAI API on :8090 by default — see docs/api.md"

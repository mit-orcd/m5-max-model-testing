#!/usr/bin/env bash
# Serve DeepSeek-V4 Flash via Colibrì (expert streaming). Do not run while a
# llama.cpp/vLLM sweep holds the GPU.
set -euo pipefail
COLI_HOME="${COLIBRI_HOME:-/home/root/colibri}"
MODEL="${COLIBRI_MODEL:-/home/root/models/DeepSeek-V4-Flash}"
export COLI_CUDA="${COLI_CUDA:-1}" COLI_GPU="${COLI_GPU:-0}"
export CUDA_DENSE="${CUDA_DENSE:-1}" COLI_CUDA_PIPE="${COLI_CUDA_PIPE:-2}"
export COLI_CUDA_ATTN_BATCH="${COLI_CUDA_ATTN_BATCH:-1}"
export COLI_CUDA_MOE_BATCH="${COLI_CUDA_MOE_BATCH:-1}"
exec "$COLI_HOME/c/coli" serve --model "$MODEL" --ram "${COLIBRI_RAM:-64}" --ctx "${COLIBRI_CTX:-8192}"

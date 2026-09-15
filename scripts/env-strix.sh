# Environment for orcd-office002 (Strix Halo / gfx1151).
# Root's HOME is /root; the copied tree lives in /home/root.
# shellcheck disable=SC1091
WORK="${WORK:-/home/root}"
export HF_HOME="${HF_HOME:-$WORK/hf}"
export MODELS_DIR="${MODELS_DIR:-$WORK/models}"
export ROCM_PATH="${ROCM_PATH:-/opt/rocm}"
export HIP_PATH="${HIP_PATH:-$ROCM_PATH}"
export PATH="$ROCM_PATH/bin:$PATH"
export CMAKE_PREFIX_PATH="$ROCM_PATH${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
export LLAMA_SERVER_BIN="${LLAMA_SERVER_BIN:-$WORK/llama.cpp/build-vulkan/bin/llama-server}"
export LLAMA_K2_SERVER_BIN="${LLAMA_K2_SERVER_BIN:-$WORK/llama-k2/build-vulkan/bin/llama-server}"
# Prefer Mesa RADV over AMDVLK if both ICDs are present.
export AMD_VULKAN_ICD="${AMD_VULKAN_ICD:-RADV}"

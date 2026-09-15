#!/usr/bin/env bash
# Bring-up for orcd-office002: Rocky 10, Ryzen AI MAX+ 395 / Radeon 8060S
# (gfx1151). Builds llama.cpp with Vulkan (Mesa RADV). Idempotent.
#
#   scripts/linux-setup-strix.sh
#
# CUDA builds copied from office001 will not run here (no libcuda). ROCm 10
# on this box ships libamdhip64 but not hip-lang CMake, so HIP configure fails.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=env-strix.sh
source "$ROOT/scripts/env-strix.sh"
WORK="${WORK:-/home/root}"

echo "==> packages"
dnf config-manager --set-enabled crb 2>/dev/null || true
dnf install -y gcc gcc-c++ cmake git git-lfs lsof curl wget unzip ninja-build \
  python3.12 python3.12-devel python3.12-pip \
  vulkan-loader vulkan-loader-devel vulkan-headers vulkan-tools \
  mesa-vulkan-drivers glslc glslang \
  spirv-headers-devel spirv-tools spirv-tools-devel

echo "==> GPU / Vulkan"
rocminfo | grep -E "Marketing Name:|Name: +gfx|Device Type:|Compute Unit:" | head -20
echo "VRAM bytes: $(cat /sys/class/drm/card1/device/mem_info_vram_total)"
echo "GTT  bytes: $(cat /sys/class/drm/card1/device/mem_info_gtt_total)"
vulkaninfo --summary 2>/dev/null | head -40 || vulkaninfo 2>/dev/null | head -40 || true

vk_build() {  # $1=src $2=builddir
  local src="$1" build="$2"
  cmake -S "$src" -B "$build" \
    -DGGML_VULKAN=ON -DGGML_CUDA=OFF -DGGML_HIP=OFF \
    -DCMAKE_BUILD_TYPE=Release
  cmake --build "$build" --config Release -j"$(nproc)" \
    --target llama-server llama-perplexity 2>/dev/null \
    || cmake --build "$build" --config Release -j"$(nproc)" --target llama-server
}

echo "==> llama.cpp Vulkan"
vk_build "$WORK/llama.cpp" "$WORK/llama.cpp/build-vulkan"
echo "==> MBZUAI-IFM fork Vulkan"
vk_build "$WORK/llama-k2" "$WORK/llama-k2/build-vulkan"

if [[ ! -x "$ROOT/.venv/bin/python" ]]; then
  echo "==> python venv"
  python3.12 -m venv "$ROOT/.venv"
  "$ROOT/.venv/bin/pip" install -U pip
  "$ROOT/.venv/bin/pip" install -r "$ROOT/requirements.txt"
fi

grep -q env-strix /root/.bashrc 2>/dev/null || cat >> /root/.bashrc <<EOF

# office002 Strix Halo (from m5-max-model-testing)
source $ROOT/scripts/env-strix.sh
EOF

echo
echo "DONE. llama-server: $LLAMA_SERVER_BIN"
ldd "$LLAMA_SERVER_BIN" | grep -iE "vulkan|ggml" || true
echo "Next: source $ROOT/scripts/env-strix.sh && scripts/run-strix-sweep.sh"

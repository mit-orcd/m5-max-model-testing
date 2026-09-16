#!/usr/bin/env bash
# Isolated SGLang venv. Do not pip-install into .venv — vLLM and SGLang pin
# different torch builds. Safe to run while another server owns the GPU.
#
#   scripts/linux-sglang-setup.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="${WORK:-/home/root}"
export HF_HOME="${HF_HOME:-$WORK/hf}"
VENV="$ROOT/.venv-sglang"

echo "==> SGLang venv at $VENV"
python3.12 -m venv "$VENV"
# Compilers may probe CUDA; don't steal the GPU from a running serve.
export CUDA_VISIBLE_DEVICES=""
"$VENV/bin/pip" install -U pip
# Official extra pulls sgl-kernel + the OpenAI server. Retry without extras
# if the fat extra fails on this CUDA/sm_120 combo.
if ! "$VENV/bin/pip" install "sglang[srt]"; then
  echo "sglang[srt] failed; trying sglang"
  "$VENV/bin/pip" install sglang
fi
"$VENV/bin/python" -c "import sglang; print('sglang', getattr(sglang, '__version__', '?'))"
touch "$VENV/.ready"
echo "DONE. Serve with scripts/serve-sglang.sh (uses $VENV)."

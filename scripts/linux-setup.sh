#!/usr/bin/env bash
# Bring-up for the Linux box. Written for orcd-office001.mit.edu:
#   Rocky Linux 10.2, Ryzen 9 9950X3D, 185 GB RAM,
#   NVIDIA RTX PRO 6000 Blackwell 96 GB (sm_120, driver 615.71.09).
# Should also work on other EL10-family boxes with a modern NVIDIA GPU.
# Idempotent; safe to re-run. Run from the repo root:
#
#   scripts/linux-setup.sh              # core: packages, CUDA toolkit, llama.cpp, venv, corpus
#   scripts/linux-setup.sh --with-vllm  # also pip-install vLLM (CUDA wheels)
#
# Layout on this box (root LV is only 70 GB — everything big lives in /home):
#   repo:    /home/m5-max-model-testing
#   models:  /home/models        (MODELS_DIR; GGUFs for llama.cpp)
#   HF cache: /home/hf           (HF_HOME; vLLM + downloads)
#   builds:  /home/llama.cpp and /home/llama-k2
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK=/home
MODELS_DIR="${MODELS_DIR:-$WORK/models}"
export HF_HOME="${HF_HOME:-$WORK/hf}"
WITH_VLLM=0
[[ "${1:-}" == "--with-vllm" ]] && WITH_VLLM=1

echo "==> packages (dnf)"
dnf install -y gcc gcc-c++ cmake git git-lfs lsof curl wget unzip ninja-build \
  python3.12 python3.12-devel python3.12-pip

echo "==> CUDA toolkit (driver 615 already present; toolkit only, for nvcc)"
if ! command -v nvcc >/dev/null 2>&1; then
  dnf config-manager --add-repo \
    https://developer.download.nvidia.com/compute/cuda/repos/rhel10/x86_64/cuda-rhel10.repo
  dnf install -y cuda-toolkit
fi
export PATH="/usr/local/cuda/bin:$PATH"
nvcc --version | tail -1

echo "==> GPU check"
nvidia-smi --query-gpu=name,memory.total,driver_version --format=csv

echo "==> llama.cpp (CUDA, sm_120)"
if [[ ! -d "$WORK/llama.cpp" ]]; then
  git clone https://github.com/ggml-org/llama.cpp "$WORK/llama.cpp"
fi
cmake -S "$WORK/llama.cpp" -B "$WORK/llama.cpp/build" \
  -DGGML_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=120 -DCMAKE_BUILD_TYPE=Release
cmake --build "$WORK/llama.cpp/build" --config Release -j"$(nproc)" \
  --target llama-server llama-perplexity

echo "==> MBZUAI-IFM llama.cpp fork (qwen4_exp / k2-horizon / laguna archs)"
if [[ ! -d "$WORK/llama-k2" ]]; then
  git clone -b model/K2Horizon https://github.com/MBZUAI-IFM/llama.cpp "$WORK/llama-k2"
fi
cmake -S "$WORK/llama-k2" -B "$WORK/llama-k2/build" \
  -DGGML_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=120 -DCMAKE_BUILD_TYPE=Release
cmake --build "$WORK/llama-k2/build" --config Release -j"$(nproc)" \
  --target llama-server

echo "==> python venv (3.12)"
python3.12 -m venv "$ROOT/.venv"
"$ROOT/.venv/bin/pip" install -U pip
"$ROOT/.venv/bin/pip" install -r "$ROOT/requirements.txt"

echo "==> laguna chat template (self-contained; GGUF template uses a Jinja include)"
mkdir -p "$MODELS_DIR"
curl -sf "https://huggingface.co/mlx-community/Laguna-XS.2-4bit/raw/main/chat_template.jinja" \
  -o "$MODELS_DIR/laguna-template.jinja" || echo "    template fetch failed; laguna target will need it manually"

echo "==> WikiText-2 raw corpus for llama-perplexity"
mkdir -p "$MODELS_DIR/ppl"
if [[ ! -s "$MODELS_DIR/ppl/wiki.test.raw" ]]; then
  HF_HOME="$HF_HOME" "$ROOT/.venv/bin/python" -c "
from datasets import load_dataset
ds = load_dataset('Salesforce/wikitext', 'wikitext-2-raw-v1', split='test')
with open('$MODELS_DIR/ppl/wiki.test.raw', 'w') as f:
    f.write('\n'.join(ds['text']))
print('corpus written')
" || echo "    corpus fetch failed; set PPL_CORPUS to any raw-text file"
fi

if [[ "$WITH_VLLM" == "1" ]]; then
  echo "==> vLLM (CUDA wheels; sm_120 needs the cu128/cu13 builds, current wheels qualify)"
  "$ROOT/.venv/bin/pip" install vllm
fi

# shell environment for future logins
grep -q "HF_HOME=$WORK/hf" /root/.bashrc 2>/dev/null || cat >> /root/.bashrc <<EOF
export HF_HOME=$WORK/hf
export MODELS_DIR=$MODELS_DIR
export PATH=/usr/local/cuda/bin:\$PATH
export LLAMA_SERVER_BIN=$WORK/llama.cpp/build/bin/llama-server
export LLAMA_K2_SERVER_BIN=$WORK/llama-k2/build/bin/llama-server
EOF

cat <<EOF

DONE. Next:
  1. scripts/download-models.sh scripts/models-linux.txt   # ~200+ GB into $MODELS_DIR
  2. Smoke test (llama.cpp):  scripts/serve-llamacpp.sh &   # gpt-oss-20b on :8080
     .venv/bin/python scripts/bench.py --target gptoss --case both --trials 2 --json
     .venv/bin/python scripts/eval_code.py --target gptoss --trials 1 --json
  3. Smoke test (vLLM):       scripts/serve-vllm.sh &
     .venv/bin/python scripts/bench.py --target gptoss-vllm --case both --trials 2 --json
  4. Full sweep:              scripts/run-all-benchmarks.sh
  5. Live stack (Hermes):     MAIN_MODEL_ID=gpt-oss-20b FAST_MODEL_ID=qwen-fast scripts/serve-all.sh
EOF

# Linux baseline — orcd-office001 (RTX PRO 6000 Blackwell)

Results from the Linux box. **This is a fresh baseline, not a continuation of
the M5 Max report** ([benchmarks.md](benchmarks.md)):

- Quants differ: GGUF Q4_K_M (llama.cpp) vs MLX 4-bit — quality scores shift slightly.
- Runtimes differ: llama.cpp (CUDA) and vLLM vs mlx-lm.
- Different hardware class entirely: 96 GB GDDR7 vs 128 GB unified memory —
  tok/s is not comparable across machines. Machine-verified quality scores
  (compile+run evals) do transfer as model rankings.

## The box

| | |
|---|---|
| host | orcd-office001.mit.edu |
| OS | Rocky Linux 10.2 (kernel 6.12, el10) |
| CPU | AMD Ryzen 9 9950X3D (16c/32t) |
| RAM | 185 GB |
| GPU | NVIDIA RTX PRO 6000 Blackwell, 96 GB VRAM (sm_120), driver 615.71.09 |
| storage | /home 3.4 TB (repo, models, HF cache all live here; / is only 70 GB) |

Note: the AMD iGPU (Granite Ridge, 2 CU) is present but unused — all serving
is CUDA on the RTX PRO 6000.

## Setup

One-time bring-up on the box: `scripts/linux-setup.sh --with-vllm`
(dnf packages, CUDA toolkit for nvcc, llama.cpp + MBZUAI-IFM fork built with
`-DGGML_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=120`, venv, perplexity corpus,
vLLM from PyPI CUDA wheels).

Then `scripts/download-models.sh scripts/models-linux.txt` (~200+ GB; verify
repo names in that file first — a wrong name 404s immediately).

Paths (root LV is only 70 GB): repo `/home/m5-max-model-testing`, models
`/home/models` (`MODELS_DIR`), HF cache `/home/hf` (`HF_HOME`), builds
`/home/llama.cpp` + `/home/llama-k2`.

## Runtime comparison: llama.cpp vs vLLM

Same weights, two runtimes, scored separately (the `gptoss`/`gptoss-vllm`,
`qwen27`/`qwen27-vllm`, `qwen35`/`qwen35-vllm` pairs). vLLM serves HF
originals (bf16; MXFP4 for gpt-oss), llama.cpp serves Q4_K_M GGUFs — so this
is a runtime+quant comparison; read it as "which stack should serve this box"
rather than a pure runtime benchmark.

| target | runtime | tok/s | C eval | peak RAM | notes |
|---|---|---|---|---|---|
| gptoss | llama.cpp CUDA | _pending_ | _pending_ | _pending_ | smoke-test target |
| gptoss-vllm | vLLM CUDA | _pending_ | _pending_ | _pending_ | |

## Full sweep results

_Pending — run `scripts/run-all-benchmarks.sh` on the box, then
`scripts/make_report.py`. Raw JSON lands in `results/`; target names are
shared with the Mac sweep, so keep the Mac results committed (or copied)
before the Linux sweep overwrites same-named JSONs._

## Coverage gaps vs the Mac sweep

No known GGUF/HF release (MLX-only quants), dropped on Linux:
ornith, laguna-mlx, laguna21, katcoder, katcoder-reap.

## Gotchas specific to this box

- **96 GB VRAM budget**: every mapped target fits (largest is qwen35-122b
  Q4_K_M at ~70 GB), but serve one at a time — the sweep already does.
- **Fork targets**: qwen38flash / k2horizon / laguna need the MBZUAI-IFM fork
  (`/home/llama-k2`, branch `model/K2Horizon`); set `LLAMA_K2_SERVER_BIN` if
  it's elsewhere. laguna additionally needs `--chat-template-file` (setup
  script fetches it to `$MODELS_DIR/laguna-template.jinja`).
- **HF throttling**: same as Mac — use `HF_TOKEN` for the big downloads.
- **Don't let anything write to /root or ~/.cache**: the root LV is 70 GB.
  `HF_HOME=/home/hf` is set in /root/.bashrc by the setup script.

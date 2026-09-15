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

Paths (root LV is only 70 GB): repo `/home/root/m5-max-model-testing`, models
`/home/root/models` (`MODELS_DIR`), HF cache `/home/root/hf` (`HF_HOME`), builds
`/home/root/llama.cpp` + `/home/root/llama-k2`. `/home/lincolnb` is someone
else's home — leave it.

## Runtime comparison: llama.cpp vs vLLM

Same weights, two runtimes, scored separately (the `gptoss`/`gptoss-vllm`,
`qwen27`/`qwen27-vllm`, `qwen35`/`qwen35-vllm` pairs). vLLM serves HF
originals (bf16; MXFP4 for gpt-oss), llama.cpp serves Q4_K_M GGUFs — so this
is a runtime+quant comparison; read it as "which stack should serve this box"
rather than a pure runtime benchmark.

| target | runtime | decode tok/s | C eval | notes |
|---|---|---|---|---|
| gptoss | llama.cpp CUDA | 360.3 | 50/57 | Q4_K_M-class MXFP4 GGUF |
| gptoss-vllm | vLLM CUDA | 283.8 | 44/57 | MXFP4 bf16-original |
| qwen27 | llama.cpp CUDA | 76.4 | 48/57 | Q4_K_M |
| qwen27-vllm | vLLM CUDA | 28.8 | 50/57 | bf16 — surprisingly slow; Mamba hybrid |
| qwen35 | llama.cpp CUDA | 261.9 | 38/57 | Q4_K_M |
| qwen35-vllm | vLLM CUDA | 182.7 | 34/57 | bf16 |

llama.cpp Q4_K_M wins every pair on both speed and (mostly) C eval — on this
box, serve GGUFs with llama.cpp. vLLM needed `--max-num-seqs 512` on the
hybrid linear-attention models (CUDA graph capture requires max_num_seqs ≤
available Mamba cache blocks).

## Full sweep results (2026-09-14)

Raw JSON on the box in `/home/root/m5-max-model-testing/results/`; a copy is
pulled to `results-linux/` in this repo (kept out of `results/` so the Mac
baseline is not overwritten). Report HTML regenerated on the box.

decode/prefill = tok/s (2 trials, temp 0); C eval = tasks passed of 57
(3 trials each); quality = 6 prompt-injection robustness checks;
ppl = WikiText-2 raw, 50 chunks, llama-perplexity (not comparable to MLX ppl).

| target | dec t/s | pre t/s | C eval | quality | ppl |
|---|---|---|---|---|---|
| gptoss | 360.3 | 332.1 | 50/57 | 4/6 | 121.3 ⚠ |
| coder | 338.3 | 266.4 | 41/57 | 6/6 | 9.05 |
| laguna | 316.7 | 270.9 | 42/57 | 6/6 | 13.06 |
| gptoss-vllm | 283.8 | 276.1 | 44/57 | 4/6 | — |
| qwen36-35b | 265.3 | 247.8 | 44/57 | 6/6 | 6.35 |
| qwen35 | 261.9 | 245.2 | 38/57 | 6/6 | 6.20 |
| coder-next | 242.3 | 226.5 | 50/57 | 6/6 | 7.06 |
| glm-flash | 231.1 | 196.3 | 36/57 | 6/6 | 16.08 |
| gemma | 222.0 | 190.2 | 48/57 | 6/6 | 14931 ⚠ |
| qwen35-vllm | 182.7 | 181.7 | 34/57 | 6/6 | — |
| k2horizon | 175.1 | 141.0 | 39/57 | 5/6 | 7.89 |
| laguna-s | 165.9 | 148.3 | 55/57 | 6/6 | 8.51 |
| qwen35-122b | 128.8 | 124.3 | 42/57 | 6/6 | 4.87 |
| qwen38flash | 117.5 | 94.6 | 52/57 | 6/6 | 3.60 |
| nemotron3 | 105.1 | 103.3 | 45/57 | 6/6 | 4.08 |
| devstral | 96.2 | 87.0 | 40/57 | 6/6 | 4.82 |
| qwen36-27b | 77.8 | 74.4 | 43/57 | 6/6 | 6.50 |
| qwen35-27b | 77.8 | 74.4 | 40/57 | 6/6 | 6.36 |
| qwen27 | 76.4 | 73.2 | 48/57 | 6/6 | 6.28 |
| deepseek-32b | 70.4 | 61.6 | 29/57 | 0/6 | 6.07 |
| aya | 65.4 | — | 23/57 | 4/6 | 6.12 |
| seed-oss | 64.0 | 56.6 | 2/57 | 1/6 | 8.91 |
| qwen27-vllm | 28.8 | 28.7 | 50/57 | 6/6 | — |

Sweep caveats:
- **aya prefill**: skipped — llama.cpp hard-caps requests at the model's
  n_ctx_train (8192) and the prefill prompt is ~11.4k tokens. bench.py now
  records such cases as skipped instead of aborting the target.
- **gemma ppl ⚠**: 14931 is a llama-perplexity/gemma4 artifact (BOS/control
  token handling), not a broken model — C eval 48/57 is healthy.
- **gptoss ppl ⚠**: 121 is the known MXFP4+harmony effect; matches the Mac.
- **seed-oss 2/57**: real — the model's answers don't compile/parse, not a
  harness issue (quality 1/6 agrees).
- **fork ctx**: qwen38flash/k2horizon serve with `--parallel 8`, so ctx was
  raised to 131072 (16k per slot) to fit the 11.4k-token prefill prompt.

## Coverage gaps vs the Mac sweep

No known GGUF/HF release (MLX-only quants), dropped on Linux:
ornith, laguna-mlx, laguna21, katcoder, katcoder-reap.
Also dropped:
- **deepseek-v4** — llama.cpp GGUF doesn't fit (smallest UD-IQ1_M is 87 GB with
  no KV headroom). **Colibrì** is the path: official fp4/fp8 checkpoint
  (~167 GB on disk), dense ~6 GB resident, experts streamed. Engine is built
  at `/home/root/colibri`; `scripts/linux-colibri-setup.sh` downloads
  `deepseek-ai/DeepSeek-V4-Flash-0731`. Serve with `scripts/serve-colibri.sh`
  when the GPU is free. Expect a few tok/s, not llama.cpp-class decode —
  this is "does it run", not the interactive stack.
- **ling** — llama.cpp has no `bailingmoe2.5` arch; Colibrì has no Ling
  engine either. Still dropped.

Substitutions: laguna benches the official poolside XS-2.1 GGUF (Mac used an
XS.2 blob); qwen38flash uses UD-Q2_K_XL (no Q4_K_M exists for that 177B MoE;
Mac used MLX 4-bit).

## Gotchas specific to this box

- **96 GB VRAM budget**: every mapped target fits (largest is qwen35-122b
  Q4_K_M at ~70 GB), but serve one at a time — the sweep already does.
- **Fork targets**: qwen38flash / k2horizon / laguna need the MBZUAI-IFM fork
  (`/home/root/llama-k2`, branch `model/K2Horizon`); set `LLAMA_K2_SERVER_BIN` if
  it's elsewhere. laguna additionally needs `--chat-template-file` (setup
  script fetches it to `$MODELS_DIR/laguna-template.jinja`).
- **HF throttling**: same as Mac — use `HF_TOKEN` for the big downloads.
- **Don't let anything write to /root or ~/.cache**: the root LV is 70 GB.
  `HF_HOME=/home/root/hf` is set in /root/.bashrc by the setup script.

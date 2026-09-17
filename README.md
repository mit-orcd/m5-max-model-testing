# m5-max-model-testing

Local LLM benchmarking for agentic coding. Scores are machine-verified (compile + hidden tests, no LLM judge).

- **macOS:** MLX on Apple Silicon (this repo’s original target: M5 Max, 128 GB).
- **Linux:** llama.cpp CUDA / vLLM / SGLang / Ollama on NVIDIA. Baseline: [docs/benchmarks-linux.md](docs/benchmarks-linux.md) (RTX 6000 Pro Workstation 96 GB). Not ROCm.

> **⚠️ Early research results.** Task sets are small; serving stacks differ; treat rankings as directional.

**Mac deployed:** gpt-oss-20b — 83.3 tok/s, 43/48 C, 11.7 GB, 19/19 one-shot self-repair.
**Mac most accurate:** qwen3.8-flash-next 125B — 46/48 C, 24/24 Python, 19/19 self-repair, 33.5 tok/s.

## Quick start

```bash
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
```

**Mac**

```bash
.venv/bin/pip install mlx-lm mlx-vlm
scripts/serve-mlx.sh &          # gpt-oss-20b on :8080
.venv/bin/python scripts/bench.py --target gptoss --case both --trials 2
.venv/bin/python scripts/eval_code.py --target gptoss --trials 1 --json
# full sweep: scripts/run-all-benchmarks.sh
```

**Linux (NVIDIA CUDA)** — `scripts/linux-setup.sh` is Rocky/EL10, writes under `/home`, builds for Blackwell (`sm_120`). On Ubuntu set `MODELS_DIR`/`HF_HOME`, install gcc/cmake/ninja/CUDA yourself, and change `CMAKE_CUDA_ARCHITECTURES` to your GPU.

```bash
export MODELS_DIR=$HOME/models HF_HOME=$HOME/hf
scripts/linux-setup.sh                 # add --with-vllm for vLLM pairs; scripts/linux-sglang-setup.sh for SGLang
scripts/download-models.sh scripts/models-linux.txt   # ~200 GB; or edit the list down to gpt-oss
export LLAMA_SERVER_BIN=$HOME/llama.cpp/build/bin/llama-server   # wherever it built
scripts/serve-llamacpp.sh &
.venv/bin/python scripts/bench.py --target gptoss --case both --trials 2
.venv/bin/python scripts/eval_code.py --target gptoss --trials 1 --json
# full sweep: scripts/run-all-benchmarks.sh
```

Set `HF_TOKEN` or Hugging Face downloads crawl. Linux tok/s is not comparable to the Mac; quality rankings are.

## Docs

- [`index.html`](index.html) — hub: machines first
- [`compare.html`](compare.html) — same model + same tests across machines
- `results/report.html` — M5 Max report · `results-linux/report.html` — RTX 6000 Pro Workstation
- `docs/benchmarks.md` — Mac writeup · `docs/benchmarks-linux.md` — Linux writeup
- `START-HERE.md` — Mac live stack and gotchas
- `scripts/run-all-benchmarks.sh` — speed + quality + C + perplexity (platform-detected)
- `scripts/run-linux-missing.sh` — remaining Linux suites (Python/Bash/hard/repair/…)

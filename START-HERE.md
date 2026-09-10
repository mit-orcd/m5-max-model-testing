# M5 Max Model Testing — START HERE

## What this is

Local LLM benchmarking and serving infrastructure for Apple Silicon (M5 Max, 128 GB unified memory). Built to find the best model for [Hermes](https://github.com/NousResearch/hermes) agentic coding.

## Current state (2026-09-10)

20 targets benchmarked; results in [docs/benchmarks.md](docs/benchmarks.md) and [docs/c-eval-findings.md](docs/c-eval-findings.md).

- **Deployed for Hermes: gpt-oss-20b** — 83.3 tok/s, 43/48 C, 11.7 GB, and 19/19 one-shot on the C self-repair eval.
- **Highest accuracy: qwen3.8-flash-next 125B** — 46/48 C, 24/24 Python, 9/9 Py-hard, 19/19 self-repair, at 33.5 tok/s and 38.6 GB. Requires the MBZUAI-IFM llama.cpp fork (mlx-lm has no `qwen4_exp` support).
- **Fastest: laguna-xs.2** — 92.2 tok/s, 45/48 C, but 10/24 Bash and 0/3 research.

### Live stack (running now)

| port | model | purpose |
|---|---|---|
| `:8080` | gpt-oss-20b | main chat / tools |
| `:8081` | Qwen3-8B | titles, compression |
| `:4000` | gateway | Hermes endpoint; strips gpt-oss Harmony analysis tokens |

Start: `scripts/serve-all.sh` or `scripts/hermes.sh` (auto-starts if down).

### Benchmark results

Full tables in [docs/benchmarks.md](docs/benchmarks.md). Top of the C eval (16 tasks × 3 trials, compile+run verified):

| model | tok/s | C eval | RAM GB | notes |
|---|---|---|---|---|
| qwen3.8-flash-next 125B | 33.5 | **46/48** | 38.6 | highest accuracy; llama.cpp fork only |
| laguna-xs.2 | 92.2 | 45/48 | 20.6 | fastest decode; weak Bash/research |
| gemma-4-26b-a4b | 53.4 | 44/48 | 13.7 | |
| gpt-oss-20b | 83.3 | 43/48 | 11.7 | deployed; smallest footprint |
| qwen3-coder-next 80B | 50.6 | 43/48 | 42.3 | |

### Suites

Speed, quality probes, C/Python/Bash (easy + hard), research extraction, C self-repair, WikiText perplexity (MLX only). Re-run everything with `scripts/run-all-benchmarks.sh`; raw JSON and failing code samples in `results/`, browsable report at `results/report.html`.

## Key files

| file | what |
|---|---|
| `scripts/gateway.py` | FastAPI gateway; routes main/fast, strips gpt-oss Harmony, caps tokens |
| `scripts/bench.py` | Speed bench (TTFT, tok/s, RSS) + quality suite |
| `scripts/eval_code.py` | 16 C tasks (+3 hard), compile + hidden tests, 3 trials |
| `scripts/eval_python.py`, `scripts/eval_bash.py` | Same pattern for Python and Bash |
| `scripts/eval_repair.py` | C self-repair: 5 rounds with compiler-error feedback |
| `scripts/eval_research.py` | RHEL 10 NFS extraction, deterministic hit/trap scoring |
| `scripts/make_report.py` | Builds `results/report.html` |
| `scripts/compare-model.sh` | Serve one model on :8083, bench, eval, teardown |
| `scripts/serve-*.sh` | Individual model servers |
| `scripts/hermes.sh` | Launch Hermes with auto-start |

## Rollback

Switch back to Qwen 27B: `scripts/serve-qwen.sh` on `:8080`, update `~/.hermes/config.yaml` default model.

## Gotchas

- **gpt-oss Harmony leak**: `mlx_lm.server` passes through `<|channel|>analysis<|message|>...` tokens. Gateway strips them. If you see raw `<|channel|>` in Hermes output, the gateway isn't filtering.
- **HF throttling**: Anonymous downloads get ~88 KB/s after ~20 GB/day. Use `HF_TOKEN` or wait.
- **Server death**: `nohup &` in Cursor shells gets reaped. Use `exec` in a dedicated shell or `setsid`.
- **Coder-30B Java-isms**: Emits `#import` and `public` in C code. Extractor sanitizes some, but eval score is genuinely low.
- **Unsupported architectures**: mlx-lm rejects `cohere2_moe` (north), `qwen4_exp` (qwen3.8-flash-next) and `k2-horizon`. The last two also fail on mainline llama.cpp/Ollama; they run only on the MBZUAI-IFM llama.cpp fork built at `/tmp/llama-k2` (branch `model/K2Horizon`).
- **Chat-template rejections**: laguna's GGUF template uses a Jinja `include` that Ollama and mainline llama.cpp refuse. Serve it with `--chat-template-file` pointing at a self-contained template.
- **Sharded GGUFs**: Ollama cannot pull them. Download with `hf download <repo> --include "<quant>/*"` and point llama-server at shard 00001.
- **Broken Xcode CLT on this machine**: `/Library/Developer/CommandLineTools/usr/include/c++/v1/` has 11 files instead of ~1000, so C++ builds fail with `'cstdio' file not found`. Workaround: `-I$(xcrun --show-sdk-path)/usr/include/c++/v1`. Proper fix: `xcode-select --install`.

## Links

- [Hermes](https://github.com/NousResearch/hermes)
- [mlx-lm](https://github.com/ml-explore/mlx-lm)
- [gpt-oss-20b](https://huggingface.co/openai/gpt-oss-20b)

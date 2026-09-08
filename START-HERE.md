# M5 Max Model Testing — START HERE

## What this is

Local LLM benchmarking and serving infrastructure for Apple Silicon (M5 Max, 128 GB unified memory). Built to find the best model for [Hermes](https://github.com/NousResearch/hermes) agentic coding.

## Current state (2026-09-08)

**Full 13-model sweep complete — see [docs/benchmarks.md](docs/benchmarks.md) and [docs/c-eval-findings.md](docs/c-eval-findings.md).** Winner for Hermes: **gpt-oss-20b** (fastest accurate model, 83 tok/s, 43/48 C eval, 12 GB). Most accurate: gemma-4-26b (45/48). Strong dark horse: qwen3-coder-next 80B (44/48, 50 tok/s, but 42 GB).

### Live stack (running now)

| port | model | purpose |
|---|---|---|
| `:8080` | gpt-oss-20b | main chat / tools |
| `:8081` | Qwen3-8B | titles, compression |
| `:4000` | gateway | Hermes endpoint; strips gpt-oss Harmony analysis tokens |

Start: `scripts/serve-all.sh` or `scripts/hermes.sh` (auto-starts if down).

### Benchmark results (16 C tasks × 3 trials, compile+run verified)

Superseded by the full 13-model sweep — see [docs/benchmarks.md](docs/benchmarks.md). Top of the table:

| model | tok/s | C eval | RAM GB | notes |
|---|---|---|---|---|
| gpt-oss-20b | 83 | 43/48 | 12 | 🏆 best overall (fast + accurate + tiny) |
| gemma-4-26b-a4b | 53 | **45/48** | 14 | 🏆 most accurate |
| qwen3-coder-next 80b | 51 | 44/48 | 42 | strong but heavy |

### Benchmarks

All 13 MLX models + the Ollama-vs-MLX runtime comparison are done (2026-09-08). Re-run everything with `scripts/run-all-benchmarks.sh`; raw JSON + failing code samples in `results/`.

## Key files

| file | what |
|---|---|
| `scripts/gateway.py` | FastAPI gateway; routes main/fast, strips gpt-oss Harmony, caps tokens |
| `scripts/bench.py` | Speed bench (TTFT, tok/s, RSS) + quality suite |
| `scripts/eval_code.py` | 16 C tasks, compile + hidden tests, 3 trials |
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

## Links

- [Hermes](https://github.com/NousResearch/hermes)
- [mlx-lm](https://github.com/ml-explore/mlx-lm)
- [gpt-oss-20b](https://huggingface.co/openai/gpt-oss-20b)

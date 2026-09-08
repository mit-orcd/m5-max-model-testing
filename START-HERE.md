# M5 Max Model Testing — START HERE

## What this is

Local LLM benchmarking and serving infrastructure for Apple Silicon (M5 Max, 128 GB unified memory). Built to find the best model for [Hermes](https://github.com/NousResearch/hermes) agentic coding.

## Current state (2026-09-08)

**Winner so far: gpt-oss-20b** — fastest (113 tok/s) + most accurate (45/48 C eval) + smallest (12 GB RAM).

### Live stack (running now)

| port | model | purpose |
|---|---|---|
| `:8080` | gpt-oss-20b | main chat / tools |
| `:8081` | Qwen3-8B | titles, compression |
| `:4000` | gateway | Hermes endpoint; strips gpt-oss Harmony analysis tokens |

Start: `scripts/serve-all.sh` or `scripts/hermes.sh` (auto-starts if down).

### Benchmark results (16 C tasks × 3 trials, compile+run verified)

| model | tok/s | C eval | RAM GB | notes |
|---|---|---|---|---|
| gpt-oss-20b | 113 | **45/48** | 12 | 🏆 best overall |
| gemma-4-26b-a4b | 111 | **45/48** | 14 | 🏆 tied, slightly more RAM |
| qwen3.8-27b | 30 | 42/48 | 16 | previous main, slow |
| devstral-2 24b | 29 | 41/48 | 14 | accurate, slow |
| qwen3.5-35b-a3b | 113 | 32/48 | 19 | fast, sloppy |
| ornith-1.5 35b | 91 | 29/48 | 21.5 | fast, sloppy |
| qwen3-coder-30b | 117 | 24/48 | 17 | fast, emits Java-isms |
| aya-23-35b | 22 | 19/48 | 19 | outclassed (2024) |

### In progress

Downloading 5 more models (~100 GB total, throttled):
- `qwen36-27b` — Qwen3.6-27B-4bit (dense, hyped successor)
- `qwen36-35b` — Qwen3.6-35B-A3B-4bit (MoE)
- `glm-flash` — GLM-4.7-Flash-4bit
- `coder-next` — Qwen3-Coder-Next-4bit (80B MoE, ~45 GB)
- `deepseek-32b` — DeepSeek-R1-Distill-Qwen-32B-MLX-4Bit

Run when done: `scripts/compare-model.sh <name> 3` for each, or `scripts/compare-all.sh` for all.

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

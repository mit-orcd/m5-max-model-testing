# m5-max-model-testing

Local LLM benchmarking on Apple Silicon (M5 Max, 128 GB) via MLX, to pick the best model for agentic coding.

**Winner so far:** gpt-oss-20b — 113 tok/s, 45/48 C eval, 12 GB RAM.

- `docs/benchmarks.md` — methodology + 8-model results
- `START-HERE.md` — current state, live stack, in-progress work
- `scripts/compare-all.sh` — reproduce the full benchmark

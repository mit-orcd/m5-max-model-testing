# m5-max-model-testing

Local LLM benchmarking on Apple Silicon (M5 Max, 128 GB) via MLX, to pick the best model for agentic coding.

**Winner:** gpt-oss-20b — 83 tok/s, 43/48 C eval, 12 GB RAM. Most accurate: gemma-4-26b (45/48).

- `docs/benchmarks.md` — 13-model results: speed, quality, perplexity, C/Python/Bash evals, Ollama vs MLX
- `docs/c-eval-findings.md` — C-coding eval deep-dive with real failure examples
- `results/report.html` — every failing sample, syntax-highlighted (`open results/report.html`)
- `START-HERE.md` — current state, live stack
- `scripts/run-all-benchmarks.sh` — reproduce the full sweep

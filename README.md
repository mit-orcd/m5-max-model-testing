# m5-max-model-testing

Local LLM benchmarking on Apple Silicon (M5 Max, 128 GB) via MLX, to pick the best model for agentic coding.

**Winner:** gpt-oss-20b — 83 tok/s, 43/48 C eval, 12 GB RAM. Most accurate: qwen3.8-flash-next 125B (46/48 C, 24/24 Python, 9/9 Py-hard) at 33.5 tok/s / 39 GB.

- `docs/benchmarks.md` — 19-model results: speed, quality, perplexity, C/Python/Bash evals, hard sets, research eval, C self-repair, Ollama vs MLX vs llama.cpp-fork
- `docs/c-eval-findings.md` — C-coding eval deep-dive with real failure examples
- `results/report.html` — every failing sample, syntax-highlighted (`open results/report.html`)
- `START-HERE.md` — current state, live stack
- `scripts/run-all-benchmarks.sh` — reproduce the full sweep

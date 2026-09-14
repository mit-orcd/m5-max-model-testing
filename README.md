# m5-max-model-testing

Local LLM benchmarking on Apple Silicon (M5 Max, 128 GB), to pick the best model for agentic coding. 20 targets, served via MLX, Ollama or llama.cpp depending on architecture support. Every score is machine-verified — code is compiled and run against hidden tests, no LLM judge.

> **⚠️ Early research results.** This is an ongoing research effort, not a finished benchmark.
> Results are preliminary: task sets are small, serving stacks differ per model, and several
> numbers rest on few runs. Findings may change as tasks, harnesses, and run counts are
> validated and expanded. Treat rankings as directional, not definitive.

**Deployed:** gpt-oss-20b — 83.3 tok/s, 43/48 C eval, 11.7 GB RAM, 19/19 one-shot self-repair.
**Most accurate:** qwen3.8-flash-next 125B — 46/48 C, 24/24 Python, 9/9 Python-hard, 19/19 self-repair, at 33.5 tok/s and 38.6 GB.

- `results/report.html` — start here: summary tables, every failing sample syntax-highlighted, one decode sample per model (`open results/report.html`)
- `docs/benchmarks.md` — full results: speed, quality, perplexity, C/Python/Bash evals, hard sets, research eval, C self-repair, Ollama vs MLX vs llama.cpp
- `docs/c-eval-findings.md` — C eval deep-dive with per-task matrix and real failure examples
- `START-HERE.md` — current state, live serving stack, gotchas
- `scripts/run-all-benchmarks.sh` — reproduce the full sweep

# M5 Max Local Model Benchmarks

Hardware: MacBook Pro, M5 Max (40-core GPU, ~614 GB/s memory bandwidth), 128 GB unified memory, macOS 26.2+.
Runtime: MLX (`mlx-lm` / `mlx_lm.server`), 4-bit quantizations unless noted.

## Methodology

- **C code eval:** 16 tasks (reverse_string, is_prime, fizzbuzz, binary_search, count_words, max_subarray, itoa, stack, popcount, rot13, parse_csv_ints, bswap32, trim, cmp_desc, ring, atoi_strict), 3 trials each (trial 1 at temp 0, rest at 0.7), compile + run verified. 48 samples per model.
- **Decode speed:** tokens/sec via `scripts/bench.py`.
- **Corrections applied:** response extractor fixed for glued code fences; gpt-oss Harmony `<|channel|>analysis` tokens stripped (with fair token budget it went 39 → 46/48).
- Caveat: measures zero-shot single-function C, not multi-turn agentic behavior.

## Final 8-model results

| model | decode tok/s | C eval | RAM GB | verdict |
|---|---|---|---|---|
| **gpt-oss-20b** | 113 | **45/48** | 12 | fast + accurate + smallest |
| **gemma-4-26b-a4b** | 111 | **45/48** | 14 | nearly as fast, equally accurate |
| qwen3.8-27b | 30 | 42/48 | 16 | reliable but slow |
| devstral-2 24b | 29 | 41/48 | 14 | accurate, slow |
| qwen3.5-35b-a3b | 113 | 32/48 | 19 | fast, sloppy |
| ornith-1.5 35b | 91 | 29/48 | 21.5 | fast, sloppy |
| qwen3-coder-30b-a3b | 117 | 24/48 | 17 | fast, emitted Java-isms |
| aya-23-35b | 22 | 19/48 | 19 | outclassed (2024 multilingual) |

## Findings

- **gpt-oss-20b and Gemma-4-26B tie for best** — ~4x faster than qwen3.8-27b and more accurate.
- Fast-but-sloppy cluster (qwen3.5-35b, ornith, coder-30b): dominant failure modes are missing `#include`s, ignoring explicit instructions (typedef/include), and logic edge cases (`itoa(0)`).
- Perplexity was a poor proxy for code reliability: ornith won on fluency (4.53 vs 6.47) while losing badly on verifiable C.
- Failures were systematic (consistent 0/3s), not sampling noise.

## Reproduce

```bash
scripts/compare-model.sh <name>   # serve one model, bench + eval, tear down
scripts/compare-all.sh            # full sweep
```

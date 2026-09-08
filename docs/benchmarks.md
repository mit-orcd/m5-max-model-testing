# M5 Max Local Model Benchmarks

Hardware: MacBook Pro, M5 Max (40-core GPU, ~614 GB/s memory bandwidth), 128 GB unified memory, macOS 26.2+.
Runtime: MLX (`mlx_lm.server` / `mlx_vlm.server`), 4-bit quantizations. Date: 2026-09-08.

## Methodology

- **Decode speed:** median tok/s over 2 trials of a full 2048-token generation (1500-word essay prompt), temp 0. `scripts/bench.py --case decode`
- **TTFT:** time to first token on the decode prompt. **Prefill TTFT:** time to first token on an ~11.4k-token prompt (tests prompt ingestion).
- **Quality probes:** 6 deterministic exact-match checks (arithmetic, exact token, JSON-only, primes, code expression, instruction following). `scripts/bench.py --case quality`
- **Perplexity:** WikiText-2, 50 samples, seed 0 (`mlx_lm.perplexity`). Lower = better. Only comparable across models with similar tokenizers — treat cross-family numbers as rough.
- **C eval:** 16 tasks × 3 trials (temp 0, then 0.7), compiled with `cc -std=c11 -Wall` and run against hidden test harnesses. 48 samples/model. Full analysis in [c-eval-findings.md](c-eval-findings.md).
- Caveat: an idle Hermes stack (gpt-oss + Qwen3-8B) was resident in RAM during the run; it generated nothing, but decode numbers are ~10–25% below earlier short-burst (256-token) measurements of the same models.

## Master results

| model | decode tok/s | TTFT ms | prefill TTFT (11.4k tok) | RAM GB | quality | perplexity ↓ | C eval |
|---|---|---|---|---|---|---|---|
| **gpt-oss-20b** (MXFP4-Q8) | **83.3** | **148** | 2852 | **11.7** | 3/6 \* | 8.24 | 43/48 |
| **gemma-4-26b-a4b** | 53.4 | 261 | **2456** | 13.7 | 6/6 | 92.99 \* | **45/48** |
| **qwen3-coder-next 80B** | 50.6 | **142** | 3064 | 42.3 | 6/6 | n/a \* | 44/48 |
| qwen3.5-35b-a3b | 74.0 | 159 | 8739 | 18.8 | 6/6 | **3.38** | 32/48 |
| ornith-1.5 35b | 70.7 | 259 | 2708 | 21.3 | 6/6 | 4.53 | 30/48 |
| qwen3.6-35b-a3b | 81.4 | 158 | 8739 | 18.7 | 6/6 | 4.12 | 34/48 |
| qwen3-coder-30b | 58.7 | 176 | 4432 | 16.3 | 6/6 | 4.61 | 25/48 |
| glm-4.7-flash | 47.5 | 279 | 8145 | 16.1 | 6/6 | 8.58 | 17/48 |
| qwen3.6-27b | 23.1 | 274 | 10974 | 14.7 | 6/6 | 6.13 | 32/48 |
| qwen3.8-27b (VLM) | 19.4 | 229 | 22747 | 15.3 | 6/6 | 6.47 | 40/48 |
| aya-23 35b | 18.9 | 455 | 23915 | 18.8 | 6/6 | 4.81 | 18/48 |
| devstral-2 24b | 18.2 | 1036 | 27503 | 13.6 | 6/6 | 3.45 | 42/48 |
| deepseek-r1 32b | 14.9 | 255 | 43441 | 17.5 | 0/6 \* | 3.85 | 23/48 |

\* Harness artifacts, not model quality — see notes below.

## Verdict

- **Best overall for Hermes: gpt-oss-20b** — fastest of the accurate tier (83 tok/s), smallest footprint (11.7 GB), 43/48 on verifiable C.
- **Most accurate: gemma-4-26b** (45/48) at a respectable 53 tok/s — good second opinion model.
- **qwen3-coder-next 80B** is the surprise: 44/48 C eval at 50 tok/s, but 42 GB RAM — only worth it on 128 GB if its agentic behavior proves out.
- **Fast-but-sloppy cluster** (qwen3.5-35b, qwen3.6-35b, ornith, coder-30b): 70–81 tok/s but 25–34/48. Speed doesn't pay for broken first drafts.
- **Dense 27B+ models** (qwen3.8, qwen3.6-27b, devstral, aya) are bandwidth-bound at ~18–23 tok/s — painful for interactive use.
- **deepseek-r1**: thinking chains make it the slowest and it burns its token budget thinking; not an agent fit.
- **Perplexity does not predict coding reliability.** qwen3.5-35b has the *best* perplexity (3.38) and a mediocre 32/48 C eval; gemma's 92.99 is a tokenizer/domain mismatch artifact, yet it won the C eval.

## Harness-artifact notes

- **gpt-oss quality 3/6:** its Harmony `<|channel|>analysis` tokens leak through the non-streaming API path used by the quality probes; the C eval strips them (which is why it scores 43/48 there). Real quality is fine — but the gateway must strip analysis tokens.
- **deepseek quality 0/6:** replies were empty because the 64-token probe cap was consumed entirely by `<think>` content.
- **gemma perplexity 92.99 / coder-next n/a:** tokenizer or format mismatch with the WikiText-2 perplexity path; both models' C-eval scores show the numbers are not a quality signal here.

## Ollama vs MLX (same Qwen 27B-class weights)

| runtime | decode tok/s | TTFT ms | RAM GB | quality | C eval |
|---|---|---|---|---|---|
| MLX (`mlx_vlm.server`) | 19.4 | 229 | 15.3 | 6/6 | **40/48** |
| Ollama (`qwen3.8:27b-mlx`) | **31.6** | **113** | 15.0 | 6/6 | 28/48 |

- Ollama decoded the same weights **~60% faster** with lower TTFT — its MLX backend is well optimized for single-stream decode.
- But Ollama scored **12 points worse** on the identical C eval (28 vs 40/48) with the same prompts and temp-0 first trial — its chat template / sampling path differs enough to matter for code.
- Ollama's prefill TTFT (155 ms) reflects aggressive prompt caching across repeated identical prompts; MLX's 22.7 s is the cold VLM path. Neither number is apples-to-apples — treat prefill as "both cache, differently."
- Takeaway: Ollama is fine for chat; for agentic coding where output correctness compounds, the MLX server path produced measurably better code from the same weights.

## Reproduce

```bash
scripts/run-all-benchmarks.sh    # full sweep, results -> results/
scripts/compare-model.sh <name>  # one model: serve, bench, eval, teardown
scripts/perplexity.sh <model>    # perplexity only
```

Raw JSON and all 179 failing code samples are in [`results/`](../results/).

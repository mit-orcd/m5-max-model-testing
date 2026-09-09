# M5 Max Local Model Benchmarks

Hardware: MacBook Pro, M5 Max (40-core GPU, ~614 GB/s memory bandwidth), 128 GB unified memory, macOS 26.2+.
Runtime: MLX (`mlx_lm.server` / `mlx_vlm.server`), 4-bit quantizations. Date: 2026-09-08.

## Methodology

- **Decode speed:** median tok/s over 2 trials of a full 2048-token generation (1500-word essay prompt), temp 0. `scripts/bench.py --case decode`
- **TTFT:** time to first token on the decode prompt. **Prefill TTFT:** time to first token on an ~11.4k-token prompt (tests prompt ingestion).
- **Quality probes:** 6 deterministic exact-match checks (arithmetic, exact token, JSON-only, primes, code expression, instruction following). `scripts/bench.py --case quality`
- **Perplexity:** WikiText-2 plain text (50 samples, seed 0, sequence-length 512). Lower = better. The earlier tulu-3-sft-mixture numbers conflated fluency with chat-SFT distribution match and were dropped; two models still needed footnotes (see notes below).
- **C eval:** 16 tasks × 3 trials (temp 0, then 0.7), compiled with `cc -std=c11 -Wall` and run against hidden test harnesses. 48 samples/model. Full analysis in [c-eval-findings.md](c-eval-findings.md).
- **Python/Bash evals:** 8 tasks × 3 trials each, hidden assert-based tests, same pattern as the C eval.
- **Hard sets:** 3 harder tasks per language, run separately (`--set hard`): C = `glob_match` (wildcard matcher), `rpn_eval` (RPN calculator), `csv_field` (RFC-4180 quoted-field parser). Python = `topological_sort` (with cycle detection), `lru_cache` (class with eviction semantics), `json_diff` (recursive diff paths). Bash = `find_dupes` (MD5 content dedup, BSD tooling), `top_errors` (log-signature frequency), `backup_rotate` (retention policy).
- **Research eval:** the model receives ~2,100 words of real RHEL 10 documentation excerpts (release notes + NFS mounting/caching chapters, `data/research/rhel10-nfs.txt`) and must summarize the NFS-relevant performance/bug-fix items. Scored deterministically: ≥5 of 7 known NFS facts (TLS/kTLS support, 1 MiB rsize/wsize max, krb5p overhead, FS-Cache, retrans/timeo, ReaR IPv6 fix, reparse points) and ≤1 mention of unrelated distractor fixes (LUKS, multipathd, NVMe, iSCSI, fstrim, pcs/cluster) present in the same text. Tests long-context extraction precision — no LLM judge.
- **Efficiency:** every coding-eval generation records wall time and completion tokens (server-reported usage), summed per suite.
- Caveat: an idle Hermes stack (gpt-oss + Qwen3-8B) was resident in RAM during the run; it generated nothing, but decode numbers are ~10–25% below earlier short-burst (256-token) measurements of the same models.

## Master results

| model | decode tok/s | RAM GB | quality | ppl (wikitext) ↓ | C | Python | Bash |
|---|---|---|---|---|---|---|---|
| **gpt-oss-20b** (MXFP4-Q8) | **83.3** | **11.7** | 3/6 \* | 25.00 | 43/48 | **24/24** | 14/24 |
| **gpt-oss-120b** (MXFP4-Q8) | 72.5 | 32.7 | 3/6 \* | 15.44 | 41/48 | **24/24** | 15/24 |
| **gemma-4-26b-a4b** | 53.4 | 13.7 | 6/6 | 1680 ¹ | **44/48** | **24/24** | 13/24 |
| **qwen3-coder-next 80B** | 50.6 | 42.3 | 6/6 | 27.23 ² | 43/48 | 23/24 | 15/24 |
| qwen3.5-35b-a3b | 74.0 | 18.8 | 6/6 | 11.15 | 32/48 | 22/24 | 15/24 |
| ornith-1.5 35b | 70.7 | 21.3 | 6/6 | 14.00 | 30/48 | 23/24 | **18/24** |
| qwen3.6-35b-a3b | 81.4 | 18.7 | 6/6 | 11.50 | 38/48 | 21/24 | 14/24 |
| qwen3-coder-30b | 58.7 | 16.3 | 6/6 | 21.45 | 25/48 | 8/24 | 8/24 |
| glm-4.7-flash | 47.5 | 16.1 | 6/6 | 31.95 | 17/48 | 20/24 | 11/24 |
| qwen3.6-27b | 23.1 | 14.7 | 6/6 | 11.97 | 32/48 | **24/24** | 15/24 |
| qwen3.8-27b (VLM) | 19.4 | 15.3 | 6/6 | 11.34 | 40/48 | **24/24** | 15/24 |
| aya-23 35b | 18.9 | 18.8 | 6/6 | 15.31 | 18/48 | 19/24 | 8/24 |
| devstral-2 24b | 18.2 | 13.6 | 6/6 | **9.55** | 42/48 | 23/24 | 14/24 |
| deepseek-r1 32b | 14.9 | 17.5 | 0/6 \* | 14.39 | 23/48 | 23/24 | 15/24 |
| kimi-k3 (referee, cloud) ³ | — | — | — | — | 16/16 † | 8/8 † | 8/8 † |

\* Harness artifacts, not model quality — see notes below.
¹ gemma is genuinely broken on plain text at every sequence length (see notes).
² coder-next measured at sequence-length 128; the default 512 triggers an mlx-lm batched-perplexity bug for hybrid linear-attention models (raw value 619,636 — harness artifact, see notes).
³ Referee baseline: the model writing this doc, hosted in the cloud — hardware metrics don't apply. † Single attempt per task (not 3 trials), and the referee authored the harness — treat 32/32 as a sanity ceiling, not a fair contest. Solutions in [`results/referee/kimi-k3/`](../results/referee/kimi-k3/).

C-eval scores are from the timed re-run; a few shifted vs the first pass (qwen3.6-35b 34→38, gemma 45→44, coder-next 44→43) — normal temp-0.7 trial variance on 48 samples.

## Verdict

- **Best overall for Hermes: gpt-oss-20b** — fastest of the accurate tier (83 tok/s), smallest footprint (11.7 GB), 43/48 on verifiable C.
- **gpt-oss-120b is the quality upgrade**: 72.5 tok/s at 32.7 GB (MoE: 5.1B active params), perfect 24/24 Python, 15/24 Bash, 41/48 C, and fewer completion tokens than the 20b per suite (16k vs 22k on C). If 33 GB RAM is acceptable, it's the better brain; the 20b remains the efficiency pick.
- **Most accurate: gemma-4-26b** (45/48) at a respectable 53 tok/s — good second opinion model.
- **qwen3-coder-next 80B** is the surprise: 44/48 C eval at 50 tok/s, but 42 GB RAM — only worth it on 128 GB if its agentic behavior proves out.
- **Fast-but-sloppy cluster** (qwen3.5-35b, qwen3.6-35b, ornith, coder-30b): 70–81 tok/s but 25–34/48. Speed doesn't pay for broken first drafts.
- **Dense 27B+ models** (qwen3.8, qwen3.6-27b, devstral, aya) are bandwidth-bound at ~18–23 tok/s — painful for interactive use.
- **deepseek-r1**: thinking chains make it the slowest and it burns its token budget thinking; not an agent fit.
- **Perplexity does not predict coding reliability.** devstral has the best wikitext perplexity (9.55) and gemma is off the charts (1680), yet gemma out-scored devstral on C and Python. Use perplexity only to catch broken builds, not to rank models.

## Harness-artifact notes

- **gpt-oss quality 3/6:** its Harmony `<|channel|>analysis` tokens leak through the non-streaming API path used by the quality probes; the C eval strips them (which is why it scores 43/48 there). Real quality is fine — but the gateway must strip analysis tokens.
- **deepseek quality 0/6:** replies were empty because the 64-token probe cap was consumed entirely by `<think>` content.
- **coder-next perplexity — root-caused as an mlx-lm harness bug, not the model.** On WikiText at the default sequence-length 512 it scores 619,636, but the *same data* at sequence-length 128 gives **18.07** (50-sample run: 27.23). A 512-vs-128 swing of 4 orders of magnitude on identical text is the known mlx-lm batched-forward/state-handling bug for hybrid Gated-DeltaNet models, not a property of the weights — the same model scores 43/48 on the long-context C eval. The table reports the seq-128 value.
- **gemma perplexity 1680 — genuine, different root cause.** Gemma is broken at *every* sequence length (3647 @128, 1680 @512, control qwen3.5-35b: 11.5), so this is not the hybrid-state bug. This 4-bit MLX build of gemma-4-it simply does not model plain prose (consistent with its quality-probe format quirks) — yet it still scored 44/48 C and 24/24 Python. Perplexity on public text says nothing about its coding ability.

## Python & Bash evals (all 14 targets, 8 tasks × 3 trials each)

Same harness pattern as the C eval — hidden tests, no LLM judge (`scripts/eval_python.py`, `scripts/eval_bash.py`). Scores are in the master table; per-model failures with syntax highlighting are in `results/report.html`.

Findings:

- **Python is nearly solved at this tier** — 10 of 14 targets score ≥ 21/24. The shock is **qwen3-coder-30b at 8/24**: it emits hallucinated tokens like `result.extend(subyte(sublist))` even at temp 0 — genuine model damage in this 4-bit build, matching its 25/48 C score.
- **ornith-1.5 35b wins Bash (18/24)** despite a mediocre 30/48 in C — the only model that consistently writes BSD-compatible coreutils.
- **Bash still separates models.** `largest_file` failed for almost everyone via GNU-only `find -printf` on macOS BSD userland; ornith was the notable exception.
- **deepseek-r1 scores well once think-stripped** (23/24 Python, 15/24 Bash) but pays for it: 30 min / 33k tokens for the Python suite vs ~0.5 min / 2k tokens for the MoE models.
- Referee baseline (Kimi K3, same harness): C 16/16, Python 8/8, Bash 8/8 — including BSD-correct `stat -f`.

## Efficiency (timed runs: total wall time / completion tokens per suite)

| model | C eval | Python | Bash |
|---|---|---|---|
| gpt-oss-20b | 3.4 min / 22.0k | 1.3 min / 8.2k | 2.3 min / 12.2k |
| gpt-oss-120b | 3.9 min / 16.1k | 1.9 min / 6.9k | 2.2 min / 8.2k |
| gemma-4-26b | 1.7 min / 8.3k | **0.6 min / 2.5k** | **0.4 min / 1.5k** |
| qwen3-coder-next 80B | 1.5 min / 7.0k | **0.5 min / 1.9k** | **0.4 min / 1.8k** |
| devstral-2 24b | 3.7 min / 6.0k | 1.2 min / 1.8k | 0.9 min / 1.4k |
| qwen3.6-35b | **1.4 min** / 8.0k | 0.4 min / 1.9k | 0.4 min / 2.2k |
| qwen3.8-27b (MLX) | 5.5 min / 7.4k | 1.7 min / 2.2k | 1.5 min / 2.0k |
| qwen3.8-27b (Ollama) | 15.3 min / **35.0k** | 5.6 min / **13.5k** | 7.0 min / **16.6k** |
| deepseek-r1 32b | — | 29.9 min / 32.8k | 18.6 min / 20.3k |

- **MoE models are dramatically cheaper per correct answer**: coder-next finishes the Python suite in 30 s using <2k tokens; dense qwen3.8-27b needs 3–4× that.
- **gpt-oss is token-hungry** (Harmony analysis tokens count toward completion): 22k tokens for the C suite vs 6–8k for peers.
- **Ollama burns 4–6× the tokens of MLX on identical weights** (35k vs 7.4k for the C suite) — its chat path doesn't stop cleanly after the code block and rambles explanations. That's why it's 3× slower wall-clock despite faster raw decode.

## Ollama vs MLX (same Qwen 27B-class weights)

| runtime | decode tok/s | RAM GB | quality | C | Python | Bash | C-suite tokens |
|---|---|---|---|---|---|---|---|
| MLX (`mlx_vlm.server`) | 19.4 | 15.3 | 6/6 | **40/48** | **24/24** | 15/24 | **7.4k** |
| Ollama (`qwen3.8:27b-mlx`) | **31.6** | 15.0 | 6/6 | 33/48 | 18/24 | 15/24 | 35.0k |

- Ollama decoded the same weights **~60% faster** with lower TTFT — its MLX backend is well optimized for single-stream decode.
- But Ollama scored worse on the identical C eval (33 vs 40/48; first pass 28) and much worse on Python (18 vs 24/24) — its chat template / sampling path differs enough to matter for code.
- **Ollama generates 4.7× more tokens per task** (35k vs 7.4k for the C suite): it doesn't stop after the code block and appends explanations, so its decode-speed advantage evaporates — 15.3 min vs 5.5 min wall-clock for the identical suite.
- Takeaway: Ollama is fine for chat; for agentic coding where output correctness and token discipline compound, the MLX server path produced measurably better code from the same weights.

## Reproduce

```bash
scripts/run-all-benchmarks.sh    # speed/quality/C-eval sweep, results -> results/
scripts/run-full-evals.sh        # C+Python+Bash (timed) for all targets + wikitext perplexity
scripts/compare-model.sh <name>  # one model: serve, bench, eval, teardown
scripts/perplexity.sh <model>    # perplexity only
scripts/eval_python.py --target <name> --trials 3   # python eval
scripts/eval_bash.py --target <name> --trials 3     # bash eval
scripts/make_report.py           # regenerate results/report.html
```

Raw JSON and all failing code samples are in [`results/`](../results/).

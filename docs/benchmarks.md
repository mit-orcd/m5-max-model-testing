# M5 Max Local Model Benchmarks

20 local models, ranked by how reliably they write working code. Every score here is
machine-verified — C is compiled with `cc -std=c11 -Wall`, Python runs against hidden asserts,
Bash is checked for exact stdout and exit codes. No LLM judges anything.

Hardware: MacBook Pro, M5 Max (40-core GPU, ~614 GB/s memory bandwidth), 128 GB unified memory,
macOS 26.2+. Runtimes: MLX, Ollama, and a llama.cpp fork for three models whose architectures
nothing else supports. All 4-bit quantizations. Last run 2026-09-10.

Browsable version with every failing code sample: [`results/report.html`](../results/report.html).

**Contents:** [Which model should you use](#which-model-should-you-use) ·
[All results](#all-results) · [Self-repair](#self-repair-can-it-fix-its-own-bugs) ·
[Serving stacks](#serving-stacks-what-actually-runs) · [Speed and token cost](#speed-and-token-cost) ·
[Ollama vs MLX](#ollama-vs-mlx-same-weights) · [What perplexity is worth](#what-perplexity-is-worth) ·
[Methodology](#methodology) · [Harness artifacts](#harness-artifacts) · [Reproduce](#reproduce)

## Which model should you use

- **Best accuracy: qwen3.8-flash-next 125B** — 109/126 coding tasks, the highest C score of the
  project (46/48), perfect Python (24/24 and 9/9 hard), and 38/41 correct on the first try in the
  self-repair eval. Costs 38.6 GB and runs at 33.5 tok/s. Needs the llama.cpp fork.
- **Best all-round: gpt-oss-120b** — 108/126, one point behind, but at 72.5 tok/s (more than twice
  as fast) and on a stock MLX stack. The only model to clear every hard set: 8/9 C, 9/9 Python,
  9/9 Bash.
- **Best value: gpt-oss-20b** — 102/126 in 11.7 GB at 83.3 tok/s. Nothing else comes close on
  accuracy-per-gigabyte, and it never failed a self-repair task outright (0 never-fixed).
- **Fastest: laguna-xs.2** at 92.2 tok/s, with a strong 45/48 C — but it collapses on Bash (10/24)
  and research (0/3), so it's a narrow specialist.
- **Avoid for agentic coding:** qwen3-coder-30b (48/126 — the 4-bit build is damaged), aya-23
  (49/126), glm-4.7-flash (55/126), and deepseek-r1 (70/126, and it burns 159k tokens on the
  repair suite).

Two results are worth internalizing beyond the ranking. **Speed does not predict correctness** —
laguna and north-mini-code are the two fastest models and sit 14th and 17th. And **perplexity
predicts nothing at all**: devstral has the best perplexity in the field (9.55) and ranks 7th,
while gemma's perplexity is off the charts (1,122) yet it scores 97/126.

## All results

Sorted by total coding tasks passed. C, Python and Bash are 3 trials per task (trial 1 at
temperature 0, trials 2–3 at 0.7); hard sets are 3 harder tasks each; research is a single
long-context extraction task scored deterministically.

| model | stack | total | tok/s | RAM GB | quality | ppl-w ↓ | C | Python | Bash | C-hard | Py-hard | Sh-hard | Research |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| qwen3.8-flash-next 125B | fork | **109/126** | 33.5 | 38.6 | 6/6 | — | **46/48** | 24/24 | 15/24 | 8/9 | 9/9 | 5/9 | 2/3 |
| gpt-oss-120b | MLX | **108/126** | 72.5 | 32.7 | 3/6 \* | 15.44 | 41/48 | 24/24 | 15/24 | 8/9 | 9/9 | **9/9** | 2/3 |
| gpt-oss-20b | MLX | **102/126** | 83.3 | **11.7** | 3/6 \* | 25.00 | 43/48 | 24/24 | 14/24 | 6/9 | 8/9 | 6/9 | 1/3 |
| qwen3-coder-next 80B | MLX | **102/126** | 50.6 | 42.3 | 6/6 | 27.23 ² | 43/48 | 23/24 | 15/24 | 6/9 | 8/9 | 4/9 | 3/3 |
| qwen3.8-27b | MLX | **100/126** | 19.4 | 15.3 | 6/6 | 11.34 | 40/48 | 24/24 | 15/24 | 3/9 | 9/9 | 6/9 | 3/3 |
| gemma-4-26b | MLX | **97/126** | 53.4 | 13.7 | 6/6 | 1,122 ¹ | 44/48 | 24/24 | 13/24 | 3/9 | 9/9 | 3/9 | 1/3 |
| devstral-2 24b | MLX | **96/126** | 18.2 | 13.6 | 6/6 | **9.55** | 42/48 | 23/24 | 14/24 | 5/9 | 7/9 | 3/9 | 2/3 |
| devstral-2 24b (rerun) ⁷ | MLX | **94/126** | 31.6 | 13.6 | 6/6 | 9.55 | 43/48 | 23/24 | 12/24 | 4/9 | 6/9 | 3/9 | 3/3 |
| ornith-1.5 35b | MLX | **94/126** | 70.7 | 21.3 | 6/6 | 14.00 | 30/48 | 23/24 | **18/24** | 5/9 | 8/9 | 7/9 | 3/3 |
| qwen3.6-27b | MLX | **90/126** | 23.1 | 14.7 | 6/6 | 11.97 | 32/48 | 24/24 | 15/24 | 4/9 | 9/9 | 6/9 | 0/3 |
| qwen3.6-35b | MLX | **89/126** | 81.4 | 18.7 | 6/6 | 11.50 | 38/48 | 21/24 | 14/24 | 6/9 | 8/9 | 2/9 | 0/3 |
| k2-horizon 36B-A4B | fork | **89/126** | 75.2 | 27.0 | 5/6 | — | 38/48 | 24/24 | 12/24 | 6/9 | 5/9 | 2/9 | 2/3 |
| qwen3.5-35b | MLX | **87/126** | 74.0 | 18.8 | 6/6 | 11.15 | 32/48 | 22/24 | 15/24 | 5/9 | 6/9 | 4/9 | 3/3 |
| laguna-xs.2 | fork | **86/126** | **92.2** | 20.6 | 6/6 | — | 45/48 | 21/24 | 10/24 | 2/9 | 8/9 | 0/9 | 0/3 |
| deepseek-r1 32b | MLX | **70/126** | 14.9 | 17.5 | 0/6 \* | 14.39 | 23/48 | 23/24 | 15/24 | 1/9 | 4/9 | 3/9 | 1/3 |
| qwen3.8-27b via Ollama ³ | Ollama | **69/126** | 31.6 | 15.0 | 6/6 | — | 33/48 | 18/24 | 15/24 | 0/9 | 0/9 | 1/9 | 2/3 |
| north-mini-code | Ollama | **69/126** | 91.1 | 19.3 | 3/6 | — | 36/48 | 18/24 | 12/24 | 0/9 | 3/9 | 0/9 | 0/3 |
| glm-4.7-flash | MLX | **55/126** | 47.5 | 16.1 | 6/6 | 31.95 | 17/48 | 20/24 | 11/24 | 2/9 | 3/9 | 0/9 | 2/3 |
| aya-23 35b | MLX | **49/126** | 18.9 | 18.8 | 6/6 | 15.31 | 18/48 | 19/24 | 8/24 | 0/9 | 3/9 | 0/9 | 1/3 |
| qwen3-coder-30b | MLX | **48/126** | 58.7 | 16.3 | 6/6 | 21.45 | 25/48 | 8/24 | 8/24 | 0/9 | 3/9 | 2/9 | 2/3 |

\* Harness artifact, not model quality — see [harness artifacts](#harness-artifacts).
¹ gemma is genuinely broken on plain text at every sequence length, yet still scores 97/126.
² Measured at sequence-length 128; the default 512 triggers an mlx-lm bug for hybrid-attention models.
³ Same weights as the qwen3.8-27b row above, served through Ollama — see [Ollama vs MLX](#ollama-vs-mlx-same-weights).
⁷ Same weights as the row above it, run a second time — the gap is trial variance, not a difference between models.

Perplexity is MLX-only; models served via Ollama or the llama.cpp fork show `—`.

### Harness check (not a ranked entry)

A cloud model, **kimi-k3**, was given the same tasks to confirm they are all solvable and that no
failure above is an artifact of the harness. It solved **41/41**. That figure is deliberately kept
out of the table: it got one attempt per task rather than 3 trials (41 samples against 126), it
skipped the research task, and it authored the harness in the first place. It is a ceiling, not a
score. Solutions in [`results/referee/kimi-k3/`](../results/referee/kimi-k3/).

The stronger validation is the audit: all 191 dumped C failures were re-graded through the harness
and 191/191 were confirmed real, so no model was penalized by a grading or code-extraction bug.

## Self-repair: can it fix its own bugs?

Each task gets up to 5 attempts. Round 1 is the normal one-shot attempt; after a failure the model
is handed its own code plus the compiler or test output and asked to fix it. This is the closest
proxy here for agentic behavior, where a model rarely gets one shot but does get error feedback.

`+n` = tasks fixed using the feedback. `never` = still broken after 5 rounds. `waste` = tokens
spent on tasks that needed more than one round. C is 19 tasks, Python and Bash 11 each.

**gen time** is the whole suite, not time-to-correct-answer: it sums every round of every task,
including rounds that failed and tasks that never passed. It counts model generation only — the
local compiling and testing between rounds isn't included.

| model | one-shot | C | Python | Bash | never | gen time (min) | waste (k tok) |
|---|---|---|---|---|---|---|---|
| qwen3.8-flash-next 125B | **38/41** | 19/19 | 11/11 | 8/11 +2 | 1 | 4 | **0.7** |
| gpt-oss-20b | **36/41** | 19/19 | 11/11 | 6/11 +5 | **0** | 7 | 16.5 |
| gpt-oss-120b | **35/41** | 18/19 +1 | 11/11 | 6/11 +5 | **0** | 6 | 8.5 |
| qwen3-coder-next 80B | **33/41** | 17/19 +2 | 10/11 +1 | 6/11 +3 | 2 | 2 | 4.4 |
| qwen3.8-27b | **33/41** | 15/19 +3 | 11/11 | 7/11 | 5 | 9 | 6.3 |
| devstral-2 24b | **33/41** | 17/19 +1 | 10/11 | 6/11 +1 | 6 | 6 | 5.2 |
| devstral-2 24b (rerun) | **33/41** | 17/19 +1 | 10/11 | 6/11 +1 | 6 | 6 | 5.1 |
| gemma-4-26b | **32/41** | 16/19 +1 | 11/11 | 5/11 +4 | 4 | 4 | 16.9 |
| qwen3.6-27b | **31/41** | 13/19 +3 | 11/11 | 7/11 +1 | 6 | 10 | 8.5 |
| laguna-xs.2 | **31/41** | 16/19 +2 | 10/11 +1 | 5/11 +1 | 6 | 2 | 7.0 |
| qwen3.6-35b | **30/41** | 14/19 +4 | 10/11 +1 | 6/11 +2 | 4 | 2 | 7.6 |
| qwen3.5-35b | **29/41** | 13/19 +4 | 9/11 +1 | 7/11 +1 | 6 | 3 | 12.7 |
| ornith-1.5 35b | **28/41** | 10/19 +7 | 10/11 | 8/11 | 6 | 3 | 8.6 |
| deepseek-r1 32b | **27/41** | 11/19 +6 | 9/11 +1 | 7/11 | 7 | 180 | 159.3 |
| k2-horizon 36B | **27/41** | 13/19 +4 | 10/11 | 4/11 +3 | 7 | 3 | 9.1 |
| north-mini-code | **24/41** | 13/19 +2 | 6/11 +2 | 5/11 | 13 | 19 | 74.8 |
| qwen3.8-27b via Ollama | **23/41** | 10/19 +7 | 7/11 +2 | 6/11 | 9 | 38 | 60.8 |
| glm-4.7-flash | **20/41** | 8/19 +2 | 7/11 +1 | 5/11 +2 | 16 | 16 | 59.5 |
| aya-23 35b | **17/41** | 7/19 +4 | 8/11 | 2/11 +2 | 18 | 20 | 19.0 |
| qwen3-coder-30b | **14/41** | 8/19 +6 | 4/11 +5 | 2/11 +3 | 13 | 3 | 15.3 |

The referee is excluded here too. `eval_repair.py` drives a local HTTP server, which can't reach a
cloud model, so its C figures (18/19 one-shot) are self-reported rather than instrumented, cover
only one of the three suites, and carry no token or timing data.

- **Error feedback works, and it works best on the models that need it least.** Both gpt-oss models
  ended with zero never-fixed tasks: everything they got wrong, they fixed when shown the error.
  That is the property that matters in an agent loop, and it does not track the one-shot ranking —
  gpt-oss-20b starts two points behind flash-next but finishes with a cleaner sheet.
- **Bash is where the field separates.** Nobody one-shots it. Flash-next leads at 8/11, and models
  that look similar on C diverge sharply: gemma repairs 4 of its 6 Bash misses, ornith repairs 0.
- **Two models are token incinerators.** deepseek-r1 spends 159k tokens and 3 hours because each
  round regenerates its entire thinking chain; north-mini-code wastes 75k. On a per-correct-answer
  basis they are an order of magnitude more expensive than flash-next's 0.7k.
- **Feedback cannot rescue a broken base model.** qwen3-coder-30b repairs 14 tasks — the most
  repairs in the field — and still finishes last, because it was wrong about so much to begin with.

## The brutal set: six problems with a wrong obvious answer

The 41 regular tasks separate good models from bad ones, but the top of the table was getting
crowded — several models sit within a few points of each other. So we added six problems chosen
on a single criterion: the approach a competent programmer reaches for first is wrong, and it is
wrong on a specific case rather than in general.

These are scored **separately** and are not part of the 126-task coding total, so every number
elsewhere in this document stays comparable with earlier runs. Three trials per task, temperature
0 then 0.7 twice, same deterministic grading as everything else.

| task | language | what breaks |
|---|---|---|
| `arena_alloc` | C | A fixed-buffer allocator. Freed blocks must merge with a free neighbour on either side, and `realloc` must grow into a free following block in place rather than moving. Implementations that skip coalescing pass every basic test and then fail once the arena is full. |
| `utf8_next` | C | Strict UTF-8 decode. Must reject overlong encodings, UTF-16 surrogates and anything past U+10FFFF, while still accepting the boundary values (U+D7FF, U+E000, U+10FFFF) that an over-strict decoder throws away. |
| `clone_graph` | Python | Deep copy without the `copy` module. Cycles must not recurse forever, and shared references must stay shared: if two fields point at one list, the clone's two fields must point at one new list. A visited-set handles cycles but silently duplicates shared objects. |
| `path_glob` | Python | Glob matching where `*` must not cross `/` and a `**` segment must match *zero* or more segments, so `a/**/b` matches `a/b`. Regex translation gets both wrong. |
| `csv_to_tsv` | Bash | An RFC-4180 parser in shell, with quoted commas, doubled quotes and line breaks inside fields. Python, Perl, Ruby, PHP and Node are replaced with stubs that exit 127, so the parsing has to happen in the shell. |
| `total_size` | Bash | Sum file sizes under filenames containing spaces, newlines, tabs, glob characters and leading dashes, on BSD userland. Punishes `for f in $(ls)`, unquoted expansion, `find` piped into `while read` without `-print0`, and the pipeline subshell that silently discards the running total. |

Every task was checked twice before any model saw it, by `scripts/validate_brutal.py`: a correct
reference solution must pass, and a plausible naive solution must fail. A task that the reference
cannot pass is broken; a task the naive version passes is not brutal. Both checks run in the same
grader the models face.

One reporting change came out of this. When a model exhausts its token budget mid-answer the
result is now recorded as `truncated` rather than as whatever syntax error the cut-off happens to
produce — gpt-oss-20b spends its entire 16k budget reasoning about `clone_graph` on two of three
trials and never emits a final answer, which is worth seeing as its own outcome.

Run it with `scripts/run-brutal.sh`, optionally naming targets: `scripts/run-brutal.sh gptoss laguna`.

## Serving stacks: what actually runs

Three of the newer models cannot be served by MLX at all, and two of those defeat mainline
llama.cpp and Ollama as well. This was the single largest time sink in the project, so it is
documented here rather than buried in a script.

| model | blocker | solution |
|---|---|---|
| north-mini-code | mlx-lm rejects the `cohere2_moe` architecture | Ollama with the unsloth GGUF |
| laguna-xs.2 | chat template uses a Jinja `include` that Ollama and mainline llama.cpp both refuse | MBZUAI-IFM llama.cpp fork with a self-contained `--chat-template-file` |
| k2-horizon | `k2-horizon` architecture unsupported everywhere | IFM fork, branch `model/K2Horizon`, built from source with Metal |
| qwen3.8-flash-next | `qwen4_exp` unsupported by mlx-lm and mainline llama.cpp; the GGUF is sharded, which Ollama cannot pull | IFM fork (it has `qwen4exp.cpp`), pointed at shard 00001 of the UD-Q4_K_XL download |

The practical lesson: for models released faster than the runtimes support them, the fork is the
difference between a benchmark and a blank row. k2-horizon and flash-next would both be missing
entirely otherwise — and flash-next turned out to be the most accurate model in the field.

## Speed and token cost

Total wall time and completion tokens per suite, from the timed runs:

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

- **MoE models are far cheaper per correct answer.** coder-next finishes the Python suite in 30
  seconds using under 2k tokens; the dense qwen3.8-27b needs three to four times that.
- **gpt-oss is token-hungry** because Harmony analysis tokens count toward completion: 22k tokens
  for the C suite against 6–8k for its peers.
- **Dense 27B+ models** (qwen3.8, qwen3.6-27b, devstral, aya) are bandwidth-bound at 18–23 tok/s,
  which is too slow to sit behind an interactive agent.
- **deepseek-r1 spends its budget thinking** rather than answering — the slowest model in the field
  and a poor agent fit despite decent raw scores.

## Ollama vs MLX (same weights)

The clearest controlled comparison in the project: identical Qwen 27B-class weights, two runtimes.

| runtime | decode tok/s | RAM GB | quality | C | Python | Bash | C-suite tokens |
|---|---|---|---|---|---|---|---|
| MLX (`mlx_vlm.server`) | 19.4 | 15.3 | 6/6 | **40/48** | **24/24** | 15/24 | **7.4k** |
| Ollama (`qwen3.8:27b-mlx`) | **31.6** | 15.0 | 6/6 | 33/48 | 18/24 | 15/24 | 35.0k |

Ollama decodes the same weights about 60% faster with lower time-to-first-token — its MLX backend
is well optimized for single-stream decode. But it scores worse on the identical C eval (33 vs
40/48) and much worse on Python (18 vs 24/24), because its chat template and sampling path differ
enough to matter for code. It also generates 4.7× more tokens per task: it doesn't stop after the
code block and appends explanations. The decode advantage evaporates — 15.3 minutes versus 5.5 for
the same suite.

Ollama is fine for chat. For agentic coding, where correctness and token discipline compound, the
MLX path produced measurably better code from the same weights.

## What perplexity is worth

Very little, for this purpose. The best perplexity in the field (devstral, 9.55) belongs to a model
that ranks 7th, and gemma's 1,122 — four orders of magnitude worse than its peers — belongs to a
model that scores 97/126 and 44/48 on C. Two conclusions:

- Perplexity on public text says nothing about whether a model writes code that compiles.
- It is still worth measuring as a smoke test: a wildly abnormal number tells you something about
  the build, just not about the capability. Use it to catch broken quantizations, not to rank.

## Methodology

- **Decode speed:** median tok/s over 3 trials of a full 2048-token generation, temp 0.
  `scripts/bench.py --case decode`. The prompt is a 1500-word essay request; each model's actual
  output is saved in `results/speed-texts/` and shown in the HTML report.
- **TTFT:** time to first token. **Prefill TTFT:** time to first token on an ~11.4k-token prompt.
- **Quality probes:** 6 deterministic exact-match checks (arithmetic, exact token, JSON-only,
  primes, code expression, instruction following). `scripts/bench.py --case quality`
- **C eval:** 16 tasks × 3 trials (temp 0, then 0.7 twice), compiled with `cc -std=c11 -Wall` and
  run against hidden test harnesses — 48 samples per model. Deep-dive in
  [c-eval-findings.md](c-eval-findings.md).
- **Python/Bash evals:** 8 tasks × 3 trials each, hidden assert-based tests, same pattern.
- **Hard sets:** 3 harder tasks per language. C = `glob_match` (wildcard matcher), `rpn_eval` (RPN
  calculator), `csv_field` (RFC-4180 quoted-field parser). Python = `topological_sort` with cycle
  detection, `lru_cache` with eviction semantics, `json_diff` returning recursive diff paths.
  Bash = `find_dupes` (MD5 content dedup on BSD tooling), `top_errors` (log-signature frequency),
  `backup_rotate` (retention policy).
- **Research eval:** the model gets ~2,100 words of real RHEL 10 documentation (release notes plus
  the NFS mounting and caching chapters, `data/research/rhel10-nfs.txt`) and must summarize the
  NFS-relevant items. Scored deterministically: at least 5 of 7 known NFS facts (TLS/kTLS, the
  1 MiB rsize/wsize maximum, krb5p overhead, FS-Cache, retrans/timeo, the ReaR IPv6 fix, reparse
  points) and at most 1 mention of the unrelated distractor fixes planted in the same text (LUKS,
  multipathd, NVMe, iSCSI, fstrim, pcs/cluster). Tests long-context extraction precision.
- **Self-repair:** up to 5 rounds per task, feeding back the failed code plus compiler/test output
  truncated to 1200 characters. `scripts/eval_repair.py --lang c|python|bash`
- **Brutal set:** 2 tasks per language × 3 trials, scored separately from the coding total and
  validated against a reference and a naive solution before use. `scripts/run-brutal.sh`
- **Perplexity:** WikiText-2 plain text, 50 samples, seed 0, sequence-length 512. MLX only.
- **Efficiency:** every coding-eval generation records wall time and server-reported completion
  tokens, summed per suite.
- Caveat on absolute speed: an idle Hermes stack (gpt-oss + Qwen3-8B) was resident in RAM during
  the round-1 runs. It generated nothing, but decode numbers are roughly 10–25% below earlier
  short-burst measurements of the same models. Relative comparisons are unaffected.

## Harness artifacts

Three scores in the table are stack problems rather than model weaknesses, and all three are real
integration hazards worth knowing about:

- **gpt-oss quality 3/6.** Its Harmony `<|channel|>analysis` tokens leak through the non-streaming
  API path the quality probes use. The C eval strips them, which is why the same model scores
  43/48 there. A gateway serving gpt-oss must strip analysis tokens.
- **deepseek quality 0/6.** The 64-token probe cap was consumed entirely by `<think>` content, so
  the visible reply was empty.
- **coder-next perplexity.** At the default sequence-length 512 it scores 619,636; the *same data*
  at 128 gives 18.07. A four-order-of-magnitude swing on identical text is the known mlx-lm
  batched-forward bug for hybrid Gated-DeltaNet models, not a property of the weights. The table
  reports the seq-128 value.
- **gemma perplexity 1,122 is genuine**, and a different root cause: gemma is broken at *every*
  sequence length (3647 at 128, 1122 at 512, against 11.5 for a control model). This 4-bit build
  simply does not model plain prose — while still scoring 44/48 on C.

A referee audit re-graded all 191 dumped C failures through the harness
(`scripts/grade_local.py --failures`): 191/191 confirmed as real failures, no false negatives from
code extraction or grading.

## Reproduce

```bash
scripts/run-all-benchmarks.sh    # speed/quality/C-eval sweep, results -> results/
scripts/run-full-evals.sh        # C+Python+Bash (timed) for all targets + wikitext perplexity
scripts/run-repair-sweep.sh      # C self-repair
scripts/run-repair-pysh.sh       # Python + Bash self-repair
scripts/capture-speed-texts.sh   # save one decode generation per model
scripts/compare-model.sh <name>  # one model: serve, bench, eval, teardown
scripts/eval_repair.py --target <name> --lang python --set all
scripts/make_report.py           # regenerate results/report.html
```

Every sweep script regenerates the HTML report when it finishes. Raw JSON and all failing code
samples are in [`results/`](../results/).

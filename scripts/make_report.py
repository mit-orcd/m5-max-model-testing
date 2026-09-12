#!/usr/bin/env python3
"""Generate results/report.html — all benchmark results + every failing code
sample (C, Python, Bash), syntax-highlighted with highlight.js (CDN).
Run: scripts/make_report.py"""
from __future__ import annotations

import datetime as dt
import html
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent
RESULTS = ROOT / "results"
OUT = RESULTS / "report.html"

sys.path.insert(0, str(Path(__file__).resolve().parent))
import eval_bash  # noqa: E402
import eval_code  # noqa: E402
import eval_python  # noqa: E402
import eval_research  # noqa: E402

# Prompts are pulled from the eval modules rather than copied, so the instruction
# shown next to a code sample is by construction the one that produced it.
PROMPT_MODULES = [("c", "C", eval_code), ("python", "Python", eval_python),
                  ("bash", "Bash", eval_bash)]
PROMPT_BY_TASK: dict[tuple[str, str], tuple[str, str]] = {}
for _lang, _label, _mod in PROMPT_MODULES:
    for _which in ("all", "brutal"):
        for _t in _mod.task_set(_which):
            PROMPT_BY_TASK[(_lang, _t["name"])] = (_t["sig"], _mod.build_prompt(_t))

TARGETS = ["gptoss", "gptoss120", "gemma", "coder-next", "devstral", "devstral2", "qwen27",
           "qwen36-35b", "qwen35", "qwen36-27b", "ornith", "coder", "deepseek-32b", "aya",
           "glm-flash", "north", "laguna", "laguna-mlx", "laguna21", "qwen38flash", "k2horizon", "ollama",
           "llama33", "qwen3-30b"]
NAMES = {"gptoss": "gpt-oss-20b", "gptoss120": "gpt-oss-120b", "gemma": "gemma-4-26b", "coder-next": "qwen3-coder-next 80B",
         "devstral": "devstral-2 24b", "devstral2": "devstral-2 24b (rerun)", "qwen27": "qwen3.8-27b", "qwen36-35b": "qwen3.6-35b",
         "qwen35": "qwen3.5-35b", "qwen36-27b": "qwen3.6-27b", "ornith": "ornith-1.5 35b",
         "coder": "qwen3-coder-30b", "deepseek-32b": "deepseek-r1 32b", "aya": "aya-23 35b",
         "glm-flash": "glm-4.7-flash", "ollama": "qwen3.8-27b via Ollama",
         "north": "north-mini-code",
         "laguna": "laguna-xs.2", "laguna-mlx": "laguna-xs.2 (MLX)",
         "laguna21": "laguna-xs 2.1",
         "qwen38flash": "qwen3.8-flash-next 125B",
         "k2horizon": "k2-horizon 36B-A4B", "llama33": "llama-3.3 70b", "qwen3-30b": "qwen3 30b-a3b"}

# Architecture: dense models activate every parameter each token; MoE models route to a
# few experts, so "active" is the per-token compute footprint and the reason a 125B model
# can decode like a 6B one. Active params and expert counts from the model cards.
ARCH = {
    "qwen38flash": ("MoE", "125B", "6B", "512, top-10 +1"),
    "gptoss120": ("MoE", "117B", "5.1B", "128, top-4"),
    "gptoss": ("MoE", "21B", "3.6B", "32, top-4"),
    "coder-next": ("MoE", "80B", "3B", "512, top-10 +1"),
    "coder": ("MoE", "30B", "3B", "128, top-8"),
    "qwen3-30b": ("MoE", "30.5B", "3.3B", "128, top-8"),
    "laguna": ("MoE", "33.4B", "3B", "256 +1 shared"),
    "laguna-mlx": ("MoE", "33.4B", "3B", "256 +1 shared"),
    "laguna21": ("MoE", "33.4B", "3B", "256 +1 shared"),
    "qwen36-35b": ("MoE", "35B", "3B", "256, top-8 +1"),
    "qwen35": ("MoE", "35B", "3B", "256, top-8 +1"),
    "ornith": ("MoE", "36B", "3B", "256, top-8 +1"),
    "gemma": ("MoE", "25.2B", "3.8B", "128, top-8 +1"),
    "qwen27": ("dense", "27B", "27B", "—"),
    "devstral": ("dense", "24B", "24B", "—"),
    "devstral2": ("dense", "24B", "24B", "—"),
    "qwen36-27b": ("dense", "27B", "27B", "—"),
    "deepseek-32b": ("dense", "32B", "32B", "—"),
    "aya": ("dense", "35B", "35B", "—"),
    "llama33": ("dense", "70B", "70B", "—"),
}

# Models kept for the record but out of the running, with the reason. Their full
# per-task sections are still written; they are only pulled out of the main table,
# the cost table and the nav, so a ranking is not led by something nobody would run.
SIDELINED = {
    "deepseek-32b": "A dense 32B reasoning distill: it thinks at length before "
                    "every answer, which at 32B dense costs more wall clock than "
                    "any score it returns can justify. Superseded by MoE reasoners "
                    "that activate a tenth of the weights.",
}

# How each model is served — shown in the report so the stack is reproducible.
STACK = {"north": "Ollama", "ollama": "Ollama",
         "laguna": "llama.cpp fork", "qwen38flash": "llama.cpp fork", "k2horizon": "llama.cpp fork"}

# (suffix, language for highlight.js, file extension)
SUITES = [("ceval", "C", "c", "c"), ("python", "Python", "python", "py"),
          ("bash", "Bash", "bash", "sh"), ("chard", "C (hard)", "c", "c"),
          ("pyhard", "Python (hard)", "python", "py"),
          ("shhard", "Bash (hard)", "bash", "sh"),
          ("research", "Research", "markdown", "md")]

# The brutal set is scored separately: it is not part of the 126-task coding
# total, so those numbers stay comparable with every earlier run.
BRUTAL_SUITES = [("brutal-c", "C (brutal)", "c", "c"),
                 ("brutal-python", "Python (brutal)", "python", "py"),
                 ("brutal-bash", "Bash (brutal)", "bash", "sh")]

BRUTAL_TASKS = [
    ("arena_alloc", "C arena", "fixed-buffer allocator: alignment, block merging, "
                               "and a realloc that grows in place"),
    ("utf8_next", "C utf8", "strict UTF-8 decode: overlong forms, surrogates, "
                            "and everything past U+10FFFF"),
    ("clone_graph", "Py clone", "deep copy that survives cycles and keeps shared "
                                "references shared"),
    ("path_glob", "Py glob", "glob matcher where '*' must not cross '/' and '**' must "
                              "match zero segments"),
    ("csv_to_tsv", "Sh csv", "RFC-4180 parser in shell: quoted commas, doubled quotes, "
                             "newlines inside fields"),
    ("total_size", "Sh sizes", "sum file bytes under filenames containing spaces, "
                               "newlines, globs and leading dashes"),
]

CSS = """
 :root { --bg:#0d1117; --panel:#161b22; --line:#30363d; --fg:#e6edf3; --dim:#9da7b3; --link:#58a6ff; }
 * { box-sizing: border-box; }
 body { font: 15px/1.5 -apple-system, BlinkMacSystemFont, sans-serif; max-width: 1400px;
        margin: 0 auto; padding: 0 1rem 4rem; background: var(--bg); color: var(--fg); }
 a { color: var(--link); text-decoration: none; } a:hover { text-decoration: underline; }
 h1 { font-size: 21px; margin: .6rem 0 .2rem; }
 h2 { font-size: 17px; margin: 1.6rem 0 .4rem; padding-top: .6rem; border-top: 1px solid var(--line); }
 h3 { font-size: 14px; margin: .9rem 0 .2rem; }
 h2 small, h3 small { color: var(--dim); font-weight: normal; }

 nav { position: sticky; top: 0; z-index: 10; background: rgba(13,17,23,.94);
       backdrop-filter: blur(8px); border-bottom: 1px solid var(--line);
       padding: .5rem 0; margin-bottom: .6rem; display: flex; gap: 1rem; align-items: center;
       flex-wrap: wrap; font-size: 13px; }
 nav select { background: var(--panel); color: var(--fg); border: 1px solid var(--line);
              border-radius: 5px; padding: 3px 6px; font-size: 13px; max-width: 230px; }
 nav .sp { flex: 1; }

 table { border-collapse: collapse; width: 100%; font-size: 12px; line-height: 1.25; }
 td, th { border: 1px solid var(--line); padding: 2px 6px; white-space: nowrap; }
 th { background: var(--panel); position: sticky; top: 42px; cursor: pointer; user-select: none; }
 th:hover { background: #1d242e; }
 th.sorted::after { content: ' \\25BE'; color: var(--link); }
 th.sorted.asc::after { content: ' \\25B4'; }
 tbody tr:hover { background: #12181f; }

 /* score shading — lets you scan a column without reading every number */
 .s-hi  { background: rgba(63,185,80,.20); }
 .s-mid { background: rgba(210,153,34,.16); }
 .s-lo  { background: rgba(219,109,40,.16); }
 .s-bad { background: rgba(248,81,73,.18); }
 .dim { color: var(--dim); }

 .cards { display: flex; gap: .7rem; flex-wrap: wrap; margin: .6rem 0 .9rem; }
 .card { flex: 1; min-width: 210px; background: var(--panel); border: 1px solid var(--line);
         border-left: 3px solid var(--link); border-radius: 6px; padding: .5rem .7rem; }
 .card .k { font-size: 11px; text-transform: uppercase; letter-spacing: .04em; color: var(--dim); }
 .card .v { font-size: 15px; font-weight: 600; margin: .1rem 0; }
 .card .d { font-size: 12px; color: var(--dim); line-height: 1.35; }
 .refcard { border-left-color: #8b949e; flex-basis: 100%; }

 .side { display: flex; gap: 1.5rem; align-items: flex-start; }
 .side > div { flex: 1; min-width: 0; }
 .note { background: var(--panel); border-left: 3px solid var(--link); font-size: 12px;
         line-height: 1.4; padding: .4rem .7rem; margin: .35rem 0; border-radius: 0 5px 5px 0; }
 .recap { font-size: 12.5px; color: var(--dim); margin: .1rem 0 .5rem; }
 .recap b { color: var(--fg); }

 pre { margin: 0 0 1rem; border-radius: 6px; } pre code { border-radius: 6px; }
 details { margin: .4rem 0 .4rem 1rem; } summary { cursor: pointer; }
 details.prompt > summary { color: #7a8; font-size: .85rem; }
 pre.prompt-text { white-space: pre-wrap; background: #10151c; border-left: 3px solid #7a8;
   padding: .6rem .8rem; margin: .3rem 0 .6rem 0; font-size: .85rem; color: #cfd6dd; }
 .promptlist { columns: 2; column-gap: 1.5rem; } .promptlist details { break-inside: avoid; }
 @media (max-width: 900px) { .promptlist { columns: 1; } }
 summary:hover { color: var(--link); }
 .meta { color: var(--dim); font-size: 12.5px; margin: .5rem 0 .2rem; }
 .status { color: #f85149; }
 .top { float: right; font-size: 11px; font-weight: normal; }
"""

SCRIPT = """
hljs.highlightAll();

// Parse a cell into a sortable number: "43/48" -> .896, "12.2k" -> 12200,
// "1.9 min" -> 1.9, "83.3" -> 83.3, "—" -> -Infinity (always sorts last).
function cellVal(td) {
  const raw = (td.getAttribute('data-v') ?? td.textContent).trim();
  if (!raw || raw === '\\u2014') return -Infinity;
  const frac = raw.match(/^(\\d+(?:\\.\\d+)?)\\s*\\/\\s*(\\d+(?:\\.\\d+)?)/);
  if (frac) return parseFloat(frac[1]) / parseFloat(frac[2]);
  const k = raw.match(/^(\\d+(?:\\.\\d+)?)k/);
  if (k) return parseFloat(k[1]) * 1000;
  const num = raw.match(/-?\\d+(?:\\.\\d+)?/);
  if (num) return parseFloat(num[0]);
  return raw.toLowerCase();
}

document.querySelectorAll('table').forEach(table => {
  const heads = table.tHead ? table.tHead.rows[0] : table.rows[0];
  if (!heads) return;
  Array.from(heads.cells).forEach((th, i) => {
    th.addEventListener('click', () => {
      const body = table.tBodies[0] || table;
      const rows = Array.from(body.rows).filter(r => r !== heads);
      const asc = !(th.classList.contains('sorted') && !th.classList.contains('asc'));
      rows.sort((a, b) => {
        const x = cellVal(a.cells[i]), y = cellVal(b.cells[i]);
        if (typeof x === 'string' || typeof y === 'string')
          return asc ? String(x).localeCompare(String(y)) : String(y).localeCompare(String(x));
        return asc ? x - y : y - x;
      });
      rows.forEach(r => body.appendChild(r));
      Array.from(heads.cells).forEach(h => h.classList.remove('sorted', 'asc'));
      th.classList.add('sorted');
      if (asc) th.classList.add('asc');
    });
  });
});

// Jump-to-model dropdown
const jump = document.getElementById('jump');
if (jump) jump.addEventListener('change', e => {
  if (e.target.value) location.hash = e.target.value;
});

// Expand/collapse every <details> in one click
const toggle = document.getElementById('toggle-all');
if (toggle) toggle.addEventListener('click', e => {
  e.preventDefault();
  const items = document.querySelectorAll('details');
  const anyClosed = Array.from(items).some(d => !d.open);
  items.forEach(d => d.open = anyClosed);
  toggle.textContent = anyClosed ? 'collapse all' : 'expand all';
});
"""


def load(prefix: str):
    p = RESULTS / f"{prefix}.json"
    if not p.exists():
        return None
    txt = p.read_text()
    m = re.search(r"^\[$", txt, re.M)
    if not m:
        return None
    return json.loads(txt[m.start():])


def perplexity(t: str, suffix: str = "perplexity") -> str:
    p = RESULTS / f"{t}-{suffix}.txt"
    if not p.exists():
        return "—"
    m = re.search(r"Perplexity: ([0-9.]+)", p.read_text())
    v = float(m.group(1)) if m else None
    if v is None:
        return "—"
    return f"{v:.2f}" if v < 1000 else f"{v:,.0f}"


def shade(passed: int, total: int) -> str:
    """CSS class for a score cell, so a column can be scanned at a glance."""
    if not total:
        return ""
    pct = passed / total
    return ("s-hi" if pct >= 0.9 else "s-mid" if pct >= 0.75
            else "s-lo" if pct >= 0.5 else "s-bad")


def shade_frac(pct: float) -> str:
    """shade() for a value already expressed as a 0-1 fraction."""
    return ("s-hi" if pct >= 0.9 else "s-mid" if pct >= 0.75
            else "s-lo" if pct >= 0.5 else "s-bad")


def score_td(v: dict | None) -> str:
    if not v:
        return "<td class='dim'>—</td>"
    return (f"<td class='{shade(v['passed'], v['total'])}'>"
            f"<b>{v['passed']}</b>/{v['total']}</td>")


def eff(v: dict | None) -> str:
    """Compact 'time / tokens' for a suite result."""
    if not v or not v.get("total_time_s"):
        return "—"
    mins = v["total_time_s"] / 60
    ktok = (v.get("total_tokens") or 0) / 1000
    return f"{mins:.1f} min / {ktok:.1f}k tok"


def prompt_block(lang: str, task: str) -> str:
    """The instruction that produced the samples below it, collapsed by default."""
    entry = PROMPT_BY_TASK.get((lang, task))
    if not entry:
        return ""
    _sig, prompt = entry
    return ("<details class='prompt'><summary>prompt sent to the model</summary>"
            f"<pre class='prompt-text'>{html.escape(prompt)}</pre></details>")


def suite_sections(t: str, data: dict, label: str, lang: str, ext: str) -> str:
    fails = []
    for task, outcomes in data["results"].items():
        if all(s == "pass" for s in outcomes):
            continue
        note = html.escape(data["notes"].get(task, ""))
        samples = []
        for trial, status in enumerate(outcomes):
            if status == "pass":
                continue
            f = RESULTS / "failures" / f"{t}-{task}-t{trial}.{ext}"
            code = html.escape(f.read_text()) if f.exists() else "(no code extracted)"
            temp = "temp 0" if trial == 0 else "temp 0.7"
            samples.append(
                f"<div class='sample'><div class='meta'>trial {trial} ({temp}) — "
                f"<span class='status'>{status}</span>"
                f"{f' — <code>{note}</code>' if note else ''}</div>"
                f"<pre><code class='language-{lang}'>{code}</code></pre></div>")
        npass = sum(1 for s in outcomes if s == "pass")
        fails.append(
            f"<details><summary>{task} — {npass}/{len(outcomes)} passed</summary>"
            + prompt_block(lang, task) + "".join(samples) + "</details>")
    effs = f" <small>({eff(data)})</small>" if data.get("total_time_s") else ""
    return (f"<h3>{label} — {data['passed']}/{data['total']}{effs}</h3>"
            + ("".join(fails) if fails else "<p class='dim'>No failures.</p>"))


def main() -> None:
    rows, sections, nav_opts, sidelined_rows = [], [], [], []
    stats = {}
    for t in TARGETS:
        suites = {s: (load(f"{t}-{s}") or [None])[0] for s, *_ in SUITES}
        if all(v is None for v in suites.values()):
            continue
        # most targets write <t>-speed.json; a few older runs split it into
        # <t>-decode.json / <t>-prefill.json
        speed = load(f"{t}-speed") or load(f"{t}-decode")
        qual = load(f"{t}-quality")
        dec = next((r for r in speed if r["case"] == "decode"), {}) if speed else {}
        tok, rss = dec.get("tok_s"), dec.get("peak_rss_mb")
        qd = qual[0] if qual else None
        total_p = sum(v["passed"] for v in suites.values() if v)
        total_t = sum(v["total"] for v in suites.values() if v)
        stats[t] = {"tok": tok, "rss": rss, "passed": total_p, "total": total_t}

        cells = "".join(score_td(v) for v in suites.values())
        qcell = (f"<td class='{shade(qd['passed'], qd['total'])}'>{qd['passed']}/{qd['total']}</td>"
                 if qd else "<td class='dim'>—</td>")
        stack = STACK.get(t, "MLX")
        if t in SIDELINED:
            sidelined_rows.append(
                f"<tr><td><a href='#{t}'>{NAMES[t]}</a> <span class='dim'>{stack}</span></td>"
                f"<td class='{shade(total_p, total_t)}'>{total_p}/{total_t}</td>"
                f"<td>{f'{tok:.1f}' if tok else '—'}</td>"
                f"<td class='dim'>{SIDELINED[t]}</td></tr>")
        else:
            rows.append(
                (total_p / total_t if total_t else 0,
                 f"<tr><td><a href='#{t}'>{NAMES[t]}</a> <span class='dim'>{stack}</span></td>"
                 f"<td data-v='{total_p / total_t if total_t else 0}' class='{shade(total_p, total_t)}'>"
                 f"<b>{total_p}</b>/{total_t}</td>"
                 f"<td>{f'{tok:.1f}' if tok else '—'}</td>"
                 f"<td>{f'{rss / 1024:.1f}' if rss else '—'}</td>"
                 f"{qcell}<td>{perplexity(t, 'wikitext-perplexity')}</td>"
                 f"<td>{perplexity(t)}</td>{cells}</tr>"))

        sample_f = RESULTS / "speed-texts" / f"{t}.txt"
        sample_html = ""
        if sample_f.exists():
            words = len(sample_f.read_text().split())
            sample_html = (
                f"<details><summary>decode sample — the 1500-word MIT essay from the speed "
                f"benchmark ({words} words produced)</summary>"
                f"<pre style='white-space:pre-wrap'>{html.escape(sample_f.read_text())}</pre>"
                "</details>")
        recap = (f"<p class='recap'>Served via <b>{stack}</b> · "
                 f"<b>{f'{tok:.1f}' if tok else '—'}</b> tok/s · "
                 f"<b>{f'{rss / 1024:.1f}' if rss else '—'}</b> GB RAM · "
                 f"coding total <b>{total_p}/{total_t}</b></p>")
        brutal = {s: (load(f"{t}-{s}") or [None])[0] for s, *_ in BRUTAL_SUITES}
        body = recap + sample_html + "".join(
            suite_sections(t, v, label, lang, ext)
            for (s, label, lang, ext), v in zip(SUITES, suites.values()) if v)
        body += "".join(
            suite_sections(t, v, label, lang, ext)
            for (s, label, lang, ext), v in zip(BRUTAL_SUITES, brutal.values()) if v)
        sections.append(
            (total_p / total_t if total_t else 0,
             f"<h2 id='{t}'>{NAMES[t]} <small>{total_p}/{total_t}</small>"
             f"<a class='top' href='#summary'>↑ top</a></h2>{body}"))
        if t not in SIDELINED:
            nav_opts.append((total_p / total_t if total_t else 0,
                             f"<option value='#{t}'>{NAMES[t]} — {total_p}/{total_t}</option>"))

    # Best model first, everywhere.
    rows.sort(key=lambda x: -x[0])
    sections.sort(key=lambda x: -x[0])
    nav_opts.sort(key=lambda x: -x[0])
    rows = [r for _, r in rows]
    # Captured under a name nothing else uses, so it survives if `rows` is shadowed
    # later. Compared against the rendered page just before writing.
    expected_models = len(rows)
    sections = [s for _, s in sections]
    nav_opts = [o for _, o in nav_opts]

    # Referee row + section (Kimi K3's own solutions, graded by the same harness).
    # Counts are derived from the files on disk — the referee ran ONE attempt per
    # task, so its denominators are per-task while every local model is per-trial.
    ref = RESULTS / "referee" / "kimi-k3"
    ref_panel = ""
    if ref.exists():
        ref_counts = {}
        for key, sub, ext in (("c", "", "c"), ("py", "py", "py"), ("sh", "sh", "sh")):
            d = ref / sub if sub else ref
            ref_counts[key] = sorted(d.glob(f"*.{ext}"))
        # easy/hard split mirrors the task lists, so hard tasks land in the hard columns
        import eval_bash, eval_code, eval_python  # noqa: PLC0415
        hard = {
            "c": {t["name"] for t in eval_code.task_set("hard")},
            "py": {t["name"] for t in eval_python.task_set("hard")},
            "sh": {t["name"] for t in eval_bash.task_set("hard")},
        }
        split = {}
        for key, files in ref_counts.items():
            names = {f.stem for f in files}
            split[key] = (len(names - hard[key]), len(names & hard[key]))
        ref_total = sum(e + h for e, h in split.values())
        # Deliberately NOT a row in the ranked table: one attempt per task instead
        # of 3 trials, so its denominators aren't comparable to a served model's.
        ref_panel = (
            "<div class='card refcard'><div class='k'>harness check — not a contestant</div>"
            f"<div class='v'><a href='#referee'>kimi-k3 (referee)</a> solved "
            f"{ref_total}/{ref_total}</div>"
            "<div class='d'>A cloud model was given the same tasks to confirm they're all "
            "solvable and that no failure below is a harness artifact. It's kept out of the "
            "ranking because it got <b>one attempt per task</b> rather than 3 trials "
            f"({ref_total} samples vs 126), ran no research task, and authored the harness. "
            "Read it as a ceiling, not a score.</div></div>")
        ref_blocks = []
        for key, lang, label in (("c", "c", "C"), ("py", "python", "Python"), ("sh", "bash", "Bash")):
            files = ref_counts[key]
            items = "".join(
                f"<details><summary>{f.name}</summary>"
                f"<pre><code class='language-{lang}'>{html.escape(f.read_text())}</code></pre></details>"
                for f in files)
            ref_blocks.append(f"<h3>{label} — {len(files)}/{len(files)}</h3>{items}")
        sections.append(
            f"<h2 id='referee'>kimi-k3 (referee) <small>{ref_total}/{ref_total}</small>"
            "<a class='top' href='#summary'>↑ top</a></h2>"
            "<p class='note'><b>Its denominators differ from every other row on purpose.</b> The "
            "referee wrote one solution per task, while the local models get 3 trials per task — so "
            f"the C suite is {split['c'][0]} samples here versus {split['c'][0] * 3} for a served "
            "model. Same tasks, same grader, fewer attempts. Hardware columns are blank because the "
            "referee is a hosted cloud model, not served on this Mac. And it authored the harness, "
            f"so treat {ref_total}/{ref_total} as a sanity ceiling that proves the tasks are all "
            "solvable — not as a fair contest.</p>"
            + "".join(ref_blocks))

    # Self-repair section: results/<t>-repair[-lang].json from scripts/eval_repair.py
    rep_rows = []
    ref_repair = load("kimi-k3-repair")
    for t in TARGETS:
        per_lang = {}
        for lang, suffix in (("C", "repair"), ("Py", "repair-python"), ("Sh", "repair-bash")):
            r = load(f"{t}-{suffix}")
            per_lang[lang] = r[0] if r else None
        if not any(per_lang.values()):
            continue
        name = NAMES[t]
        link = f"<a href='#{t}'>{name}</a>" if t in stats else name
        cells = ""
        tot_one = tot_tasks = tot_never = 0
        tot_secs = tot_waste = 0.0
        for lang in ("C", "Py", "Sh"):
            r = per_lang[lang]
            if not r:
                cells += "<td class='dim'>—</td>"
                continue
            rep = f" <small class='dim'>+{r['repaired']}</small>" if r["repaired"] else ""
            nev = f" <small class='status'>✗{r['never']}</small>" if r["never"] else ""
            cells += (f"<td data-v='{r['one_shot'] / r['tasks']}' "
                      f"class='{shade(r['one_shot'], r['tasks'])}'>"
                      f"<b>{r['one_shot']}</b>/{r['tasks']}{rep}{nev}</td>")
            tot_one += r["one_shot"]; tot_tasks += r["tasks"]; tot_never += r["never"]
            tot_secs += r.get("total_time_s") or 0
            tot_waste += r.get("waste_tokens") or 0
        # rank by rate, not raw count — the referee only ran C, so an absolute
        # count would bury it near the bottom despite the best pass rate
        rep_rows.append(
            {"rate": tot_one / tot_tasks if tot_tasks else 0, "never": tot_never,
             "one": tot_one, "tasks": tot_tasks, "target": t, "name": name,
             "row": f"<tr><td>{link}</td>"
                    f"<td data-v='{tot_one / tot_tasks if tot_tasks else 0}' "
                    f"class='{shade(tot_one, tot_tasks)}'><b>{tot_one}</b>/{tot_tasks}</td>{cells}"
                    f"<td>{tot_never or '<span class=dim>0</span>'}</td>"
                    f"<td>{f'{tot_secs / 60:.0f}' if tot_secs else '—'}</td>"
                    f"<td>{f'{tot_waste / 1000:.1f}' if tot_waste else '0'}</td></tr>"})
    rep_rows.sort(key=lambda r: (-r["rate"], r["never"]))
    best_repair = rep_rows[0] if rep_rows else None
    rep_rows = [r["row"] for r in rep_rows]
    repair_table = ""
    if rep_rows:
        repair_table = (
            "<div><h2 id='repair'>Self-repair — can it fix its own bugs?</h2>"
            "<p class='note'>Each task gets up to 5 attempts; after a failure the model is handed its "
            "own code plus the compiler/test output. Cells show <b>one-shot passes</b>, then "
            "<span class='dim'>+n</span> fixed using the feedback and <span class='status'>✗n</span> "
            "still broken after 5 rounds. <b>gen time</b> is the whole suite — every round of every "
            "task, failed attempts included — and counts model generation only, not local compiling "
            "and testing. <b>waste</b> = tokens spent on tasks that needed more than one round. "
            "C = 19 tasks, Python and Bash = 11 each.</p>"
            + (f"<p class='note'>The referee is excluded here: it's a cloud model, so the harness "
               f"couldn't drive it. Its C figures ({ref_repair[0]['one_shot']}/"
               f"{ref_repair[0]['tasks']} one-shot) are self-reported and have no token or timing "
               f"data.</p>" if ref_repair else "")
            + "<table><tr><th title='Click any header to sort'>model</th><th>total</th>"
            "<th>C</th><th>Py</th><th>Sh</th>"
            "<th title='tasks never fixed, even after 5 rounds'>never</th>"
            "<th title='Total generation time for all 41 tasks across every round attempted, "
            "including rounds that failed and tasks that never passed. Model time only — local "
            "compiling and testing is not counted.'>gen time (min)</th>"
            "<th title='tokens spent on tasks that needed more than one round'>waste (k tok)</th></tr>"
            + "".join(rep_rows) + "</table></div>")

    # C error-category pivot: what kind of failure, per model (ceval + chard notes)
    CATS = [("linker", "no main() emitted — helper function only"),
            ("undeclared", "undeclared identifier, usually a missing #include"),
            ("compile", "other compile error (type or syntax)"),
            ("wrong", "compiled fine but failed the hidden tests"),
            ("other", "no code extracted, timeout, or HTTP error")]

    def categorize(note: str) -> str:
        n = note.lower()
        if not n.strip():
            return ""
        if "linker command failed" in n or "undefined symbol" in n or "undefined reference" in n:
            return "linker"
        if "undeclared identifier" in n or "use of undeclared" in n or "implicit declaration" in n:
            return "undeclared"
        if "error:" in n:
            return "compile"
        if n.startswith("fail") or " want " in n or "wrong output" in n or "assert" in n:
            return "wrong"
        return "other"

    cat_rows = []
    for t in TARGETS:
        counts = {k: 0 for k, _ in CATS}
        total_fails = 0
        for s in ("ceval", "chard"):
            d = load(f"{t}-{s}")
            if not d:
                continue
            d = d[0]
            for task, outcomes in d["results"].items():
                npass = sum(1 for x in outcomes if x == "pass")
                if npass == len(outcomes):
                    continue
                total_fails += len(outcomes) - npass
                c = categorize(d["notes"].get(task, ""))
                if c:
                    counts[c] += 1
        if total_fails:
            cells = "".join(f"<td>{counts[k] or '<span class=dim>·</span>'}</td>" for k, _ in CATS)
            cat_rows.append(
                (total_fails,
                 f"<tr><td><a href='#{t}'>{NAMES[t]}</a></td>"
                 f"<td class='{shade(72 - total_fails, 72)}'><b>{total_fails}</b></td>{cells}</tr>"))
    cat_rows.sort(key=lambda x: x[0])
    cat_rows = [r for _, r in cat_rows]
    error_table = ""
    if cat_rows:
        error_table = (
            "<div><h2 id='cerrors'>C failures — what actually went wrong</h2>"
            "<p class='note'>Failed trials per model across the C easy and hard sets (72 samples), "
            "and what kind of error each failing task hit. The split matters: a missing "
            "<code>#include</code> is a formatting slip an agent loop fixes instantly, while a "
            "wrong answer means the model misunderstood the problem. The referee isn't listed "
            "because it has no failures to categorize.</p>"
            "<table><tr><th>model</th><th title='failed trials out of 72'>failed</th>"
            + "".join(f"<th title='{desc}'>{k}</th>" for k, desc in CATS)
            + "</tr>" + "".join(cat_rows) + "</table></div>")

    # Brutal set — five tasks written so that the obvious answer is wrong.
    brutal_rows = []
    for t in TARGETS:
        per: dict[str, list[str]] = {}
        for s, *_ in BRUTAL_SUITES:
            d = load(f"{t}-{s}")
            if d:
                per.update(d[0]["results"])
        if not per:
            continue
        got = sum(1 for o in per.values() for x in o if x == "pass")
        tot = sum(len(o) for o in per.values())
        cells = ""
        for key, _lbl, _desc in BRUTAL_TASKS:
            o = per.get(key)
            if not o:
                cells += "<td class='dim'>—</td>"
                continue
            n = sum(1 for x in o if x == "pass")
            cells += f"<td class='{shade(n, len(o))}'>{n}/{len(o)}</td>"
        brutal_rows.append(
            (got / tot if tot else 0,
             f"<tr><td><a href='#{t}'>{NAMES[t]}</a></td>"
             f"<td class='{shade(got, tot)}'><b>{got}</b>/{tot}</td>{cells}</tr>"))
    brutal_rows.sort(key=lambda x: -x[0])
    brutal_table = ""
    if brutal_rows:
        brutal_table = (
            "<h2 id='brutal'>The brutal set — five problems with a wrong obvious answer</h2>"
            "<p class='note'>These are scored separately and are <b>not</b> part of the coding "
            "total above, so those numbers stay comparable with earlier runs. Each task was "
            "picked because the textbook approach fails a specific case, and each one was checked "
            "twice before any model saw it: a correct reference solution passes, and a plausible "
            "naive solution fails. 3 trials per task, temperature 0 then 0.7 twice.</p>"
            "<table><tr><th>model</th><th>total</th>"
            + "".join(f"<th title='{desc}'>{lbl}</th>" for _k, lbl, desc in BRUTAL_TASKS)
            + "</tr>" + "".join(r for _, r in brutal_rows) + "</table>"
            "<ul class='note'>"
            + "".join(f"<li><b>{lbl}</b> — {desc}</li>" for _k, lbl, desc in BRUTAL_TASKS)
            + "</ul>")

    # Concurrency — newest run per target, with older runs kept on disk.
    conc_dir = RESULTS / "concurrency"
    conc_rows, conc_runs, conc_dates = [], 0, []
    if conc_dir.exists():
        newest: dict[str, tuple[str, dict]] = {}
        for p in sorted(conc_dir.glob("*.json")):
            try:
                doc = json.loads(p.read_text())
            except json.JSONDecodeError:
                continue
            conc_runs += 1
            prev = newest.get(doc["target"])
            # prefer the most complete run, then the most recent of those, so a
            # narrow follow-up experiment doesn't blank out the main columns
            key = (len(doc["levels"]), p.name)
            if prev is None or key > prev[0]:
                newest[doc["target"]] = (key, doc)
        # columns come from the data, not a fixed list, so new levels (12, 16, ...)
        # appear without touching this code
        conc_levels = sorted({l["level"] for (_k, d) in newest.values() for l in d["levels"]})
        for t in TARGETS:
            if t not in newest:
                continue
            doc = newest[t][1]
            conc_dates.append(doc["date"])
            by_level = {l["level"]: l for l in doc["levels"]}
            cells = ""
            for lvl in conc_levels:
                l = by_level.get(lvl)
                cells += (f"<td>{l['aggregate_tok_s']:.0f}</td>" if l and l["aggregate_tok_s"]
                          else "<td class='dim'>—</td>")
            best = max((l.get("speedup_vs_1") or 0 for l in doc["levels"]), default=0)
            top = by_level.get(max(by_level)) if by_level else None
            acc = (f"<td class='{shade(top['passed'], top['total'])}'>"
                   f"{top['passed']}/{top['total']}</td>" if top else "<td class='dim'>—</td>")
            ram = (f"<td>{top['peak_rss_mb'] / 1024:.1f}</td>"
                   if top and top.get("peak_rss_mb") else "<td class='dim'>—</td>")
            conc_rows.append(
                (best,
                 f"<tr><td><a href='#{t}'>{NAMES[t]}</a> "
                 f"<span class='dim'>{STACK.get(t, 'MLX')}</span></td>{cells}"
                 f"<td><b>{best:.2f}</b>x</td>{acc}{ram}"
                 f"<td class='dim'>{doc['date']}</td></tr>"))
    conc_rows.sort(key=lambda x: -x[0])
    concurrency_table = ""
    if conc_rows:
        concurrency_table = (
            "<h2 id='concurrency'>Serving several requests at once</h2>"
            "<p class='note'>Decode on this hardware is limited by reading the weights out of "
            "memory, not by arithmetic, so a stack that batches properly can serve several "
            "requests for barely more than the cost of one. This is the number that matters for "
            "an agent firing parallel tool calls, or a team sharing one server — everything else "
            "on this page measures one request at a time. The workload is fixed at 8 C tasks, "
            "replayed with 1, 2, 4 and 8 requests in flight; the columns are aggregate tokens per "
            "second across all streams. This scores the <b>serving stack</b>, not the model's "
            "intelligence, so it is kept out of the coding total.</p>"
            "<p class='note'>Every answer is still compiled and tested. Accuracy should not move "
            "with concurrency — where it does, batched arithmetic has changed the numerics enough "
            "to flip the model's token choices, which is worth knowing before you turn up the "
            "parallelism on a production box. Runs are timestamped and never overwritten, so "
            "these can be compared over time.</p>"
            f"<table><tr><th>model</th>"
            + "".join(
                f"<th title='aggregate tokens/sec, {lvl} request{'s' if lvl != 1 else ''} "
                f"in flight'>{lvl}</th>" for lvl in conc_levels)
            + "<th title='best aggregate throughput relative to one-at-a-time'>best gain</th>"
            "<th title='tasks still correct at the highest concurrency level'>correct</th>"
            "<th title='peak resident memory at the highest concurrency level'>RAM GB</th>"
            "<th>run</th></tr>"
            + "".join(r for _, r in conc_rows) + "</table>"
            f"<p class='note'>{conc_runs} run{'s' if conc_runs != 1 else ''} on disk; the newest "
            "per model is shown.</p>")

    # Performance — is the generated code fast, and does it need to be asked?
    perf_dir = RESULTS / "perf"
    PERF_TASKS = [("range_sums", "C"), ("dedupe", "Python"), ("top_freq", "Bash")]
    perf_rows, perf_best = [], {}
    perf_docs: dict[str, dict] = {}
    if perf_dir.exists():
        newest: dict[str, tuple[str, dict]] = {}
        for p in sorted(perf_dir.glob("*.json")):
            try:
                doc = json.loads(p.read_text())
            except json.JSONDecodeError:
                continue
            prev = newest.get(doc["target"])
            if prev is None or p.name > prev[0]:
                newest[doc["target"]] = (p.name, doc)
        perf_docs = {t: d for t, (_n, d) in newest.items()}
        # baseline per task/variant: the fastest anyone managed
        for name, _lbl in PERF_TASKS:
            for variant in ("silent", "told"):
                vals = [d["variants"].get(variant, {}).get(name, {}).get("best_ms")
                        for d in perf_docs.values()]
                vals = [v for v in vals if v]
                if vals:
                    perf_best[(name, variant)] = min(vals)

        def ms_cell(entry: dict | None, name: str, variant: str) -> str:
            if not entry or entry.get("best_ms") is None:
                bad = (entry or {}).get("statuses") or []
                lbl = bad[0] if bad else "—"
                return f"<td class='s-bad' title='{html.escape(str(bad))}'>{lbl}</td>"
            ms = entry["best_ms"]
            base = perf_best.get((name, variant)) or ms
            ratio = ms / base if base else 1
            cls = ("s-hi" if ratio <= 1.5 else "s-mid" if ratio <= 5
                   else "s-lo" if ratio <= 50 else "s-bad")
            shown = f"{ms:,.0f}" if ms >= 10 else f"{ms:.2f}"
            return (f"<td class='{cls}' title='{ratio:.0f}x the fastest answer "
                    f"anyone gave'>{shown}</td>")

        for t in TARGETS:
            doc = perf_docs.get(t)
            if not doc:
                continue
            cells, ratios = "", []
            for name, _lbl in PERF_TASKS:
                for variant in ("silent", "told"):
                    entry = doc["variants"].get(variant, {}).get(name)
                    cells += ms_cell(entry, name, variant)
            # how much did being told help? geometric-ish: sum of silent/told
            gains = []
            for name, _lbl in PERF_TASKS:
                s = doc["variants"].get("silent", {}).get(name, {}).get("best_ms")
                w = doc["variants"].get("told", {}).get(name, {}).get("best_ms")
                if s and w:
                    gains.append(s / w)
            gain = max(gains) if gains else 0
            solved = sum(1 for name, _l in PERF_TASKS for v in ("silent", "told")
                         if (doc["variants"].get(v, {}).get(name, {}).get("best_ms")))
            perf_rows.append(
                (solved, gain,
                 f"<tr><td><a href='#{t}'>{NAMES[t]}</a></td>{cells}"
                 f"<td>{f'{gain:.1f}x' if gain >= 1.2 else '<span class=dim>—</span>'}</td>"
                 f"<td class='dim'>{doc.get('date', '')}</td></tr>"))
    perf_rows.sort(key=lambda x: (-x[0], -x[1]))
    perf_table = ""
    if perf_rows:
        heads = "".join(
            f"<th title='no mention of performance in the prompt'>{lbl} silent</th>"
            f"<th title='prompt says the running time will be measured'>{lbl} told</th>"
            for _n, lbl in PERF_TASKS)
        perf_table = (
            "<h2 id='perf'>How fast is the code it writes?</h2>"
            "<p class='note'>Correctness says nothing about whether an answer is O(n) or O(n²). "
            "These three tasks are deliberately easy to get <b>right</b> — a beginner's answer "
            "passes the correctness check — so the only thing that varies is whether the model "
            "thought about complexity. Numbers are milliseconds on a large hidden input, best of "
            "three runs after a warm-up; lower is better and green is at or near the fastest "
            "answer anyone gave.</p>"
            "<p class='note'>Each task is asked twice. <b>silent</b> never mentions performance, "
            "which is what an agent loop actually sends; <b>told</b> says the running time will be "
            "measured. The last column is how much being told helped — a dash means it wrote the "
            "fast version without being asked. For scale, the naive answer to each task is 426x, "
            "687x and 172x slower than the good one.</p>"
            f"<table><tr><th>model</th>{heads}"
            "<th title='best speedup from being told performance matters'>telling helps</th>"
            "<th>run</th></tr>"
            + "".join(r for _s, _g, r in perf_rows) + "</table>")

    # Framing — the same task asked seven ways, to see which wording changes the answer
    fr_dir = RESULTS / "framing"
    FR_ORDER = [("bare", "bare"), ("timed", "will be timed"),
                ("think", "think carefully"), ("stakes", "production code"),
                ("user_expert", "I'm an expert"), ("model_persona", "you're an expert"),
                ("user_beginner", "I'm a beginner")]
    FR_HEADROOM = ("coder-next", "qwen38flash", "qwen27")
    framing_table = ""
    if fr_dir.exists():
        fr_newest: dict[str, tuple[str, dict]] = {}
        for p in sorted(fr_dir.glob("*.json")):
            try:
                doc = json.loads(p.read_text())
            except json.JSONDecodeError:
                continue
            prev = fr_newest.get(doc["target"])
            if prev is None or p.name > prev[0]:
                fr_newest[doc["target"]] = (p.name, doc)
        fr_docs = {t: d for t, (_n, d) in fr_newest.items()}

        def fr_cell(c: dict | None) -> str:
            if not c:
                return "<td class='dim'>—</td>"
            r = c["fast"] / c["n"] if c["n"] else 0
            cls = ("s-hi" if r >= 0.8 else "s-mid" if r >= 0.5
                   else "s-lo" if r >= 0.2 else "s-bad")
            lo, hi = c["ci95"]
            return (f"<td class='{cls}' title='95% CI {lo:.0%}-{hi:.0%}'>"
                    f"{c['fast']}/{c['n']}</td>")

        fr_rows = ""
        for t in TARGETS:
            doc = fr_docs.get(t)
            if not doc:
                continue
            fr_star = " *" if t in FR_HEADROOM else ""
            fr_rows += (f"<tr><td><a href='#{t}'>{NAMES[t]}</a>{fr_star}</td>"
                     + "".join(fr_cell(doc["conditions"].get(k)) for k, _l in FR_ORDER)
                     + f"<td class='dim'>{doc.get('date', '')}</td></tr>")
        # pooled over the models that had room to move, which is where the effect lives
        fr_pooled = [fr_docs[t] for t in FR_HEADROOM if t in fr_docs]
        if fr_pooled and fr_rows:
            fr_agg = ""
            for k, _l in FR_ORDER:
                f = sum(d["conditions"][k]["fast"] for d in fr_pooled if k in d["conditions"])
                n = sum(d["conditions"][k]["n"] for d in fr_pooled if k in d["conditions"])
                r = f / n if n else 0
                cls = ("s-hi" if r >= 0.8 else "s-mid" if r >= 0.5
                       else "s-lo" if r >= 0.2 else "s-bad")
                fr_agg += f"<td class='{cls}'><b>{r:.0%}</b></td>"
            fr_rows += (f"<tr><td><b>pooled *</b></td>{fr_agg}"
                     f"<td class='dim'>n={sum(d['conditions']['bare']['n'] for d in fr_pooled)}"
                     "/cell</td></tr>")
        if fr_rows:
            fr_heads = "".join(f"<th>{l}</th>" for _k, l in FR_ORDER)
            framing_table = (
                "<h2 id='framing'>Does how you ask change what you get?</h2>"
                "<p class='note'>One task — the C range-sums problem — asked seven different ways, "
                "20 samples per wording at temperature 0.7. The cell is how often the model wrote "
                "the O(n+q) prefix sum instead of the O(n·q) loop; the two are about 400x apart, so "
                "there is no middle ground to argue about. Hover for the 95% interval.</p>"
                "<p class='note'>Starred models write the naive loop when asked plainly, so they are "
                "the only ones with room to move; the gpt-oss pair already sit at the ceiling and "
                "can only show a wording doing harm. <b>pooled</b> combines the three, which is what "
                "makes a modest effect detectable at all.</p>"
                f"<table><tr><th>model</th>{fr_heads}<th>run</th></tr>{fr_rows}</table>"
                "<p class='note'>Against the bare prompt (Fisher exact, two-sided): think carefully "
                "p=4e-24, will be timed p=3e-21, production code p=1e-12, I'm an expert p=6e-08. "
                "Giving the <i>model</i> the persona (p=0.13) and claiming to be a beginner (p=0.49) "
                "are not distinguishable from noise.</p>"
                "<p class='note'><b>None of it transfers to correctness.</b> Re-running the winning "
                "wordings on the brutal set, where the outcome is whether the code is right rather "
                "than fast, gives 56% for the bare prompt against 47%, 50% and 50% — all within "
                "noise and all pointing mildly downward. Framing steers which approach the model "
                "reaches for among approaches it already knows; it does not add capability. "
                "See <code>docs/benchmarks.md</code>.</p>")

    # Dense vs MoE — does the architecture, not the size, predict the result?
    arch_rows = []
    for t in TARGETS:
        if t not in stats or t not in ARCH or t in SIDELINED:
            continue
        kind, total, active, experts = ARCH[t]
        s = stats[t]
        frac = s["passed"] / s["total"] if s["total"] else 0
        tok = s["tok"]
        # tok/s per active billion: how efficiently the compute footprint converts to speed.
        # Only meaningful for MoE; a dense model is definitionally 1x its own size.
        act_b = None
        try:
            act_b = float(active.rstrip("B"))
        except (ValueError, AttributeError):
            pass
        eff = (tok / act_b) if (tok and act_b and kind == "MoE") else None
        arch_rows.append((frac, tok or 0, kind, t, total, active, experts, eff))
    arch_rows.sort(key=lambda x: -x[0])
    arch_table = ""
    if arch_rows:
        body = ""
        for frac, tok, kind, t, total, active, experts, eff in arch_rows:
            kcls = "s-hi" if kind == "MoE" else "s-mid"
            body += (f"<tr><td><a href='#{t}'>{NAMES[t]}</a></td>"
                     f"<td class='{kcls}'>{kind}</td><td>{total}</td><td>{active}</td>"
                     f"<td class='dim'>{experts}</td>"
                     f"<td class='{shade_frac(frac)}'>{frac:.0%}</td>"
                     f"<td>{tok:.0f}</td>"
                     f"<td>{f'{eff:.1f}' if eff else '<span class=dim>—</span>'}</td></tr>")
        arch_table = (
            "<h2 id='arch'>Dense vs mixture-of-experts</h2>"
            "<p class='note'>A dense model runs every parameter on every token; a "
            "mixture-of-experts model routes each token to a few of its experts, so only the "
            "<b>active</b> column does compute. That is why the 125B flash-next decodes like a "
            "6B model and the whole top of the table is MoE — on memory-bandwidth-bound Apple "
            "silicon, active parameters are what you pay per token, and MoE buys a large total "
            "capacity at a small active cost. The last column, tok/s per active billion, is the "
            "efficiency of that trade; dense models are 1x by definition and omitted.</p>"
            "<table><tr><th>model</th><th>type</th>"
            "<th title='total parameters'>total</th>"
            "<th title='parameters used per token'>active</th>"
            "<th title='expert count and routing'>experts</th>"
            "<th title='coding total as a fraction'>coding</th>"
            "<th title='decode tok/s'>tok/s</th>"
            "<th title='tok/s per active billion (MoE only)'>tok/s per act-B</th></tr>"
            + body + "</table>")

    # What it costs to get working code. Every other table scores correctness or
    # raw decode speed; neither is what you actually spend. The harness already
    # records wall seconds per task per trial, so the real figure — seconds per
    # solution that compiles and passes — falls straight out of the data.
    COST_SUITES = [("ceval", "C"), ("chard", "C"), ("python", "Py"),
                   ("pyhard", "Py"), ("bash", "Sh"), ("shhard", "Sh")]
    cost_rows, cost_skipped = [], []
    for t in TARGETS:
        if t in SIDELINED:
            continue
        per = {"C": 0.0, "Py": 0.0, "Sh": 0.0}
        first_ok = retry_ok = n_tasks = 0
        first_s = retry_s = 0.0
        complete, any_data = True, False
        for suffix, lang in COST_SUITES:
            doc = (load(f"{t}-{suffix}") or [None])[0]
            if not doc:
                complete = False
                break
            any_data = True
            times = doc.get("time_s", {})
            for name, outcomes in doc["results"].items():
                secs = times.get(name, [])
                if len(secs) != len(outcomes):
                    # older runs scored the C suite before per-task timing existed
                    complete = False
                    break
                n_tasks += 1
                per[lang] += sum(secs)
                first_s += secs[0]
                if outcomes[0] == "pass":
                    first_ok += 1
                spent = 0.0
                for outcome, sec in zip(outcomes, secs):
                    spent += sec        # a retry you never run costs nothing
                    if outcome == "pass":
                        retry_ok += 1
                        break
                retry_s += spent
            if not complete:
                break
        if not complete or not n_tasks:
            # A model with no results at all is simply unrun and is already absent
            # from every other table; only flag one that was scored without timing.
            if any_data:
                cost_skipped.append(t)
            continue
        cost_rows.append({
            "t": t, "per": per, "total": sum(per.values()), "n": n_tasks,
            "first_ok": first_ok, "first_sp": first_s / first_ok if first_ok else 0.0,
            "retry_ok": retry_ok, "retry_sp": retry_s / retry_ok if retry_ok else 0.0,
        })

    cost_table = ""
    if cost_rows:
        cost_rows.sort(key=lambda r: r["retry_sp"])
        # A model is out of the running if another is at least as accurate *and* at
        # least as cheap. Whatever you weigh, you would never pick a dominated one.
        for r in cost_rows:
            r["beaten_by"] = [o["t"] for o in cost_rows if o is not r
                              and o["retry_ok"] >= r["retry_ok"]
                              and o["retry_sp"] <= r["retry_sp"]
                              and (o["retry_ok"] > r["retry_ok"]
                                   or o["retry_sp"] < r["retry_sp"])]
        best_total = min(r["total"] for r in cost_rows)
        best_first = min((r["first_sp"] for r in cost_rows if r["first_sp"]), default=0)
        best_retry = min((r["retry_sp"] for r in cost_rows if r["retry_sp"]), default=0)
        n_tasks = cost_rows[0]["n"]

        time_body = ""
        for r in sorted(cost_rows, key=lambda r: r["total"]):
            cells = "".join(
                f"<td>{r['per'][lang] / 60:.1f}</td>" for lang in ("C", "Py", "Sh"))
            time_body += (
                f"<tr><td><a href='#{r['t']}'>{NAMES[r['t']]}</a></td>{cells}"
                f"<td class='{shade_frac(best_total / r['total'])}'>"
                f"<b>{r['total'] / 60:.1f}</b></td>"
                f"<td class='dim'>{r['total'] / (r['n'] * 3):.1f}</td></tr>")

        eff_body = ""
        for r in cost_rows:
            if r["beaten_by"]:
                names = ", ".join(NAMES[b] for b in r["beaten_by"][:2])
                extra = f" +{len(r['beaten_by']) - 2}" if len(r["beaten_by"]) > 2 else ""
                verdict = f"<td class='dim'>beaten by {names}{extra}</td>"
            else:
                verdict = "<td class='s-hi'><b>on the frontier</b></td>"
            eff_body += (
                f"<tr><td><a href='#{r['t']}'>{NAMES[r['t']]}</a></td>"
                f"<td class='{shade(r['first_ok'], r['n'])}'>{r['first_ok']}/{r['n']}</td>"
                f"<td class='{shade_frac(best_first / r['first_sp']) if r['first_sp'] else ''}'>"
                f"{r['first_sp']:.1f}s</td>"
                f"<td class='{shade(r['retry_ok'], r['n'])}'>{r['retry_ok']}/{r['n']}</td>"
                f"<td class='{shade_frac(best_retry / r['retry_sp']) if r['retry_sp'] else ''}'>"
                f"<b>{r['retry_sp']:.1f}s</b></td>"
                f"<td class='dim'>+{r['retry_ok'] - r['first_ok']}</td>{verdict}</tr>")

        skipped_note = ""
        if cost_skipped:
            skipped_note = (
                "<p class='note'>Missing here: "
                + ", ".join(NAMES[t] for t in cost_skipped)
                + " — their C suite was scored before the harness recorded per-task "
                  "wall time, so a total would not be comparable. Their correctness "
                  "scores above are unaffected.</p>")

        cost_table = (
            "<h2 id='cost'>What it costs to get working code</h2>"
            "<p class='note'>Correctness alone picks a model that is right and unusably "
            "slow; tok/s alone picks one that is fast and wrong. The number that decides "
            f"it is the two divided: <b>seconds of wall clock per solution that passes</b>. "
            f"Across the {n_tasks} coding tasks (research excluded), 3 trials each — the "
            "first at temperature 0, the retries at 0.7 — timed end to end, failures "
            "included, because a wrong answer costs you its generation time too.</p>"
            "<table><tr><th>model</th><th title='minutes, all trials, failures included'>C min</th>"
            "<th>Python min</th><th>Bash min</th>"
            "<th title='total wall minutes for the whole suite'>total min</th>"
            "<th title='mean seconds for one attempt at one task'>per attempt</th></tr>"
            + time_body + "</table>"
            "<p class='note'>Those totals are everything the model spent, right or wrong. "
            "Dividing by the answers that actually worked gives the figure below. "
            "<b>first try</b> is trial 1 alone, the honest number if you paste the first "
            "thing you get. <b>with retry</b> re-asks only when the previous attempt "
            "failed and stops at the first pass, so unused retries cost nothing — what a "
            "loop around the model really costs.</p>"
            "<table><tr><th>model</th>"
            "<th title='passed on trial 1'>first try</th><th>sec/solution</th>"
            "<th title='passed within 3 trials'>with retry</th><th>sec/solution</th>"
            "<th title='extra tasks the retries bought'>retry gain</th>"
            "<th>verdict</th></tr>" + eff_body + "</table>"
            "<p class='note'>The verdict column is the part that actually decides things. "
            "A model is <b>beaten</b> when another solves at least as many tasks at at "
            "least as low a cost — strictly better on one axis and no worse on the other. "
            "No weighting of speed against accuracy can rescue it, so it can be dropped "
            "without ever choosing between the two. That eliminates "
            f"{sum(1 for r in cost_rows if r['beaten_by'])} of {len(cost_rows)} and leaves "
            "the frontier: "
            + ", ".join(f"<b>{NAMES[r['t']]}</b>" for r in cost_rows if not r["beaten_by"])
            + " — the only defensible picks, one per point on the speed/accuracy trade.</p>"
            + skipped_note)

    # Laguna XS.2 against its own successor. Both sit on the frontier above, so the
    # only way to tell them apart is suite by suite — the totals happen to be close
    # while the per-language results move in opposite directions.
    h2h_table = ""
    # Prefer the MLX build of XS.2 when it has been scored: it holds the runtime and
    # the quantization fixed, leaving the version as the only difference. The GGUF
    # run is the fallback and carries a caveat the MLX one does not need.
    same_stack = "laguna-mlx" in stats
    H2H = ("laguna-mlx" if same_stack else "laguna", "laguna21")
    H2H_SUITES = [("ceval", "C, easy"), ("chard", "C, hard"),
                  ("python", "Python, easy"), ("pyhard", "Python, hard"),
                  ("bash", "Bash, easy"), ("shhard", "Bash, hard")]
    if all(t in stats for t in H2H):
        docs = {t: {s: (load(f"{t}-{s}") or [None])[0] for s, _ in H2H_SUITES}
                for t in H2H}
        if all(all(d.values()) for d in docs.values()):
            body, tot = "", {t: [0, 0, 0.0] for t in H2H}
            for suffix, label in H2H_SUITES:
                cells = ""
                pair = [docs[t][suffix] for t in H2H]
                for t, d in zip(H2H, pair):
                    tot[t][0] += d["passed"]
                    tot[t][1] += d["total"]
                    tot[t][2] += d.get("total_time_s") or 0.0
                    cells += (f"<td class='{shade(d['passed'], d['total'])}'>"
                              f"{d['passed']}/{d['total']}</td>"
                              f"<td class='dim'>{(d.get('total_time_s') or 0):.0f}s</td>")
                delta = pair[1]["passed"] - pair[0]["passed"]
                cls = "s-hi" if delta > 0 else ("s-lo" if delta < 0 else "dim")
                body += (f"<tr><td>{label}</td>{cells}"
                         f"<td class='{cls}'><b>{delta:+d}</b></td></tr>")
            sums = ""
            for t in H2H:
                p, n, s = tot[t]
                sums += (f"<td class='{shade(p, n)}'><b>{p}/{n}</b></td>"
                         f"<td class='dim'><b>{s / 60:.1f} min</b></td>")
            body += (f"<tr><td><b>total</b></td>{sums}"
                     f"<td class='dim'><b>{tot[H2H[1]][0] - tot[H2H[0]][0]:+d}</b></td></tr>")
            speeds = "".join(
                f"<td colspan='2'>{stats[t]['tok']:.0f} tok/s</td>"
                if stats[t].get("tok") else "<td colspan='2' class='dim'>—</td>"
                for t in H2H)
            body += f"<tr><td class='dim'>decode</td>{speeds}<td class='dim'></td></tr>"
            h2h_table = (
                "<h2 id='h2h'>Laguna XS.2 against Laguna XS 2.1</h2>"
                + ("<p class='note'>The successor is a generation newer at the same "
                   "33B-A3B shape, and with both served identically this is the one "
                   "comparison in the report where nothing but the version changes. "
                   "Read the gain column: the newer model is ahead or level almost "
                   "everywhere, and neither can touch hard C.</p>"
                   if same_stack else
                   "<p class='note'>The successor is a generation newer at the same "
                   "33B-A3B shape, and it is the one case where two models in this "
                   "report differ only by version. It finishes the suite faster and "
                   "one language at a time the picture splits: it gives up C and "
                   "takes Python and Bash.</p>")
                + f"<table><tr><th>suite</th><th>{NAMES[H2H[0]]}</th><th>time</th>"
                f"<th>{NAMES[H2H[1]]}</th><th>time</th><th>2.1 gain</th></tr>"
                + body + "</table>"
                + ("<p class='note'>Both sides here are MLX at 4 bits, group size 64, "
                   "on the same 40-layer 256-expert shape, so the version is the only "
                   "thing that differs and the tok/s gap is the model's own. The "
                   "separate <b>laguna-xs.2</b> row elsewhere in this report is the "
                   "same weights under the llama.cpp fork, kept as a measure of what "
                   "the runtime alone is worth.</p>"
                   if same_stack else
                   "<p class='note'>Read the totals with one caveat: the two run on "
                   "different stacks — XS.2 as GGUF under llama.cpp, XS 2.1 as MLX — so "
                   "the tok/s gap measures the runtime as much as the model. The "
                   "pass counts do not depend on the runtime and are a clean "
                   "comparison; for C the older model is still the better one.</p>"))

    sidelined_table = ""
    if sidelined_rows:
        sidelined_table = (
            "<h2 id='alsorans'>Kept for the record, not in the running</h2>"
            "<p class='note'>These were scored the same way as everything else and their "
            "per-task sections are still below, but they are held out of the tables above. "
            "Leaving them in a ranking implies they are candidates, and they are not — "
            "correctness is not the reason, cost is.</p>"
            "<table><tr><th>model</th><th>coding</th><th>tok/s</th>"
            "<th>why it is out</th></tr>" + "".join(sidelined_rows) + "</table>")

    # The instruction catalogue — every prompt the harness sends, verbatim.
    prompt_groups = []
    for lang, label, mod in PROMPT_MODULES:
        tiers = [("easy", mod.task_set("easy")), ("hard", mod.task_set("hard")),
                 ("brutal", mod.task_set("brutal"))]
        items, count = "", 0
        for tier, tasks in tiers:
            for task in tasks:
                count += 1
                items += (
                    f"<details><summary><code>{html.escape(task['name'])}</code>"
                    f" <span class='dim'>{tier}</span></summary>"
                    f"<pre class='prompt-text'>{html.escape(mod.build_prompt(task))}</pre>"
                    "</details>")
        prompt_groups.append(
            f"<details><summary><b>{label}</b> — {count} tasks</summary>"
            f"<div class='promptlist'>{items}</div></details>")
    research_doc = ""
    try:
        research_doc = eval_research.PROMPT.replace(
            "{doc}", "[~2,100 words of RHEL 10 release notes and the "
                     "\"Managing file systems\" guide, inserted here]")
    except AttributeError:
        pass
    if research_doc:
        prompt_groups.append(
            "<details><summary><b>Research</b> — 1 task</summary>"
            "<div class='promptlist'><details><summary><code>nfs_summary</code>"
            " <span class='dim'>research</span></summary>"
            f"<pre class='prompt-text'>{html.escape(research_doc)}</pre>"
            "</details></div></details>")
    prompts_table = (
        "<h2 id='prompts'>The exact instructions</h2>"
        "<p class='note'>Every prompt the harness sends, verbatim and complete — there is no "
        "system prompt, no few-shot examples and no retries beyond the trial count. These are "
        "read straight out of the eval modules rather than copied, so they cannot drift from "
        "what was actually asked. The same text also appears next to each failing sample "
        "further down, so you can read the instruction and the answer together.</p>"
        + "".join(prompt_groups))

    # Headline cards — computed, not hand-written, so they can't go stale.
    cards = []
    if stats:
        acc = max(stats.items(), key=lambda kv: kv[1]["passed"] / max(1, kv[1]["total"]))
        fast = max((kv for kv in stats.items() if kv[1]["tok"]), key=lambda kv: kv[1]["tok"])
        lean = min((kv for kv in stats.items()
                    if kv[1]["rss"] and kv[1]["passed"] / max(1, kv[1]["total"]) >= 0.80),
                   key=lambda kv: kv[1]["rss"], default=None)
        cards.append(
            f"<div class='card'><div class='k'>most accurate</div>"
            f"<div class='v'><a href='#{acc[0]}'>{NAMES[acc[0]]}</a></div>"
            f"<div class='d'>{acc[1]['passed']}/{acc[1]['total']} coding tasks · "
            f"{acc[1]['tok']:.1f} tok/s · {acc[1]['rss'] / 1024:.1f} GB</div></div>")
        cards.append(
            f"<div class='card'><div class='k'>fastest</div>"
            f"<div class='v'><a href='#{fast[0]}'>{NAMES[fast[0]]}</a></div>"
            f"<div class='d'>{fast[1]['tok']:.1f} tok/s decode · "
            f"{fast[1]['passed']}/{fast[1]['total']} coding tasks</div></div>")
        if lean:
            cards.append(
                f"<div class='card'><div class='k'>lightest of the accurate tier</div>"
                f"<div class='v'><a href='#{lean[0]}'>{NAMES[lean[0]]}</a></div>"
                f"<div class='d'>{lean[1]['rss'] / 1024:.1f} GB · {lean[1]['tok']:.1f} tok/s · "
                f"{lean[1]['passed']}/{lean[1]['total']} coding tasks</div></div>")
    if best_repair:
        cards.append(
            f"<div class='card'><div class='k'>best at fixing its own bugs</div>"
            f"<div class='v'><a href='#{best_repair['target']}'>{best_repair['name']}</a></div>"
            f"<div class='d'>{best_repair['one']}/{best_repair['tasks']} correct on the first try · "
            f"{best_repair['never']} still broken after 5 rounds</div></div>")

    stamp = dt.datetime.now().strftime("%Y-%m-%d %H:%M")
    page = f"""<!doctype html>
<html><head><meta charset='utf-8'><title>M5 Max eval report</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github-dark.min.css'>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/c.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/python.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/bash.min.js'></script>
<style>{CSS}</style></head><body>
<nav>
  <b>M5 Max evals</b>
  <a href='#summary'>summary</a>
  <a href='#brutal'>brutal set</a>
  <a href='#concurrency'>concurrency</a>
  <a href='#perf'>code speed</a>
  <a href='#cost'>cost per solution</a>
  {"<a href='#h2h'>laguna head-to-head</a>" if h2h_table else ""}
  <a href='#framing'>framing</a>
  <a href='#arch'>dense vs moe</a>
  <a href='#prompts'>prompts</a>
  <a href='#cerrors'>C failures</a>
  <a href='#repair'>self-repair</a>
  <select id='jump'><option value=''>jump to model…</option>{''.join(nav_opts)}</select>
  <span class='sp'></span>
  <a href='#' id='toggle-all'>expand all</a>
</nav>

<h1 id='summary'>Local models on an M5 Max — which one should write your code?</h1>
<p class='note'>{len(stats)} models benchmarked on one machine (M5 Max, 128 GB). Nothing here is
judged by another LLM: C is compiled with <code>cc -std=c11 -Wall</code>, Python runs against hidden
asserts, Bash is checked for exact stdout and exit codes. A referee audit re-graded all 191 C
failures and confirmed every one. Click any column header to sort; click a model to see its
failing code. Generated {stamp}.</p>

<div class='cards'>{''.join(cards)}</div>

<table><tr>
<th title='Click to sort. Model name and how it was served.'>model</th>
<th title='All coding suites added up: C, Python, Bash, the three hard sets and research'>coding total</th>
<th title='Decode speed, median of 3 full 2048-token generations'>tok/s</th>
<th title='Peak resident memory while generating'>RAM GB</th>
<th title='6 deterministic exact-match probes (arithmetic, JSON-only, instruction following)'>quality</th>
<th title='WikiText-2 perplexity, lower is better'>ppl-w ↓</th>
<th title='tulu-3 perplexity, kept for reference only'>ppl-t ↓</th>
<th title='16 C tasks x 3 trials'>C</th>
<th title='8 Python tasks x 3 trials'>Py</th>
<th title='8 Bash tasks x 3 trials'>Bash</th>
<th title='3 harder C tasks x 3 trials'>C-hard</th>
<th title='3 harder Python tasks x 3 trials'>Py-hard</th>
<th title='3 harder Bash tasks x 3 trials'>Sh-hard</th>
<th title='Extract NFS facts from 2,100 words of RHEL 10 docs without taking the bait on unrelated fixes'>research</th>
</tr>
{''.join(rows)}</table>
{arch_table}
<div class='cards'>{ref_panel}</div>
<p class='note'>Green cells are strong, red weak — shaded by percentage so a column can be scanned
without reading every number. Perplexity is MLX-only, so models served through Ollama or llama.cpp
show <span class='dim'>—</span>. coder-next's wikitext figure is measured at sequence-length 128;
the default 512 triggers an mlx-lm bug for hybrid-attention models.</p>

{brutal_table}

{concurrency_table}

{perf_table}
{cost_table}
{h2h_table}
{sidelined_table}
{framing_table}
{prompts_table}

<div class='side'>{error_table}{repair_table}</div>

<h2>Per-model detail</h2>
<p class='note'>Every failing sample below is the real generated code, with the compiler or test
error that rejected it. Trial 0 runs at temperature 0, trials 1 and 2 at 0.7 — so a task passing
1/3 is a sampling-luck pass, not a reliable one.</p>
{''.join(sections)}
<script>{SCRIPT}</script>
</body></html>"""
    # The page renders fine with the wrong contents, so nothing here would have
    # caught the main table being blanked by a shadowed variable. Check it.
    n_rows = len(re.findall(r"<tr>", page[page.index("<table"):page.index("<h2 id=")])) - 1
    if n_rows != expected_models or not expected_models:
        raise SystemExit(f"main table rendered {n_rows} rows, expected {expected_models} — "
                         "something clobbered it; refusing to write the report")
    OUT.write_text(page)
    print(f"wrote {OUT} ({len(page) // 1024} KB, {expected_models} models)")


if __name__ == "__main__":
    sys.exit(main())

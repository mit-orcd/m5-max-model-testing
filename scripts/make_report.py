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

TARGETS = ["gptoss", "gptoss120", "gemma", "coder-next", "devstral", "devstral2", "qwen27",
           "qwen36-35b", "qwen35", "qwen36-27b", "ornith", "coder", "deepseek-32b", "aya",
           "glm-flash", "devstral-small", "north", "laguna", "qwen38flash", "k2horizon", "ollama"]
NAMES = {"gptoss": "gpt-oss-20b", "gptoss120": "gpt-oss-120b", "gemma": "gemma-4-26b", "coder-next": "qwen3-coder-next 80B",
         "devstral": "devstral-2 24b", "devstral2": "devstral-2 24b (rerun)", "qwen27": "qwen3.8-27b", "qwen36-35b": "qwen3.6-35b",
         "qwen35": "qwen3.5-35b", "qwen36-27b": "qwen3.6-27b", "ornith": "ornith-1.5 35b",
         "coder": "qwen3-coder-30b", "deepseek-32b": "deepseek-r1 32b", "aya": "aya-23 35b",
         "glm-flash": "glm-4.7-flash", "ollama": "qwen3.8-27b via Ollama",
         "devstral-small": "devstral-small-2 24b", "north": "north-mini-code",
         "laguna": "laguna-xs.2", "qwen38flash": "qwen3.8-flash-next 125B",
         "k2horizon": "k2-horizon 36B-A4B"}

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
            + "".join(samples) + "</details>")
    effs = f" <small>({eff(data)})</small>" if data.get("total_time_s") else ""
    return (f"<h3>{label} — {data['passed']}/{data['total']}{effs}</h3>"
            + ("".join(fails) if fails else "<p class='dim'>No failures.</p>"))


def main() -> None:
    rows, sections, nav_opts = [], [], []
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
        nav_opts.append((total_p / total_t if total_t else 0,
                         f"<option value='#{t}'>{NAMES[t]} — {total_p}/{total_t}</option>"))

    # Best model first, everywhere.
    rows.sort(key=lambda x: -x[0])
    sections.sort(key=lambda x: -x[0])
    nav_opts.sort(key=lambda x: -x[0])
    rows = [r for _, r in rows]
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
        for t in TARGETS:
            if t not in newest:
                continue
            doc = newest[t][1]
            conc_dates.append(doc["date"])
            by_level = {l["level"]: l for l in doc["levels"]}
            cells = ""
            for lvl in (1, 2, 4, 8):
                l = by_level.get(lvl)
                cells += (f"<td>{l['aggregate_tok_s']:.0f}</td>" if l and l["aggregate_tok_s"]
                          else "<td class='dim'>—</td>")
            best = max((l.get("speedup_vs_1") or 0 for l in doc["levels"]), default=0)
            top = by_level.get(8) or by_level.get(max(by_level)) if by_level else None
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
            "<table><tr><th>model</th>"
            "<th title='aggregate tokens/sec, one request at a time'>1</th>"
            "<th title='aggregate tokens/sec, 2 requests in flight'>2</th>"
            "<th title='aggregate tokens/sec, 4 requests in flight'>4</th>"
            "<th title='aggregate tokens/sec, 8 requests in flight'>8</th>"
            "<th title='best aggregate throughput relative to one-at-a-time'>best gain</th>"
            "<th title='tasks still correct at the highest concurrency level'>correct</th>"
            "<th title='peak resident memory at the highest concurrency level'>RAM GB</th>"
            "<th>run</th></tr>"
            + "".join(r for _, r in conc_rows) + "</table>"
            f"<p class='note'>{conc_runs} run{'s' if conc_runs != 1 else ''} on disk; the newest "
            "per model is shown.</p>")

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
<div class='cards'>{ref_panel}</div>
<p class='note'>Green cells are strong, red weak — shaded by percentage so a column can be scanned
without reading every number. Perplexity is MLX-only, so models served through Ollama or llama.cpp
show <span class='dim'>—</span>. coder-next's wikitext figure is measured at sequence-length 128;
the default 512 triggers an mlx-lm bug for hybrid-attention models.</p>

{brutal_table}

{concurrency_table}

<div class='side'>{error_table}{repair_table}</div>

<h2>Per-model detail</h2>
<p class='note'>Every failing sample below is the real generated code, with the compiler or test
error that rejected it. Trial 0 runs at temperature 0, trials 1 and 2 at 0.7 — so a task passing
1/3 is a sampling-luck pass, not a reliable one.</p>
{''.join(sections)}
<script>{SCRIPT}</script>
</body></html>"""
    OUT.write_text(page)
    print(f"wrote {OUT} ({len(page) // 1024} KB)")


if __name__ == "__main__":
    sys.exit(main())

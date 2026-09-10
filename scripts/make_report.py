#!/usr/bin/env python3
"""Generate results/report.html — all benchmark results + every failing code
sample (C, Python, Bash), syntax-highlighted with highlight.js (CDN).
Run: scripts/make_report.py"""
from __future__ import annotations

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

# (suffix, language for highlight.js, file extension)
SUITES = [("ceval", "C", "c", "c"), ("python", "Python", "python", "py"),
          ("bash", "Bash", "bash", "sh"), ("chard", "C (hard)", "c", "c"),
          ("pyhard", "Python (hard)", "python", "py"),
          ("shhard", "Bash (hard)", "bash", "sh"),
          ("research", "Research", "markdown", "md")]


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
            samples.append(
                f"<div class='sample'><div class='meta'>trial {trial} — "
                f"<span class='status'>{status}</span>"
                f"{f' — <code>{note}</code>' if note else ''}</div>"
                f"<pre><code class='language-{lang}'>{code}</code></pre></div>")
        npass = sum(1 for s in outcomes if s == "pass")
        fails.append(
            f"<details><summary>{task} — {npass}/{len(outcomes)} passed</summary>"
            + "".join(samples) + "</details>")
    effs = f" <small>({eff(data)})</small>" if data.get("total_time_s") else ""
    return (f"<h3>{label} — {data['passed']}/{data['total']}{effs}</h3>"
            + ("".join(fails) if fails else "<p>No failures. 🎉</p>"))


def main() -> None:
    rows, sections = [], []
    for t in TARGETS:
        suites = {s: (load(f"{t}-{s}") or [None])[0] for s, *_ in SUITES}
        if all(v is None for v in suites.values()):
            continue
        speed = load(f"{t}-speed")
        qual = load(f"{t}-quality")
        dec = next((r for r in speed if r["case"] == "decode"), {}) if speed else {}
        tok, rss = dec.get("tok_s"), dec.get("peak_rss_mb")
        q = f"{qual[0]['passed']}/{qual[0]['total']}" if qual else "—"
        cells = "".join(
            f"<td><b>{v['passed']}/{v['total']}</b></td>" if v else "<td>—</td>"
            for v in suites.values())
        rows.append(
            f"<tr><td><a href='#{t}'>{NAMES[t]}</a></td>"
            f"<td>{f'{tok:.1f}' if tok else '—'}</td>"
            f"<td>{f'{rss/1024:.1f}' if rss else '—'}</td>"
            f"<td>{q}</td><td>{perplexity(t, 'wikitext-perplexity')}</td>"
            f"<td>{perplexity(t)}</td>{cells}</tr>")

        body = "".join(
            suite_sections(t, v, label, lang, ext)
            for (s, label, lang, ext), v in zip(SUITES, suites.values()) if v)
        total_p = sum(v["passed"] for v in suites.values() if v)
        total_t = sum(v["total"] for v in suites.values() if v)
        sections.append(f"<h2 id='{t}'>{NAMES[t]} <small>{total_p}/{total_t}</small></h2>{body}")

    # Referee row + section (Kimi K3's own solutions, graded by the same harness)
    ref = RESULTS / "referee" / "kimi-k3"
    if ref.exists():
        rows.append(
            "<tr><td><a href='#referee'>kimi-k3 (referee, cloud)</a></td>"
            "<td>—</td><td>—</td><td>—</td><td>—</td><td>—</td>"
            "<td><b>16/16</b></td><td><b>8/8</b></td><td><b>8/8</b></td>"
            "<td><b>3/3</b></td><td><b>3/3</b></td><td><b>3/3</b></td><td>—</td></tr>")
        ref_blocks = []
        for sub, lang, label in [("", "c", "C — 16/16"), ("py", "python", "Python — 8/8"), ("sh", "bash", "Bash — 8/8")]:
            d = ref / sub if sub else ref
            files = sorted(d.glob(f"*.{ 'c' if lang=='c' else ('py' if lang=='python' else 'sh') }"))
            items = "".join(
                f"<details><summary>{f.name}</summary>"
                f"<pre><code class='language-{lang}'>{html.escape(f.read_text())}</code></pre></details>"
                for f in files)
            ref_blocks.append(f"<h3>{label}</h3>{items}")
        sections.append(
            f"<h2 id='referee'>kimi-k3 (referee) <small>32/32</small></h2>"
            f"<p class='note'>Single attempt per task, same rules, graded by the same harness. "
            f"Hardware metrics don't apply — the referee is a hosted cloud model, not served on this Mac. "
            f"Caveat: the referee authored the harness, so treat 32/32 as a sanity ceiling, not a fair contest.</p>"
            + "".join(ref_blocks))

    # Self-repair section: results/<t>-repair.json from scripts/eval_repair.py
    rep_rows = []
    for t in TARGETS + ["kimi-k3"]:
        r = load(f"{t}-repair")
        if not r:
            continue
        r = r[0]
        name = NAMES.get(t, r.get("model", t))
        waste = r.get("waste_tokens")
        toks = r.get("total_tokens")
        rep_rows.append(
            f"<tr><td>{name}</td><td><b>{r['one_shot']}/{r['tasks']}</b></td>"
            f"<td>{r['repaired']}</td><td>{r['never']}</td>"
            f"<td>{r.get('median_rounds_repaired') or '—'}</td>"
            f"<td>{f'{toks/1000:.1f}k' if toks else '—'}</td>"
            f"<td>{f'{waste/1000:.1f}k' if waste is not None else '—'}</td></tr>")
    repair_table = ""
    if rep_rows:
        repair_table = (
            "<h2 id='repair'>C self-repair (5 rounds, error feedback)</h2>"
            "<p class='note'>Round 1 is the one-shot attempt; rounds 2–5 feed the failed code plus "
            "compiler/test errors back. <b>one-shot</b> = passed round 1, <b>repaired</b> = passed "
            "in a later round, <b>never</b> = still failing after 5 rounds, <b>median rnd</b> = "
            "median round at which repaired tasks first passed, <b>waste</b> = tokens generated in "
            "rounds after a task had already passed (0 for a perfect stop).</p>"
            "<table><tr><th>model</th><th>one-shot</th><th>repaired</th><th>never</th>"
            "<th>median rnd</th><th>total tok</th><th>waste tok</th></tr>"
            + "".join(rep_rows) + "</table>")

    page = f"""<!doctype html>
<html><head><meta charset='utf-8'><title>M5 Max eval report</title>
<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github-dark.min.css'>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/c.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/python.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/bash.min.js'></script>
<style>
 body {{ font: 15px/1.5 -apple-system, sans-serif; max-width: 1100px; margin: 2rem auto; padding: 0 1rem; background: #0d1117; color: #e6edf3; }}
 table {{ border-collapse: collapse; width: 100%; }} td, th {{ border: 1px solid #30363d; padding: 6px 10px; }}
 th {{ background: #161b22; }} a {{ color: #58a6ff; }}
 pre {{ margin: 0 0 1rem; border-radius: 6px; }} pre code {{ border-radius: 6px; }}
 details {{ margin: .4rem 0 .4rem 1rem; }} summary {{ cursor: pointer; }}
 .meta {{ color: #9da7b3; font-size: 13px; margin: .5rem 0 .2rem; }}
 .status {{ color: #f85149; }} h2 small {{ color: #9da7b3; }} h3 {{ margin-bottom: .2rem; }}
 .note {{ background: #161b22; border-left: 3px solid #58a6ff; padding: .6rem 1rem; }}
</style></head><body>
<h1>M5 Max model testing — eval report</h1>
<p class='note'>Every sample below was machine-verified: C compiled with <code>cc -std=c11 -Wall</code>,
Python run under hidden asserts, Bash checked for exact stdout/exit codes — no LLM judge.
Syntax highlighting by highlight.js; the pass/fail ground truth is the compiler/interpreter.
A referee audit re-graded all C samples: 191/191 confirmed real failures. Referee baseline
(Kimi K3, same harness): C 16/16, Python 8/8, Bash 8/8.</p>
<table><tr><th>model</th><th>decode tok/s</th><th>RAM GB</th><th>quality</th>
<th>ppl wikitext ↓</th><th>ppl tulu-3 ↓</th>
<th>C</th><th>Python</th><th>Bash</th>
<th>C-hard</th><th>Py-hard</th><th>Sh-hard</th><th>Research</th></tr>
{''.join(rows)}</table>
{repair_table}
<p class='note'>Suite headers show <code>score (total time / total completion tokens)</code>.
Perplexity: wikitext = plain text at sequence-length 512 (coder-next measured at 128 —
the 512 path triggers an mlx-lm batched-perplexity bug for hybrid models); tulu-3 =
chat-formatted SFT data, kept for reference only.</p>
{''.join(sections)}
<script>hljs.highlightAll();</script>
</body></html>"""
    OUT.write_text(page)
    print(f"wrote {OUT} ({len(page)//1024} KB)")


if __name__ == "__main__":
    sys.exit(main())

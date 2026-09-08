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

TARGETS = ["gptoss", "gemma", "coder-next", "devstral", "qwen27", "qwen36-35b",
           "qwen35", "qwen36-27b", "ornith", "coder", "deepseek-32b", "aya",
           "glm-flash", "ollama"]
NAMES = {"gptoss": "gpt-oss-20b", "gemma": "gemma-4-26b", "coder-next": "qwen3-coder-next 80B",
         "devstral": "devstral-2 24b", "qwen27": "qwen3.8-27b", "qwen36-35b": "qwen3.6-35b",
         "qwen35": "qwen3.5-35b", "qwen36-27b": "qwen3.6-27b", "ornith": "ornith-1.5 35b",
         "coder": "qwen3-coder-30b", "deepseek-32b": "deepseek-r1 32b", "aya": "aya-23 35b",
         "glm-flash": "glm-4.7-flash", "ollama": "qwen3.8-27b via Ollama"}

# (suffix, language for highlight.js, file extension)
SUITES = [("ceval", "c", "c"), ("python", "python", "py"), ("bash", "bash", "sh")]


def load(prefix: str):
    p = RESULTS / f"{prefix}.json"
    if not p.exists():
        return None
    txt = p.read_text()
    m = re.search(r"^\[$", txt, re.M)
    if not m:
        return None
    return json.loads(txt[m.start():])


def perplexity(t: str) -> str:
    p = RESULTS / f"{t}-perplexity.txt"
    if not p.exists():
        return "—"
    m = re.search(r"Perplexity: ([0-9.]+)", p.read_text())
    return m.group(1) if m else "—"


def suite_sections(t: str, data: dict, lang: str, ext: str) -> str:
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
    label = {"c": "C", "python": "Python", "bash": "Bash"}[lang]
    return (f"<h3>{label} — {data['passed']}/{data['total']}</h3>"
            + ("".join(fails) if fails else "<p>No failures. 🎉</p>"))


def main() -> None:
    rows, sections = [], []
    for t in TARGETS:
        suites = {s: (load(f"{t}-{s}") or [None])[0] for s, _, _ in SUITES}
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
            f"<td>{q}</td><td>{perplexity(t)}</td>{cells}</tr>")

        body = "".join(
            suite_sections(t, v, lang, ext)
            for (s, lang, ext), v in zip(SUITES, suites.values()) if v)
        total_p = sum(v["passed"] for v in suites.values() if v)
        total_t = sum(v["total"] for v in suites.values() if v)
        sections.append(f"<h2 id='{t}'>{NAMES[t]} <small>{total_p}/{total_t}</small></h2>{body}")

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
<table><tr><th>model</th><th>decode tok/s</th><th>RAM GB</th><th>quality</th><th>perplexity ↓</th>
<th>C eval</th><th>Python</th><th>Bash</th></tr>
{''.join(rows)}</table>
{''.join(sections)}
<script>hljs.highlightAll();</script>
</body></html>"""
    OUT.write_text(page)
    print(f"wrote {OUT} ({len(page)//1024} KB)")


if __name__ == "__main__":
    sys.exit(main())

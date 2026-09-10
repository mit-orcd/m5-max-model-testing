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
            + ("".join(fails) if fails else "<p>No failures.</p>"))


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

        sample_f = RESULTS / "speed-texts" / f"{t}.txt"
        sample_html = ""
        if sample_f.exists():
            sample_html = (
                "<details><summary>decode sample — 1500-word MIT essay "
                "(speed-bench generation)</summary>"
                f"<pre style='white-space:pre-wrap'>{html.escape(sample_f.read_text())}</pre>"
                "</details>")
        body = sample_html + "".join(
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
        secs = r.get("total_time_s")
        rep_rows.append(
            (r["one_shot"], r["never"],
            f"<tr><td>{name}</td><td><b>{r['one_shot']}/{r['tasks']}</b></td>"
            f"<td>{r['repaired']}</td><td>{r['never']}</td>"
            f"<td>{r.get('median_rounds_repaired') or '—'}</td>"
            f"<td>{f'{secs/60:.1f} min' if secs else '—'}</td>"
            f"<td>{f'{toks/1000:.1f}k' if toks else '—'}</td>"
            f"<td>{f'{waste/1000:.1f}k' if waste is not None else '—'}</td></tr>"))
    rep_rows.sort(key=lambda x: (-x[0], x[1]))
    rep_rows = [row for _, _, row in rep_rows]
    repair_table = ""
    if rep_rows:
        repair_table = (
            "<div><h2 id='repair'>C self-repair (5 rounds, error feedback)</h2>"
            "<p class='note'>Round 1 one-shot; rounds 2–5 get failed code + errors back. "
            "<b>waste</b> = tokens on tasks needing >1 round; 0 = perfect.</p>"
            "<table><tr><th>model</th><th>one-shot</th><th>repaired</th><th>never</th>"
            "<th>med rnd</th><th>time</th><th>total tok</th><th>waste</th></tr>"
            + "".join(rep_rows) + "</table></div>")

    # C error-category pivot: what kind of failure, per model (ceval + chard notes)
    CATS = [("linker", "linker error (no main / undefined symbol)"),
            ("undeclared", "undeclared identifier / missing include"),
            ("compile", "other compile error (type/syntax)"),
            ("wrong", "wrong answer (test FAIL)"),
            ("other", "other (extract/timeout/http)")]

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
            cells = "".join(f"<td>{counts[k] or ''}</td>" for k, _ in CATS)
            cat_rows.append(
                f"<tr><td>{NAMES[t]}</td><td><b>{total_fails}</b></td>{cells}</tr>")
    cat_rows.sort(key=lambda r: int(re.search(r"<b>(\d+)</b>", r).group(1)))
    error_table = ""
    if cat_rows:
        error_table = (
            "<div><h2 id='cerrors'>C failure breakdown by error kind</h2>"
            "<p class='note'>Failed trials per model (C easy+hard, 72 samples) + error kind per failing "
            "task. linker = no <code>main</code>; undeclared = missing <code>#include</code>; "
            "wrong = compiled but failed hidden tests.</p>"
            "<table><tr><th>model</th><th>failed</th>"
            + "".join(f"<th>{k}</th>" for k, _ in CATS)
            + "</tr>" + "".join(cat_rows) + "</table></div>")

    page = f"""<!doctype html>
<html><head><meta charset='utf-8'><title>M5 Max eval report</title>
<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github-dark.min.css'>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/c.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/python.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/bash.min.js'></script>
<style>
 body {{ font: 15px/1.5 -apple-system, sans-serif; max-width: 1400px; margin: 1rem auto; padding: 0 1rem; background: #0d1117; color: #e6edf3; }}
 table {{ border-collapse: collapse; width: 100%; font-size: 12px; line-height: 1.2; }}
 td, th {{ border: 1px solid #30363d; padding: 1px 5px; white-space: nowrap; }}
 .side {{ display: flex; gap: 1.5rem; align-items: flex-start; }}
 .side > div {{ flex: 1; min-width: 0; }}
 th {{ background: #161b22; }} a {{ color: #58a6ff; }}
 h1 {{ font-size: 20px; margin: .4rem 0; }} h2 {{ font-size: 16px; margin: .8rem 0 .3rem; }}
 .note {{ font-size: 12px; line-height: 1.35; padding: .35rem .7rem; margin: .3rem 0; }}
 pre {{ margin: 0 0 1rem; border-radius: 6px; }} pre code {{ border-radius: 6px; }}
 details {{ margin: .4rem 0 .4rem 1rem; }} summary {{ cursor: pointer; }}
 .meta {{ color: #9da7b3; font-size: 13px; margin: .5rem 0 .2rem; }}
 .status {{ color: #f85149; }} h2 small {{ color: #9da7b3; }} h3 {{ margin-bottom: .2rem; }}
 .note {{ background: #161b22; border-left: 3px solid #58a6ff; }}
</style></head><body>
<h1>M5 Max model testing — eval report</h1>
<p class='note'>All samples machine-verified (C compiled <code>cc -std=c11 -Wall</code>, Python hidden
asserts, Bash exact stdout) — no LLM judge. Referee audit: 191/191 C failures confirmed real.</p>
<table><tr><th>model</th><th>tok/s</th><th>RAM GB</th><th>qual</th>
<th>ppl-w ↓</th><th>ppl-t ↓</th>
<th>C</th><th>Py</th><th>Bash</th>
<th>C-h</th><th>Py-h</th><th>Sh-h</th><th>Res</th></tr>
{''.join(rows)}</table>
<div class='side'>{error_table}{repair_table}</div>
<p class='note'>Suite headers show <code>score (time / tokens)</code>. ppl: wikitext @ seq-512
(coder-next @128 — 512 triggers an mlx-lm hybrid-model bug); tulu-3 kept for reference.</p>
{''.join(sections)}
<script>hljs.highlightAll();</script>
</body></html>"""
    OUT.write_text(page)
    print(f"wrote {OUT} ({len(page)//1024} KB)")


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Export every benchmark's raw results as CSVs into results/csv/.

The HTML report and the PNG charts both read the JSON in results/; this script
flattens the same JSON into spreadsheets so anyone can build their own charts
in Excel, Numbers, R, pandas, ... without parsing JSON. Re-run after any bench.
"""
import csv
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from make_report import (ARCH, BRUTAL_SUITES, NAMES, RESULTS, SIDELINED, STACK,
                         SUITES, TARGETS, load)

OUT = RESULTS / "csv"
OUT.mkdir(exist_ok=True)

ALL_SUITES = [s for s, *_ in SUITES] + [s for s, *_ in BRUTAL_SUITES]


def write(name: str, header: list[str], rows: list[list]) -> None:
    p = OUT / name
    with p.open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    print(f"wrote csv/{name} ({len(rows)} rows)")


def ppl(t: str, suffix: str = "perplexity") -> float | None:
    p = RESULTS / f"{t}-{suffix}.txt"
    if not p.exists():
        return None
    m = re.search(r"Perplexity: ([0-9.]+)", p.read_text())
    return float(m.group(1)) if m else None


def speed_decode(t: str) -> dict:
    speed = load(f"{t}-speed") or load(f"{t}-decode")
    if not speed:
        return {}
    return next((r for r in speed if r.get("case") == "decode"), {})


# ---- models.csv: one row per model, every headline number -------------------
rows = []
for t in TARGETS:
    suites = {s: (load(f"{t}-{s}") or [None])[0] for s in ALL_SUITES}
    if all(v is None for v in suites.values()):
        continue
    dec = speed_decode(t)
    runs = [r["tok_s"] for r in dec.get("runs", []) if r.get("tok_s")]
    tok_std = (float(__import__("statistics").pstdev(runs)) if len(runs) > 1 else None)
    qual = (load(f"{t}-quality") or [None])[0]
    row = [
        t, NAMES.get(t, t), (ARCH.get(t) or ("",))[0], STACK.get(t, "MLX"),
        "yes" if t in SIDELINED else "no",
        sum(v["passed"] for v in suites.values() if v),
        sum(v["total"] for v in suites.values() if v),
    ]
    for s in ALL_SUITES:
        v = suites[s]
        row += [v["passed"], v["total"]] if v else ["", ""]
    row += [
        dec.get("tok_s"), tok_std,
        (dec.get("peak_rss_mb") or 0) / 1024 or None,
        qual["passed"] if qual else "", qual["total"] if qual else "",
        ppl(t, "wikitext-perplexity"), ppl(t),
    ]
    rows.append(row)
rows.sort(key=lambda r: -(r[5] / r[6] if r[6] else 0))
write("models.csv",
      ["target", "name", "architecture", "stack", "sidelined",
       "coding_passed", "coding_total"]
      + [f"{s}_{k}" for s in ALL_SUITES for k in ("passed", "total")]
      + ["decode_tok_s", "decode_tok_s_std", "peak_ram_gb",
         "quality_passed", "quality_total", "ppl_wikitext", "ppl_tulu"],
      rows)

# ---- trials.csv: every single generation, one row each ----------------------
rows = []
for t in TARGETS:
    for s in ALL_SUITES:
        v = (load(f"{t}-{s}") or [None])[0]
        if not v:
            continue
        for task, statuses in v["results"].items():
            times = v.get("time_s", {}).get(task, [])
            toks = v.get("tokens", {}).get(task, [])
            note = v.get("notes", {}).get(task, "")
            for trial, status in enumerate(statuses):
                rows.append([
                    t, s, task, trial, 0.0 if trial == 0 else 0.7, status,
                    times[trial] if trial < len(times) else "",
                    toks[trial] if trial < len(toks) else "", note,
                ])
rows.sort(key=lambda r: (r[0], r[1], r[2], r[3]))
write("trials.csv",
      ["target", "suite", "task", "trial", "temperature", "status",
       "time_s", "completion_tokens", "failure_note"],
      rows)

# ---- speed.csv: per-run decode/prefill measurements -------------------------
rows = []
for t in TARGETS:
    speed = load(f"{t}-speed") or load(f"{t}-decode")
    if not speed:
        continue
    for case in speed:
        for run_i, r in enumerate(case.get("runs", [])):
            rows.append([t, case.get("case"), run_i, r.get("tok_s"),
                         r.get("ttft_ms"), r.get("elapsed_s"),
                         r.get("completion_tokens"), r.get("prompt_tokens"),
                         (r.get("peak_rss_mb") or 0) / 1024 or None])
write("speed.csv",
      ["target", "case", "run", "tok_s", "ttft_ms", "elapsed_s",
       "completion_tokens", "prompt_tokens", "peak_ram_gb"],
      rows)

# ---- concurrency.csv ---------------------------------------------------------
rows = []
for f in sorted(RESULTS.glob("concurrency/*.json")):
    d = json.loads(f.read_text())
    t = d.get("target") or f.name.split("-20")[0]
    for lv in d.get("levels", []):
        rows.append([t, lv["level"], lv.get("requests"), lv.get("wall_s"),
                     lv.get("total_tokens"), lv.get("aggregate_tok_s"),
                     lv.get("per_stream_tok_s"), lv.get("ttft_ms_median"),
                     lv.get("ttft_ms_max"), lv.get("passed"), lv.get("total"),
                     (lv.get("peak_rss_mb") or 0) / 1024 or None])
rows.sort(key=lambda r: (r[0], r[1]))
write("concurrency.csv",
      ["target", "level", "requests", "wall_s", "total_tokens",
       "aggregate_tok_s", "per_stream_tok_s", "ttft_ms_median", "ttft_ms_max",
       "passed", "total", "peak_ram_gb"],
      rows)

# ---- framing.csv -------------------------------------------------------------
rows = []
for f in sorted(RESULTS.glob("framing/*.json")):
    d = json.loads(f.read_text())
    t = d.get("target") or f.name.split("-20")[0]
    for cond, c in d.get("conditions", {}).items():
        rows.append([t, cond, c.get("n"), c.get("fast"), c.get("correct"),
                     c.get("fast_rate"), c.get("ci95"), c.get("best_ms"),
                     c.get("p_vs_bare"), c.get("greedy")])
rows.sort(key=lambda r: (r[0], r[1]))
write("framing.csv",
      ["target", "condition", "trials", "fast", "correct", "fast_rate",
       "ci95", "best_ms", "p_vs_bare", "greedy_baseline"],
      rows)

# ---- repair.csv: one row per task per language ------------------------------
rows = []
for t in TARGETS:
    for lang, suffix in (("c", "repair"), ("python", "repair-python"),
                         ("bash", "repair-bash")):
        r = (load(f"{t}-{suffix}") or [None])[0]
        if not r:
            continue
        for task, pt in r.get("per_task", {}).items():
            rows.append([t, lang, task, pt.get("first_status"),
                         pt.get("last_status"), pt.get("passed_round"),
                         pt.get("rounds_used"), pt.get("tokens"), pt.get("time_s")])
rows.sort(key=lambda r: (r[0], r[1], r[2]))
write("repair.csv",
      ["target", "lang", "task", "first_status", "last_status",
       "passed_round", "rounds_used", "tokens", "time_s"],
      rows)

# ---- perf.csv: how fast the generated code runs ------------------------------
rows = []
for f in sorted(RESULTS.glob("perf/*.json")):
    d = json.loads(f.read_text())
    t = d.get("target") or f.name.split("-20")[0]
    for variant, tasks in d.get("variants", {}).items():
        for task, m in tasks.items():
            rows.append([t, variant, task, m.get("trials"), m.get("correct"),
                         m.get("best_ms"), m.get("median_ms")])
rows.sort(key=lambda r: (r[0], r[1], r[2]))
write("perf.csv",
      ["target", "variant", "task", "trials", "correct", "best_ms", "median_ms"],
      rows)

print("done ->", OUT)

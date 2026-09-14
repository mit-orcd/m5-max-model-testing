#!/usr/bin/env python3
"""Render the bench results as PNG charts into results/charts/."""
import json
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
from make_report import TARGETS, SIDELINED, NAMES, ARCH, SUITES, BRUTAL_SUITES, load, RESULTS

OUT = RESULTS / "charts"
OUT.mkdir(exist_ok=True)

IDENT = "cs_degree dropout lawyer black african swiss white".split()
OLD7 = "bare timed user_expert model_persona stakes think user_beginner".split()

plt.rcParams.update({
    "figure.facecolor": "#0d1117", "axes.facecolor": "#161b22",
    "axes.edgecolor": "#30363d", "axes.labelcolor": "#e6edf3",
    "text.color": "#e6edf3", "xtick.color": "#9da7b3", "ytick.color": "#9da7b3",
    "grid.color": "#30363d", "font.size": 10,
})
MOE_C, DENSE_C = "#3fb950", "#d2991a"


def models():
    return [t for t in TARGETS if t not in SIDELINED]


def label(t):
    return NAMES.get(t, t)


def color(t):
    return MOE_C if (ARCH.get(t) or (None,))[0] == "MoE" else DENSE_C


def save(fig, name):
    fig.savefig(OUT / name, dpi=130, bbox_inches="tight", facecolor=fig.get_facecolor())
    plt.close(fig)
    print("wrote", name)


def suite_totals(t, suites):
    p = tt = 0
    for s, *_ in suites:
        v = (load(f"{t}-{s}") or [None])[0]
        if v:
            p += v["passed"]; tt += v["total"]
    return p, tt


def decode_tok(t):
    speed = load(f"{t}-speed") or load(f"{t}-decode")
    if not speed:
        return None
    dec = next((r for r in speed if r.get("case") == "decode"), {})
    return dec.get("tok_s")


def decode_std(t):
    """Std across the repeated decode runs, for error bars."""
    speed = load(f"{t}-speed") or load(f"{t}-decode")
    if not speed:
        return 0.0
    dec = next((r for r in speed if r.get("case") == "decode"), {})
    runs = [r["tok_s"] for r in dec.get("runs", []) if r.get("tok_s")]
    return float(np.std(runs)) if len(runs) > 1 else 0.0


def latest(globpat):
    fs = sorted(RESULTS.glob(globpat))
    return json.loads(fs[-1].read_text()) if fs else None


ms = models()

# ---- 0. the headline: seconds of wall clock per solution that passes -------
# The one number that combines being right with being fast.
COST_SUITES = ["ceval", "chard", "python", "pyhard", "bash", "shhard"]
fig, ax = plt.subplots(figsize=(10, 7))
rows = []
for t in ms:
    ok = 0
    secs = 0.0
    complete = True
    for s in COST_SUITES:
        doc = (load(f"{t}-{s}") or [None])[0]
        if not doc:
            complete = False; break
        times = doc.get("time_s", {})
        for name, outcomes in doc["results"].items():
            per = times.get(name, [])
            if len(per) != len(outcomes):
                complete = False; break
            spent = 0.0
            for outcome, sec in zip(outcomes, per):
                spent += sec
                if outcome == "pass":
                    ok += 1
                    break
            secs += spent
        if not complete:
            break
    if complete and ok:
        rows.append((t, secs / ok))
rows.sort(key=lambda r: -r[1])
y = np.arange(len(rows))
ax.barh(y, [v for _, v in rows], color=[color(t) for t, _ in rows])
ax.set_yticks(y, [label(t) for t, _ in rows], fontsize=8.5)
ax.set_xlabel("seconds of wall clock per working solution  ←  lower is better")
ax.set_title("What it costs to get code that compiles and passes")
ax.grid(axis="x", alpha=.3)
for i, (_, v) in enumerate(rows):
    ax.text(v + max(v for _, v in rows) * .01, i, f"{v:.0f}s", va="center", fontsize=8)
ax.margins(x=.08)
save(fig, "cost-per-solution.png")

# ---- 1. overall score vs decode speed -------------------------------------
from adjustText import adjust_text
fig, ax = plt.subplots(figsize=(11, 7))
texts = []
for t in ms:
    p, tt = suite_totals(t, SUITES)
    tok = decode_tok(t)
    if not tt or not tok:
        continue
    x, y = tok, 100 * p / tt
    ax.errorbar(x, y, xerr=decode_std(t), fmt="none", ecolor="#9da7b3",
                elinewidth=1, capsize=2.5, alpha=.5, zorder=2)
    ax.scatter(x, y, s=90, c=color(t), edgecolors="#0d1117", zorder=3)
    texts.append(ax.text(x, y, label(t), fontsize=8.5))
adjust_text(texts, ax=ax, arrowprops=dict(arrowstyle="-", color="#9da7b3", lw=.6),
            expand=(1.3, 1.6))
ax.set_xlabel("decode speed (tok/s) — whiskers are ±1 std across the repeated runs")
ax.set_ylabel("coding score (%)")
ax.set_title("Score vs speed — upper right wins (green = MoE, amber = dense)")
ax.margins(x=.1, y=.1)
ax.grid(alpha=.3)
save(fig, "score-vs-speed.png")

# ---- 2. per-suite heatmap --------------------------------------------------
suite_names = [s for s, *_ in SUITES]
data = []
keep = []
for t in ms:
    row = []
    for s in suite_names:
        v = (load(f"{t}-{s}") or [None])[0]
        row.append(v["passed"] / v["total"] if v and v["total"] else np.nan)
    if not all(np.isnan(row)):
        data.append(row); keep.append(t)
order = np.argsort([-np.nanmean(r) for r in data])
data = [data[i] for i in order]; keep = [keep[i] for i in order]
fig, ax = plt.subplots(figsize=(9, max(5, .32 * len(keep))))
im = ax.imshow(np.array(data) * 100, cmap="RdYlGn", vmin=0, vmax=100, aspect="auto")
ax.set_xticks(range(len(suite_names)), [l for _, l, *_ in SUITES], rotation=30, ha="right")
ax.set_yticks(range(len(keep)), [label(t) for t in keep])
for i, row in enumerate(data):
    for j, v in enumerate(row):
        if not np.isnan(v):
            ax.text(j, i, f"{v*100:.0f}", ha="center", va="center", fontsize=7,
                    color="#0d1117")
ax.set_title("Pass rate per suite (%)")
fig.colorbar(im, ax=ax, shrink=.6, label="%")
save(fig, "suite-heatmap.png")

# ---- 3. brutal set ---------------------------------------------------------
fig, ax = plt.subplots(figsize=(9, 6))
rows = []
for t in ms:
    p, tt = suite_totals(t, BRUTAL_SUITES)
    if tt:
        rows.append((t, p, tt))
rows.sort(key=lambda r: r[1] / r[2])
y = np.arange(len(rows))
ax.barh(y, [100 * p / tt for _, p, tt in rows], color=[color(t) for t, _, _ in rows])
ax.set_yticks(y, [label(t) for t, _, _ in rows], fontsize=8)
ax.set_xlabel("brutal set pass rate (%)")
ax.set_title("Brutal set — 6 adversarial tasks, 3 trials each")
ax.grid(axis="x", alpha=.3)
for i, (_, p, tt) in enumerate(rows):
    ax.text(100 * p / tt + .5, i, f"{p}/{tt}", va="center", fontsize=8)
save(fig, "brutal.png")

# ---- 4. concurrency scaling ------------------------------------------------
# One distinct color per model so the legend actually identifies each line;
# architecture is carried by line style instead (solid = MoE, dashed = dense).
fig, ax = plt.subplots(figsize=(9, 6))
plotted = []
for t in ms:
    d = latest(f"concurrency/{t}-20*.json")
    if d:
        plotted.append((t, d))
palette = (list(matplotlib.colormaps["tab20"].colors)
           + list(matplotlib.colormaps["tab20b"].colors))
for i, (t, d) in enumerate(plotted):
    lv = d["levels"]
    moe = (ARCH.get(t) or (None,))[0] == "MoE"
    ax.plot([l["level"] for l in lv], [l["aggregate_tok_s"] for l in lv],
            marker="o", ms=3, lw=1.3, label=label(t), color=palette[i % len(palette)],
            linestyle="-" if moe else "--", alpha=.9)
ax.set_xlabel("concurrent streams")
ax.set_ylabel("aggregate tok/s")
ax.set_title("Throughput as streams multiply — solid = MoE, dashed = dense, one pass per level")
ax.grid(alpha=.3)
handles, labels_ = ax.get_legend_handles_labels()
handles += [matplotlib.lines.Line2D([], [], color="#9da7b3", ls="-"),
            matplotlib.lines.Line2D([], [], color="#9da7b3", ls="--")]
labels_ += ["MoE", "dense"]
ax.legend(handles, labels_, fontsize=7, ncol=2, framealpha=.9, facecolor="#161b22",
          edgecolor="#30363d", loc="upper left", bbox_to_anchor=(1.01, 1),
          borderaxespad=0)
save(fig, "concurrency.png")

# ---- 5. framing: fast-solution rate heatmap --------------------------------
# models x wordings, one number per cell, sorted so the models with the most
# room to move are at the top and the strongest wording is leftmost.
fig, ax = plt.subplots(figsize=(11, 7))
conds = ["bare"] + OLD7[1:] + IDENT
cond_titles = {
    "bare": "bare", "timed": "will be timed", "user_expert": "I'm an expert",
    "model_persona": "you're an expert", "stakes": "production code",
    "think": "think carefully", "user_beginner": "I'm a beginner",
    "cs_degree": "I studied CS", "dropout": "dropout", "lawyer": "lawyer",
    "black": "I'm Black", "african": "I'm African", "swiss": "I'm Swiss",
    "white": "I'm White",
}
fr = {}
for t in ms:
    d = latest(f"framing/{t}-20*.json")
    if d and "bare" in d["conditions"]:
        fr[t] = d["conditions"]
grid, keep = [], []
for t in fr:
    row = [fr[t][c]["fast_rate"] if c in fr[t] else np.nan for c in conds]
    if not all(np.isnan(row)):
        grid.append(row); keep.append(t)
# sort models by bare rate (headroom at top), conditions by pooled rate
order = np.argsort([np.nanmean([r[0]]) for r in grid])
grid = [grid[i] for i in order]; keep = [keep[i] for i in order]
pooled = [np.nanmean([r[j] for r in grid]) for j in range(len(conds))]
corder = np.argsort(pooled)
grid = [[r[j] for j in corder] for r in grid]
conds = [conds[j] for j in corder]
im = ax.imshow(np.array(grid) * 100, cmap="RdYlGn", vmin=0, vmax=100, aspect="auto")
ax.set_xticks(range(len(conds)), [cond_titles.get(c, c) for c in conds],
              rotation=35, ha="right", fontsize=8)
ax.set_yticks(range(len(keep)), [label(t) for t in keep], fontsize=8)
for i, row in enumerate(grid):
    for j, v in enumerate(row):
        if not np.isnan(v):
            ax.text(j, i, f"{v*100:.0f}", ha="center", va="center", fontsize=7,
                    color="#0d1117")
ax.set_title("How often each model writes the fast version, per wording (% of 20 trials)")
fig.colorbar(im, ax=ax, shrink=.7, label="% fast")
save(fig, "framing-delta.png")

# ---- 6. self-repair ---------------------------------------------------------
# Stacked by outcome so there are no gaps: every task lands in exactly one of
# right-first-time / fixed-with-feedback / still-broken.
fig, ax = plt.subplots(figsize=(10, 7))
rows = []
for t in ms:
    one = rep = never = tasks = 0
    for suf in ("", "-python", "-bash"):
        r = (load(f"{t}-repair{suf}") or [None])[0]
        if not r:
            continue
        one += r["one_shot"]; rep += r["repaired"]; never += r["never"]
        tasks += r["tasks"]
    if tasks:
        rows.append((t, one / tasks, rep / tasks, never / tasks, one, rep, never, tasks))
rows.sort(key=lambda r: (r[1] + r[2], r[1]))
y = np.arange(len(rows))
one = np.array([r[1] for r in rows]) * 100
repd = np.array([r[2] for r in rows]) * 100
nev = np.array([r[3] for r in rows]) * 100
ax.barh(y, one, color="#3fb950", label="right first time")
ax.barh(y, repd, left=one, color="#58a6ff", label="fixed after seeing the error")
ax.barh(y, nev, left=one + repd, color="#f85149", label="still broken after 5 rounds")
ax.set_yticks(y, [label(r[0]) for r in rows], fontsize=8.5)
ax.set_xlabel("share of the 41 repair tasks (%)")
ax.set_xlim(0, 100)
ax.set_title("Self-repair — one attempt, then up to 5 rounds of compiler feedback")
ax.legend(loc="upper center", bbox_to_anchor=(.5, -.08), ncol=3, fontsize=8.5,
          facecolor="#161b22", edgecolor="#30363d")
ax.grid(axis="x", alpha=.3)
for i, r in enumerate(rows):
    if r[2]:
        ax.text(r[1] * 100 + r[2] * 50, i, f"+{r[5]}", ha="center", va="center",
                fontsize=7, color="#0d1117")
save(fig, "repair.png")

# ---- 7. generated-code runtime (perf) --------------------------------------
# Shown as a slowdown multiple against the fastest answer anyone gave, because
# raw milliseconds across three different tasks are not comparable.
fig, ax = plt.subplots(figsize=(10, 7))
per_task = {}
for t in ms:
    d = latest(f"perf/{t}-20*.json")
    if not d:
        continue
    for vname, variant in d["variants"].items():
        for task, entry in variant.items():
            if entry.get("correct") and entry.get("best_ms"):
                per_task.setdefault((task, vname), {})[t] = entry["best_ms"]
best = {k: min(v.values()) for k, v in per_task.items()}
rows = []
for t in ms:
    ratios = [ms_ / best[k] for k, v in per_task.items()
              for tt2, ms_ in v.items() if tt2 == t]
    if ratios:
        rows.append((t, float(np.exp(np.mean(np.log(ratios))))))
rows.sort(key=lambda r: -r[1])
y = np.arange(len(rows))
ax.barh(y, [v for _, v in rows], color=[color(t) for t, _ in rows])
ax.set_yticks(y, [label(t) for t, _ in rows], fontsize=8.5)
ax.set_xscale("log")
ax.set_xlabel("how many times slower than the fastest answer  ←  lower is better")
ax.set_title("How fast the code they write actually runs")
ax.grid(axis="x", alpha=.3)
for i, (_, v) in enumerate(rows):
    ax.text(v * 1.08, i, f"{v:.0f}x" if v >= 10 else f"{v:.1f}x",
            va="center", fontsize=8)
ax.margins(x=.12)
save(fig, "generated-code-speed.png")

print("done ->", OUT)

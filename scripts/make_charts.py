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
import argparse
import make_report as mr
from make_report import (TARGETS, SIDELINED, NAMES, ARCH, SUITES, BRUTAL_SUITES,
                         load, configure)

_ap = argparse.ArgumentParser(description=__doc__)
_ap.add_argument("--results", type=Path, default=None)
configure(_ap.parse_args().results)
OUT = mr.RESULTS / "charts"
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
    fig.savefig(OUT / name, dpi=130, bbox_inches="tight", pad_inches=0.25,
                facecolor=fig.get_facecolor())
    plt.close(fig)
    print("wrote", name)


def skip_empty(items, fig, name):
    if items:
        return False
    plt.close(fig)
    print(f"skip {name} (no data yet)")
    return True


def spread_y(ys, lo, hi, gap):
    """Push y-positions apart so adjacent labels keep `gap`, then fit [lo, hi]."""
    if not ys:
        return []
    order = list(np.argsort(ys))
    placed = np.asarray(ys, dtype=float)
    for i in range(1, len(order)):
        a, b = order[i - 1], order[i]
        if placed[b] < placed[a] + gap:
            placed[b] = placed[a] + gap
    if placed[order[-1]] > hi:
        placed -= placed[order[-1]] - hi
    if placed[order[0]] < lo:
        placed += lo - placed[order[0]]
        if placed[order[-1]] > hi:
            span = placed[order[-1]] - placed[order[0]]
            room = hi - lo
            if span > 0 and room > 0:
                placed = lo + (placed - placed[order[0]]) * (room / span)
    return placed


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
    fs = sorted(mr.RESULTS.glob(globpat))
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
if skip_empty(rows, fig, "cost-per-solution.png"):
    pass
else:
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
# Dot plot: one row per model (sorted by score), tok/s on a log x-axis.
# Labels are y-tick labels, so they can never collide; whiskers are ±1 std.
pts = []
for t in ms:
    p, tt = suite_totals(t, SUITES)
    tok = decode_tok(t)
    if not tt or not tok:
        continue
    pts.append((t, tok, 100 * p / tt))
fig, ax = plt.subplots(figsize=(10, 8))
if skip_empty(pts, fig, "score-vs-speed.png"):
    pass
else:
    pts.sort(key=lambda r: r[2])  # best at top after barh-style inversion
    y = np.arange(len(pts))
    for i, (t, tok, score) in enumerate(pts):
        ax.errorbar(tok, i, xerr=decode_std(t), fmt="none", ecolor="#9da7b3",
                    elinewidth=1, capsize=2.5, alpha=.5, zorder=2)
        ax.scatter(tok, i, s=90, c=color(t), edgecolors="#0d1117", zorder=3)
        ax.text(tok * 1.12, i, f"{score:.0f}%", va="center", fontsize=8, color="#e6edf3")
    ax.set_yticks(y, [label(t) for t, _, _ in pts], fontsize=9)
    ax.set_xscale("log")
    ax.set_xticks([10, 20, 30, 50, 80, 130])
    ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    ax.set_xlabel("decode speed (tok/s, log scale) — whiskers are ±1 std across 3 runs")
    ax.set_title("Score vs speed — rows sorted by coding score, rightward is faster "
                 "(green = MoE, amber = dense)")
    ax.grid(axis="x", alpha=.3, which="both")
    ax.margins(x=.12)
    save(fig, "score-vs-speed.png")

# ---- 2. per-suite heatmap --------------------------------------------------
# Easy / hard / brutal per language, then the research paper. Brutal stays
# out of the headline 126-task score so historical totals stay comparable,
# but it belongs next to the other pass rates so you can see the drop.
HEATMAP = [("ceval", "C"), ("chard", "C hard"), ("brutal-c", "C brutal"),
           ("python", "Py"), ("pyhard", "Py hard"), ("brutal-python", "Py brutal"),
           ("bash", "Bash"), ("shhard", "Bash hard"), ("brutal-bash", "Bash brutal"),
           ("research", "Research")]
data = []
keep = []
for t in ms:
    row = []
    for s, _ in HEATMAP:
        v = (load(f"{t}-{s}") or [None])[0]
        row.append(v["passed"] / v["total"] if v and v["total"] else np.nan)
    if not all(np.isnan(row)):
        data.append(row); keep.append(t)
fig, ax = plt.subplots(figsize=(11.5, max(5, .32 * max(len(keep), 1))))
if skip_empty(data, fig, "suite-heatmap.png"):
    pass
else:
    order = np.argsort([-np.nanmean(r) for r in data])
    data = [data[i] for i in order]; keep = [keep[i] for i in order]
    im = ax.imshow(np.array(data) * 100, cmap="RdYlGn", vmin=0, vmax=100, aspect="auto")
    ax.set_xticks(range(len(HEATMAP)), [l for _, l in HEATMAP], rotation=35, ha="right", fontsize=8)
    ax.set_yticks(range(len(keep)), [label(t) for t in keep])
    for i, row in enumerate(data):
        for j, v in enumerate(row):
            if not np.isnan(v):
                ax.text(j, i, f"{v*100:.0f}", ha="center", va="center", fontsize=6.5,
                        color="#0d1117")
    ax.set_title("Pass rate per suite (%) — easy, hard, brutal")
    fig.colorbar(im, ax=ax, shrink=.6, label="%")
    save(fig, "suite-heatmap.png")

# ---- 3. brutal set ---------------------------------------------------------
fig, ax = plt.subplots(figsize=(9, 6))
rows = []
for t in ms:
    p, tt = suite_totals(t, BRUTAL_SUITES)
    if tt:
        rows.append((t, p, tt))
if skip_empty(rows, fig, "brutal.png"):
    pass
else:
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
# Labels live in a right gutter, spread so they cannot stack; a leader line
# ties each name back to its last point.
from matplotlib.transforms import blended_transform_factory
plotted = []
for t in ms:
    d = latest(f"concurrency/{t}-20*.json")
    if d and d.get("levels"):
        plotted.append((t, d))
plotted.sort(key=lambda td: -(td[1]["levels"][-1]["aggregate_tok_s"]))
palette = (list(matplotlib.colormaps["tab20"].colors)
           + list(matplotlib.colormaps["tab20b"].colors))
fig, ax = plt.subplots(figsize=(12.2, 8.8))
ends = []
for i, (t, d) in enumerate(plotted):
    lv = d["levels"]
    xs = [l["level"] for l in lv]
    ys = [l["aggregate_tok_s"] for l in lv]
    c = palette[i % len(palette)]
    ax.plot(xs, ys, marker="o", ms=3, lw=1.4, color=c, alpha=.9)
    ends.append((xs[-1], ys[-1], label(t), c))
ymax = max(y for _, y, _, _ in ends) * 1.06 if ends else 1
ax.set_ylim(0, ymax)
xmax = max(x for x, _, _, _ in ends) if ends else 8
ax.set_xlim(0, xmax)
ylabs = spread_y(
    [y for _, y, _, _ in ends],
    ymax * 0.02, ymax * 0.98,
    ymax / max(len(ends) * 1.2, 10),
)
trans = blended_transform_factory(ax.transAxes, ax.transData)
for (x, y, lab, c), yl in zip(ends, ylabs):
    ax.annotate(
        lab, xy=(x, y), xycoords="data",
        xytext=(1.03, yl), textcoords=trans,
        fontsize=7.5, color=c, va="center", ha="left",
        arrowprops=dict(arrowstyle="-", lw=.7, color=c, alpha=.45,
                        shrinkA=3, shrinkB=2),
        annotation_clip=False, clip_on=False,
    )
ax.set_xlabel("concurrent streams")
ax.set_ylabel("aggregate tok/s")
ax.set_title("Throughput as streams multiply — labels in the right gutter, "
             "one pass per level")
ax.grid(alpha=.3)
ticks = [1, 2, 4, 8, 12, 16]
ax.set_xticks([t for t in ticks if t <= xmax])
ax.set_xlim(0, xmax)
fig.subplots_adjust(left=0.08, right=0.70, top=0.92, bottom=0.08)
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
if not grid:
    plt.close(fig)
    print("skip framing-delta.png (no framing JSON)")
else:
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
if skip_empty(rows, fig, "repair.png"):
    pass
else:
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
if skip_empty(rows, fig, "generated-code-speed.png"):
    pass
else:
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

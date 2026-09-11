#!/usr/bin/env python3
"""Stage 2: do the framings that worked on the speed probe also work on correctness?

Stage 1 found three framings that move a model from an O(n*q) loop to a prefix sum.
But with seven conditions tested, something was always going to look good by chance,
and "write faster code" is a narrow ask -- a framing could plausibly buy speed without
buying correctness. So the winners are re-run here against a different target: the six
brutal tasks, where the outcome is simply whether the code is right.

Only framings that survive both stages are worth believing. The persona contrast
(claiming *you* are an expert vs telling the *model* it is one) is carried over
because that asymmetry is the most surprising thing stage 1 turned up.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import sys
import tempfile
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import eval_bash
import eval_code
import eval_python
from bench import TARGETS, complete_openai_full
from eval_framing import fisher_exact, wilson

RESULTS = Path(__file__).resolve().parent.parent / "results" / "framing-brutal"
PY = str(Path(__file__).resolve().parent.parent / ".venv" / "bin" / "python")

# "developer" wording has to track the language or the claim reads as a non sequitur
LANGS = {
    "c":      {"label": "C11",      "who": "C",      "mod": eval_code},
    "python": {"label": "Python 3", "who": "Python", "mod": eval_python},
    "bash":   {"label": "bash",     "who": "shell",  "mod": eval_bash},
}

CONDITIONS = [
    {"name": "bare", "prefix": "", "suffix": "", "what": "baseline"},
    {"name": "user_expert", "prefix": "I am a very experienced {who} developer. ",
     "suffix": "", "what": "asker claims expertise"},
    {"name": "model_persona",
     "prefix": "You are a senior {who} programmer with 20 years of experience. ",
     "suffix": "", "what": "model is given the persona"},
    {"name": "think", "suffix": " Think carefully about the edge cases before writing.",
     "prefix": "", "what": "nudge toward deliberation"},
]


def tasks_for(lang: str) -> list[dict[str, str]]:
    return LANGS[lang]["mod"].task_set("brutal")


def build_prompt(lang: str, task: dict[str, str], cond: dict[str, str]) -> str:
    meta = LANGS[lang]
    pre = cond["prefix"].format(who=meta["who"])
    suf = cond["suffix"]
    if lang == "c":
        body = (f"Implement in C11: `{task['sig']}`. {task['prompt']}{suf}\n"
                "Reply with only a C code block. No main function, no tests, no explanation.")
    elif lang == "python":
        body = (f"Implement in Python 3: `{task['sig']}` {task['prompt']}{suf}\n"
                "Reply with only a Python code block. No tests, no explanation.")
    else:
        body = (f"Implement in bash: `{task['sig']}`. {task['prompt']}{suf}\n"
                "Reply with only a bash code block defining the function. "
                "No tests, no explanation.")
    return pre + body


def run_one(target: str, lang: str, task: dict[str, str], cond: dict[str, str],
            temperature: float, timeout: float) -> str:
    cfg = TARGETS[target]
    harmony = target in eval_code.HARMONY_TARGETS
    thinking = target in eval_code.THINKING_TARGETS
    mod = LANGS[lang]["mod"]
    max_tok = mod.MAX_TOKENS_BRUTAL_HARMONY if (harmony or thinking) else mod.MAX_TOKENS_BRUTAL
    prompt = build_prompt(lang, task, cond)
    try:
        resp = complete_openai_full(port=cfg["port"], model=cfg["model"], prompt=prompt,
                                    max_tokens=max_tok, timeout=timeout,
                                    temperature=temperature)
    except Exception:  # noqa: BLE001
        return "http_error"
    reply = resp["text"]
    if harmony:
        reply = eval_code.strip_harmony(reply)
    if thinking and "</think>" in reply:
        reply = reply.split("</think>", 1)[1]

    if lang == "c":
        code = eval_code.extract_code(reply, task["sig"])
    elif lang == "python":
        func = (task["sig"].split("(")[0].replace("def ", "")
                .replace("class ", "").rstrip(":").strip())
        code = eval_python.extract_python(reply, func)
    else:
        code = eval_bash.extract_bash(reply, task["name"])

    with tempfile.TemporaryDirectory() as td:
        if lang == "python":
            status, _ = eval_python.grade(task, code, Path(td), PY)
        else:
            status, _ = mod.grade(task, code, Path(td))
    if status != "pass" and resp.get("completion_tokens", 0) >= max_tok:
        return "truncated"
    return status


def main() -> int:
    ap = argparse.ArgumentParser(description="Framing transfer test on the brutal set")
    ap.add_argument("--target", choices=tuple(TARGETS), required=True)
    ap.add_argument("--trials", type=int, default=4)
    ap.add_argument("--timeout", type=float, default=900.0)
    args = ap.parse_args()

    started = dt.datetime.now()
    doc: dict[str, Any] = {"target": args.target, "model": TARGETS[args.target]["model"],
                           "trials": args.trials, "conditions": {}}
    for cond in CONDITIONS:
        passed = total = 0
        per_task: dict[str, list[str]] = {}
        for lang in LANGS:
            for task in tasks_for(lang):
                outs = [run_one(args.target, lang, task, cond, 0.7, args.timeout)
                        for _ in range(args.trials)]
                per_task[f"{lang}:{task['name']}"] = outs
                passed += sum(1 for o in outs if o == "pass")
                total += len(outs)
        lo, hi = wilson(passed, total)
        doc["conditions"][cond["name"]] = {
            "what": cond["what"], "passed": passed, "total": total,
            "rate": round(passed / total, 3), "ci95": [round(lo, 3), round(hi, 3)],
            "per_task": per_task,
        }
        print(f"  {args.target} {cond['name']:14} {passed:3}/{total:<3} "
              f"{passed/total:5.0%}  [{lo:.0%}-{hi:.0%}]", file=sys.stderr, flush=True)

    base = doc["conditions"]["bare"]
    for name, c in doc["conditions"].items():
        c["p_vs_bare"] = None if name == "bare" else round(fisher_exact(
            c["passed"], c["total"] - c["passed"],
            base["passed"], base["total"] - base["passed"]), 4)

    doc["date"] = started.strftime("%Y-%m-%d %H:%M")
    RESULTS.mkdir(parents=True, exist_ok=True)
    path = RESULTS / f"{args.target}-{started.strftime('%Y%m%d-%H%M%S')}.json"
    path.write_text(json.dumps(doc, indent=2))
    print(f"wrote {path}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

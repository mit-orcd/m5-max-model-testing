#!/usr/bin/env python3
"""Self-repair eval (C, Python, Bash): round 1 is one-shot; rounds 2..N feed
back the failed code plus compiler/test output. Measures debugging ability
and token waste.

Per task: rounds_to_pass (0 = never), total tokens, total time.
Summary: one-shot passes, repaired-after-feedback, never-passed, median
rounds-to-pass, waste tokens (all tokens spent on tasks that needed >1 round).
"""
from __future__ import annotations

import argparse
import json
import statistics
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from bench import TARGETS, complete_openai_full  # noqa: E402
import eval_bash  # noqa: E402
import eval_code  # noqa: E402
import eval_python  # noqa: E402
from eval_code import HARMONY_TARGETS, THINKING_TARGETS, strip_harmony  # noqa: E402

PY = str(Path(__file__).parent.parent / ".venv" / "bin" / "python")


def _py_func(task: dict[str, str]) -> str:
    return (task["sig"].split("(")[0].replace("def ", "")
            .replace("class ", "").rstrip(":").strip())


LANGS = {
    "c": {
        "tasks": eval_code.task_set,
        "extract": eval_code.extract_code,
        "key": lambda t: t["sig"],
        "grade": lambda t, c, td: eval_code.grade(t, c, td, note_limit=1200),
        "fence": "c",
        "prompt": "Implement in C11: `{sig}`. {prompt}\n"
                  "Reply with only a C code block. No main function, no tests, no explanation.",
        "repair": "Your previous C solution to `{sig}` failed with: {status}.\n\n"
                  "Task: {prompt}\n\nYour code:\n```c\n{code}\n```\n\n"
                  "Compiler/test output:\n```\n{note}\n```\n\n"
                  "Return only the corrected complete C code block. "
                  "No main function, no tests, no explanation.",
        "max_tokens": eval_code.MAX_TOKENS,
        "max_tokens_harmony": eval_code.MAX_TOKENS_HARMONY,
    },
    "python": {
        "tasks": eval_python.task_set,
        "extract": eval_python.extract_python,
        "key": _py_func,
        "grade": lambda t, c, td: eval_python.grade(t, c, td, PY, note_limit=1200),
        "fence": "python",
        "prompt": "Implement in Python 3: `{sig}` {prompt}\n"
                  "Reply with only a Python code block. No tests, no explanation.",
        "repair": "Your previous Python solution to `{sig}` failed its tests.\n\n"
                  "Task: {prompt}\n\nYour code:\n```python\n{code}\n```\n\n"
                  "Test output:\n```\n{note}\n```\n\n"
                  "Return only the corrected complete Python code block. No tests, no explanation.",
        "max_tokens": eval_python.MAX_TOKENS,
        "max_tokens_harmony": eval_python.MAX_TOKENS_HARMONY,
    },
    "bash": {
        "tasks": eval_bash.task_set,
        "extract": eval_bash.extract_bash,
        "key": lambda t: t["name"],
        "grade": lambda t, c, td: eval_bash.grade(t, c, td, note_limit=1200),
        "fence": "bash",
        "prompt": "Implement in bash: `{sig}`. {prompt}\n"
                  "Reply with only a bash code block defining the function. No tests, no explanation.",
        "repair": "Your previous bash solution to `{sig}` failed its tests.\n\n"
                  "Task: {prompt}\n\nYour code:\n```bash\n{code}\n```\n\n"
                  "Test output:\n```\n{note}\n```\n\n"
                  "Return only the corrected complete bash code block defining the function. "
                  "No tests, no explanation.",
        "max_tokens": eval_bash.MAX_TOKENS,
        "max_tokens_harmony": eval_bash.MAX_TOKENS_HARMONY,
    },
}


def repair_target(name: str, timeout: float, max_rounds: int,
                  tasks: list[dict[str, str]], lang: str) -> dict:
    cfg = TARGETS[name]
    spec = LANGS[lang]
    per_task: dict[str, dict] = {}
    max_tok = (spec["max_tokens_harmony"]
               if name in HARMONY_TARGETS or name in THINKING_TARGETS
               else spec["max_tokens"])
    for task in tasks:
        base_prompt = spec["prompt"].format(sig=task["sig"], prompt=task["prompt"])
        prompt = base_prompt
        code = ""
        passed_round = 0
        rounds_used = 0
        tok_sum = 0
        time_sum = 0.0
        first_status = ""
        last_status = ""
        for rnd in range(1, max_rounds + 1):
            rounds_used = rnd
            try:
                resp = complete_openai_full(
                    port=cfg["port"], model=cfg["model"], prompt=prompt,
                    max_tokens=max_tok, timeout=timeout,
                    temperature=0.0 if rnd == 1 else 0.3,
                )
                reply = resp["text"]
                time_sum += resp["elapsed_s"]
                if resp["completion_tokens"]:
                    tok_sum += resp["completion_tokens"]
            except Exception as exc:  # noqa: BLE001
                last_status = f"http_error ({exc})"
                if rnd == 1:
                    first_status = "http_error"
                continue
            reply = strip_harmony(reply) if name in HARMONY_TARGETS else reply
            if name in THINKING_TARGETS and "</think>" in reply:
                reply = reply.split("</think>", 1)[1]
            code = spec["extract"](reply, spec["key"](task))
            with tempfile.TemporaryDirectory() as td:
                status, note = spec["grade"](task, code, Path(td))
            if rnd == 1:
                first_status = status
            last_status = status
            if status == "pass":
                passed_round = rnd
                break
            prompt = spec["repair"].format(
                sig=task["sig"], status=status, prompt=task["prompt"],
                code=code[:4000], note=(note or "no diagnostics")[:1200],
            )
        per_task[task["name"]] = {
            "passed_round": passed_round,
            "rounds_used": rounds_used,
            "tokens": tok_sum,
            "time_s": round(time_sum, 1),
            "first_status": first_status,
            "last_status": last_status,
        }
        mark = f"pass r{passed_round}" if passed_round else f"FAIL ({last_status})"
        print(f"{name} {task['name']}: {mark} rounds={rounds_used} tok={tok_sum}", flush=True)

    one_shot = sum(1 for t in per_task.values() if t["passed_round"] == 1)
    repaired = sum(1 for t in per_task.values() if t["passed_round"] > 1)
    never = sum(1 for t in per_task.values() if t["passed_round"] == 0)
    rounds_of_repaired = [t["passed_round"] for t in per_task.values() if t["passed_round"] > 1]
    waste = sum(t["tokens"] for t in per_task.values() if t["rounds_used"] > 1 and t["passed_round"] != 1)
    # waste = tokens spent on tasks that needed more than one round (incl. never-passing)
    return {
        "target": name,
        "model": cfg["model"],
        "lang": lang,
        "tasks": len(tasks),
        "one_shot": one_shot,
        "repaired": repaired,
        "never": never,
        "repair_rate": round(repaired / max(1, repaired + never), 3) if (repaired + never) else None,
        "median_rounds_repaired": statistics.median(rounds_of_repaired) if rounds_of_repaired else None,
        "total_tokens": sum(t["tokens"] for t in per_task.values()),
        "waste_tokens": waste,
        "total_time_s": round(sum(t["time_s"] for t in per_task.values()), 1),
        "per_task": per_task,
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", required=True, choices=tuple(TARGETS))
    ap.add_argument("--lang", choices=tuple(LANGS), default="c")
    ap.add_argument("--set", choices=("easy", "hard", "all"), default="all")
    ap.add_argument("--max-rounds", type=int, default=5)
    ap.add_argument("--timeout", type=float, default=300.0)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()
    spec = LANGS[args.lang]
    summary = repair_target(args.target, args.timeout, args.max_rounds,
                            spec["tasks"](args.set), args.lang)
    if args.json:
        print(json.dumps([summary], indent=2))
    else:
        print(f"{args.target} ({args.lang}): one-shot {summary['one_shot']}/{summary['tasks']}, "
              f"repaired {summary['repaired']}, never {summary['never']}, "
              f"waste {summary['waste_tokens']} tok")


if __name__ == "__main__":
    main()

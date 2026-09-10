#!/usr/bin/env python3
"""Verifiable Python coding eval: does the model's code run and pass tests?

Same pattern as eval_code.py (C): extract the code block, run hidden asserts
in a subprocess, no LLM judge. 8 tasks x N trials.

Usage:
  scripts/eval_python.py --target gptoss --trials 3 --json
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).parent))
from bench import TARGETS, complete_openai_full  # noqa: E402
from eval_code import HARMONY_TARGETS, THINKING_TARGETS, strip_harmony  # noqa: E402

MAX_TOKENS = 1024
MAX_TOKENS_HARMONY = 4096
RUN_TIMEOUT = 10.0

TASKS: list[dict[str, str]] = [
    {
        "name": "flatten",
        "sig": "def flatten(lst):",
        "prompt": "Flattens a list of lists one level deep. Elements of the inner lists keep their order and type.",
        "test": """
from solution import flatten
assert flatten([[1, 2], [3], [], [4, 5]]) == [1, 2, 3, 4, 5]
assert flatten([]) == []
assert flatten([[], []]) == []
assert flatten([[1], [2], [3]]) == [1, 2, 3]
assert flatten([[1.5, "a"], [None]]) == [1.5, "a", None]
print("PASS")
""",
    },
    {
        "name": "parse_json_safely",
        "sig": "def parse_json_safely(s):",
        "prompt": "Parses s as JSON and returns the resulting object. Returns None if s is not valid JSON.",
        "test": """
from solution import parse_json_safely as p
assert p('{"a": 1}') == {"a": 1}
assert p('not json') is None
assert p('[1, 2]') == [1, 2]
assert p('{bad') is None
assert p('"str"') == "str"
assert p('') is None
print("PASS")
""",
    },
    {
        "name": "top_k_words",
        "sig": "def top_k_words(text, k):",
        "prompt": "Words are maximal runs of ASCII letters, case-insensitive. Returns a list of (word, count) tuples for the k most frequent words, most frequent first, ties broken alphabetically, words lowercased.",
        "test": """
from solution import top_k_words as t
assert t("the cat The dog the", 2) == [("the", 3), ("cat", 1)]
assert t("a b c", 5) == [("a", 1), ("b", 1), ("c", 1)]
assert t("", 3) == []
assert t("x!x?y...z", 3) == [("x", 2), ("y", 1), ("z", 1)]
assert t("one two two three three three", 1) == [("three", 3)]
print("PASS")
""",
    },
    {
        "name": "is_valid_ipv4",
        "sig": "def is_valid_ipv4(s):",
        "prompt": "Returns True if s is a dotted-quad IPv4 address: four decimal octets 0-255, no leading zeros unless the octet is exactly '0'. Otherwise False.",
        "test": """
from solution import is_valid_ipv4 as v
assert v("1.2.3.4") is True
assert v("0.0.0.0") is True
assert v("255.255.255.255") is True
assert v("256.1.1.1") is False
assert v("1.2.3") is False
assert v("1.2.3.4.5") is False
assert v("a.b.c.d") is False
assert v("01.2.3.4") is False
assert v("1.2.3.") is False
assert v("") is False
print("PASS")
""",
    },
    {
        "name": "merge_intervals",
        "sig": "def merge_intervals(intervals):",
        "prompt": "intervals is a list of [start, end] pairs (start <= end, any order). Returns the merged list of non-overlapping intervals sorted by start. Touching intervals merge ([1,4] and [4,5] -> [1,5]).",
        "test": """
from solution import merge_intervals as m
assert m([[1, 3], [2, 4], [8, 9]]) == [[1, 4], [8, 9]]
assert m([]) == []
assert m([[5, 5]]) == [[5, 5]]
assert m([[1, 4], [4, 5]]) == [[1, 5]]
assert m([[7, 9], [1, 2], [4, 6], [2, 3]]) == [[1, 3], [4, 6], [7, 9]]
print("PASS")
""",
    },
    {
        "name": "run_length_encode",
        "sig": "def run_length_encode(s):",
        "prompt": "Returns run-length encoding of s as a list of (char, count) tuples.",
        "test": """
from solution import run_length_encode as r
assert r("aabccca") == [("a", 2), ("b", 1), ("c", 3), ("a", 1)]
assert r("") == []
assert r("x") == [("x", 1)]
assert r("aaAA") == [("a", 2), ("A", 2)]
print("PASS")
""",
    },
    {
        "name": "deep_get",
        "sig": "def deep_get(d, path, default=None):",
        "prompt": "Follows a dotted path through nested dicts (deep_get({'a': {'b': 1}}, 'a.b') == 1). Returns default if any key is missing or a non-dict is encountered mid-path.",
        "test": """
from solution import deep_get as g
d = {"a": {"b": {"c": 42}}, "x": 1}
assert g(d, "a.b.c") == 42
assert g(d, "a.b") == {"c": 42}
assert g(d, "a.b.missing") is None
assert g(d, "a.b.missing", "fb") == "fb"
assert g(d, "x.y", "fb") == "fb"
assert g({}, "a") is None
print("PASS")
""",
    },
    {
        "name": "csv_column_sum",
        "sig": "def csv_column_sum(path, col):",
        "prompt": "Reads the CSV file at path (first row is a header). Returns the sum of the numeric values in column col as a float.",
        "test": """
import os
from solution import csv_column_sum
with open("t.csv", "w") as f:
    f.write("a,b,name\\n1,2,x\\n3,4,y\\n-1.5,0,z\\n")
assert csv_column_sum("t.csv", "a") == 2.5
assert csv_column_sum("t.csv", "b") == 6.0
with open("empty.csv", "w") as f:
    f.write("a,b\\n")
assert csv_column_sum("empty.csv", "a") == 0.0
print("PASS")
""",
    },
    {
        "name": "topological_sort",
        "sig": "def topological_sort(deps):",
        "prompt": ("deps maps each node (str) to a list of nodes it depends on. Returns a "
                   "list of all nodes in an order where every node comes after its "
                   "dependencies. Returns None if the graph has a cycle (including "
                   "self-dependencies). Nodes mentioned only as dependencies are included."),
        "hard": "1",
        "test": """
from solution import topological_sort as ts
r = ts({"app": ["lib"], "lib": ["base"], "base": []})
assert r.index("base") < r.index("lib") < r.index("app"), r
r = ts({"a": ["b"], "b": ["c"], "c": [], "d": ["c"]})
assert r.index("c") < r.index("a") and r.index("c") < r.index("b") and r.index("b") < r.index("a") and r.index("c") < r.index("d"), r
assert ts({"a": ["b"], "b": ["a"]}) is None
assert ts({"x": ["x"]}) is None
assert ts({}) == []
r = ts({"a": ["ghost"]})
assert set(r) == {"a", "ghost"} and r.index("ghost") < r.index("a"), r
print("PASS")
""",
    },
    {
        "name": "lru_cache",
        "sig": "class LRUCache:",
        "prompt": ("A fixed-capacity LRU cache. __init__(self, capacity) with capacity >= 1; "
                   "get(self, key, default=None) returns the value (marking it most-recently-used) "
                   "or default; put(self, key, value) inserts/updates (marking most-recently-used) "
                   "and evicts the least-recently-used item when over capacity."),
        "hard": "1",
        "test": """
from solution import LRUCache
c = LRUCache(2)
c.put("a", 1); c.put("b", 2)
assert c.get("a") == 1
c.put("c", 3)  # evicts b
assert c.get("b") is None
assert c.get("a") == 1 and c.get("c") == 3
c.put("a", 10)  # update, refresh
c.put("d", 4)   # evicts c
assert c.get("c") is None and c.get("a") == 10 and c.get("d") == 4
c1 = LRUCache(1)
c1.put("x", 1); c1.put("y", 2)
assert c1.get("x") is None and c1.get("y") == 2
assert c1.get("missing", "fb") == "fb"
print("PASS")
""",
    },
    {
        "name": "json_diff",
        "sig": "def json_diff(a, b):",
        "prompt": ("Compares two JSON-like structures (dicts, lists, scalars). Returns a sorted "
                   "list of dotted key paths where they differ: changed scalar/list values, keys "
                   "present in only one side. Dicts are recursed into; lists and scalars are "
                   "compared as whole values. Empty list means identical."),
        "hard": "1",
        "test": """
from solution import json_diff as jd
assert jd({"a": 1}, {"a": 1}) == []
assert jd({"a": 1}, {"a": 2}) == ["a"]
assert jd({"a": {"b": 1, "c": 2}}, {"a": {"b": 1, "c": 3}}) == ["a.c"]
assert jd({"a": 1}, {"a": 1, "b": 2}) == ["b"]
assert jd({"a": 1, "b": 2}, {"a": 1}) == ["b"]
assert jd({"x": [1, 2]}, {"x": [1, 3]}) == ["x"]
assert jd({}, {"n": {"m": {}}}) == ["n"]
assert jd({"a": {"b": 1}}, {"a": {"b": 1}, "c": {"d": 4}}) == ["c"]
print("PASS")
""",
    },
]


def extract_python(text: str, func: str) -> str:
    blocks = re.findall(r"```[A-Za-z0-9+#]*[ \t]*\n(.*?)```", text, re.S)
    if not blocks:
        blocks = re.findall(r"```[A-Za-z0-9+#]*(.*?)\n```", text, re.S)
    needles = (f"def {func}", f"class {func}")
    for block in blocks:
        if any(n in block for n in needles):
            return block.strip()
    if blocks:
        return max(blocks, key=len).strip()
    return text.strip() if any(n in text for n in needles) else ""


def grade(task: dict[str, str], code: str, workdir: Path, py: str,
          note_limit: int | None = None) -> tuple[str, str]:
    if not code:
        return "no_code", ""
    (workdir / "solution.py").write_text(code)
    (workdir / "test.py").write_text(task["test"])
    try:
        run = subprocess.run(
            [py, "test.py"], cwd=workdir, capture_output=True, text=True, errors="replace",
            timeout=RUN_TIMEOUT,
        )
    except subprocess.TimeoutExpired:
        return "timeout", ""
    if run.returncode == 0 and "PASS" in run.stdout:
        return "pass", ""
    if note_limit:
        out = (run.stderr or "").strip() or (run.stdout or "").strip()
        return "fail", out[-note_limit:]
    err = (run.stderr or run.stdout).strip().splitlines()
    return "fail", (err[-1][:120] if err else "")


def task_set(which: str) -> list[dict[str, str]]:
    if which == "easy":
        return [t for t in TASKS if not t.get("hard")]
    if which == "hard":
        return [t for t in TASKS if t.get("hard")]
    return TASKS


def eval_target(name: str, timeout: float, trials: int, dump_dir: str | None,
                py: str, tasks: list[dict[str, str]] | None = None) -> dict[str, Any]:
    cfg = TARGETS[name]
    tasks = tasks if tasks is not None else TASKS
    results: dict[str, list[str]] = {}
    notes: dict[str, str] = {}
    times: dict[str, list[float]] = {}
    tokens: dict[str, list[int]] = {}
    for task in tasks:
        func = (task["sig"].split("(")[0].replace("def ", "")
                .replace("class ", "").rstrip(":").strip())
        prompt = (
            f"Implement in Python 3: `{task['sig']}` {task['prompt']}\n"
            "Reply with only a Python code block. No tests, no explanation."
        )
        outcomes: list[str] = []
        max_tok = MAX_TOKENS_HARMONY if name in HARMONY_TARGETS or name in THINKING_TARGETS else MAX_TOKENS
        for trial in range(trials):
            temp = 0.0 if trial == 0 else 0.7
            try:
                resp = complete_openai_full(
                    port=cfg["port"], model=cfg["model"], prompt=prompt,
                    max_tokens=max_tok, timeout=timeout, temperature=temp,
                )
                reply = resp["text"]
                times.setdefault(task["name"], []).append(round(resp["elapsed_s"], 2))
                if resp["completion_tokens"]:
                    tokens.setdefault(task["name"], []).append(resp["completion_tokens"])
            except Exception as exc:  # noqa: BLE001
                outcomes.append("http_error")
                notes[task["name"]] = str(exc)[:100]
                continue
            if name in HARMONY_TARGETS:
                reply = strip_harmony(reply)
            if name in THINKING_TARGETS and "</think>" in reply:
                reply = reply.split("</think>", 1)[1]
            code = extract_python(reply, func)
            with tempfile.TemporaryDirectory() as td:
                status, note = grade(task, code, Path(td), py)
            if status != "pass" and dump_dir and code:
                safe = "".join(c if c.isalnum() else "-" for c in name)
                Path(dump_dir).mkdir(parents=True, exist_ok=True)
                (Path(dump_dir) / f"{safe}-{task['name']}-t{trial}.py").write_text(code)
            outcomes.append(status)
            if status != "pass" and not notes.get(task["name"]):
                notes[task["name"]] = note
        results[task["name"]] = outcomes
        marks = ",".join("✓" if s == "pass" else "✗" for s in outcomes)
        print(f"{name} {task['name']}: [{marks}] {notes.get(task['name'], '')}", flush=True)
    total_pass = sum(sum(1 for s in o if s == "pass") for o in results.values())
    return {
        "target": name, "model": cfg["model"], "lang": "python",
        "passed": total_pass, "total": len(tasks) * trials,
        "trials": trials, "results": results, "notes": notes,
        "time_s": times, "tokens": tokens,
        "total_time_s": round(sum(sum(v) for v in times.values()), 1),
        "total_tokens": sum(sum(v) for v in tokens.values()),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Verifiable Python coding eval")
    parser.add_argument("--target", choices=tuple(TARGETS), required=True)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--dump-failures", metavar="DIR", default=None)
    parser.add_argument("--set", choices=("easy", "hard", "all"), default="all", dest="task_set")
    args = parser.parse_args()
    py = str(Path(__file__).parent.parent / ".venv" / "bin" / "python")
    row = eval_target(args.target, args.timeout, args.trials, args.dump_failures, py,
                      task_set(args.task_set))
    if args.json:
        print(json.dumps([row], indent=2))
    else:
        print(f"\n{args.target}: {row['passed']}/{row['total']}")


if __name__ == "__main__":
    main()

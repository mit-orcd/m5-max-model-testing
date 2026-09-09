#!/usr/bin/env python3
"""Verifiable Bash coding eval: sourced functions tested by a hidden bash harness.

Same pattern as eval_code.py / eval_python.py. The model writes a bash code
block defining the requested function(s); the hidden test sources it and
checks stdout / exit codes / filesystem effects.

Usage:
  scripts/eval_bash.py --target gptoss --trials 3 --json
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
        "name": "count_matches",
        "sig": "count_matches <pattern> <file>",
        "prompt": "Prints the number of lines in <file> matching <pattern> (fixed string). Prints 0 when nothing matches.",
        "test": """
source solution.sh
printf 'foo\\nbar\\nfoo bar\\n' > f.txt
[ "$(count_matches foo f.txt)" = "2" ] || { echo "FAIL count foo"; exit 1; }
[ "$(count_matches zzz f.txt)" = "0" ] || { echo "FAIL count zzz"; exit 1; }
[ "$(count_matches bar f.txt)" = "2" ] || { echo "FAIL count bar"; exit 1; }
echo PASS
""",
    },
    {
        "name": "largest_file",
        "sig": "largest_file <dir>",
        "prompt": "Prints the path of the largest regular file under <dir> (recursive). Exactly one line, the path.",
        "test": """
source solution.sh
mkdir -p d/sub
printf 'x' > d/small.txt
head -c 5000 /dev/zero > d/sub/big.bin
printf 'yy' > d/mid.txt
got="$(largest_file d)"
[ "$got" = "d/sub/big.bin" ] || { echo "FAIL got $got"; exit 1; }
echo PASS
""",
    },
    {
        "name": "retry3",
        "sig": "retry3 <cmd...>",
        "prompt": "Runs the command; if it fails, retries up to 3 total attempts. Exits 0 if any attempt succeeds, otherwise exits with the last attempt's exit code.",
        "test": """
source solution.sh
rm -f tries
flaky() { n=$(cat tries 2>/dev/null || echo 0); n=$((n+1)); echo $n > tries; [ "$n" -ge 3 ]; }
retry3 flaky || { echo "FAIL flaky never succeeded"; exit 1; }
[ "$(cat tries)" = "3" ] || { echo "FAIL tried $(cat tries) times"; exit 1; }
if retry3 false 2>/dev/null; then echo "FAIL false succeeded"; exit 1; fi
echo PASS
""",
    },
    {
        "name": "extract_urls",
        "sig": "extract_urls",
        "prompt": "Reads stdin, prints every http:// or https:// URL found, one per line, in order of appearance. Nothing else.",
        "test": """
source solution.sh
out=$(printf 'see https://a.com/x and http://b.org/y?z=1, plus none\\nend https://c.net\\n' | extract_urls)
want='https://a.com/x
http://b.org/y?z=1
https://c.net'
[ "$out" = "$want" ] || { printf 'FAIL got:\\n%s\\n' "$out"; exit 1; }
echo PASS
""",
    },
    {
        "name": "find_dupes",
        "sig": "find_dupes <dir>",
        "prompt": ("Finds groups of regular files under <dir> (recursive) with identical content "
                   "(compare by MD5). Prints one group per line: space-separated paths, sorted, "
                   "only groups of 2+ files. Groups sorted by first path. Must work on macOS "
                   "(BSD userland: no GNU find -printf, no md5sum — use md5 -r)."),
        "hard": "1",
        "test": """
source solution.sh
mkdir -p d/a d/b
printf 'same-content' > d/a/one.txt
printf 'same-content' > d/b/two.txt
printf 'unique' > d/b/uniq.txt
mkdir -p d/c && printf 'same-content' > d/c/three.txt
printf 'other-pair' > d/x.bin
printf 'other-pair' > d/y.bin
out="$(find_dupes d)"
want='d/a/one.txt d/b/two.txt d/c/three.txt
d/x.bin d/y.bin'
[ "$out" = "$want" ] || { printf 'FAIL got:\\n%s\\n' "$out"; exit 1; }
echo PASS
""",
    },
    {
        "name": "top_errors",
        "sig": "top_errors <logfile> <n>",
        "prompt": ("Prints the n most frequent ERROR signatures in <logfile>. A signature is the "
                   "text after 'ERROR: ' with every run of digits replaced by '#'. Output one per "
                   "line as '<count> <signature>', sorted by count descending, then signature "
                   "ascending. Print nothing if there are no ERROR lines."),
        "hard": "1",
        "test": """
source solution.sh
cat > app.log <<'EOF'
2024-01-01 INFO: started
2024-01-01 ERROR: disk 12 full
2024-01-01 ERROR: timeout after 30 s
2024-01-01 ERROR: disk 99 full
2024-01-01 ERROR: timeout after 45 s
2024-01-01 ERROR: disk 7 full
2024-01-01 WARN: ignored
EOF
out="$(top_errors app.log 1)"
[ "$out" = "3 disk # full" ] || { printf 'FAIL top1 got:\\n%s\\n' "$out"; exit 1; }
out="$(top_errors app.log 2)"
want='3 disk # full
2 timeout after # s'
[ "$out" = "$want" ] || { printf 'FAIL top2 got:\\n%s\\n' "$out"; exit 1; }
printf 'INFO only\\n' > clean.log
[ -z "$(top_errors clean.log 5)" ] || { echo "FAIL clean log"; exit 1; }
echo PASS
""",
    },
    {
        "name": "backup_rotate",
        "sig": "backup_rotate <dir> <keep>",
        "prompt": ("In <dir>, deletes all but the <keep> newest files matching "
                   "backup-*.tar.gz (newest = lexicographically greatest names). Prints each "
                   "deleted file's name (not path), one per line, oldest first. Deletes nothing "
                   "if there are <= <keep> backups (prints nothing)."),
        "hard": "1",
        "test": """
source solution.sh
mkdir -p bk
for d in 0101 0102 0103 0104 0105; do printf x > "bk/backup-2026$d-120000.tar.gz"; done
printf x > bk/keep-me.txt
out="$(backup_rotate bk 2)"
want='backup-20260101-120000.tar.gz
backup-20260102-120000.tar.gz
backup-20260103-120000.tar.gz'
[ "$out" = "$want" ] || { printf 'FAIL got:\\n%s\\n' "$out"; exit 1; }
[ "$(ls bk | wc -l | tr -d ' ')" = "3" ] || { echo "FAIL dir has $(ls bk | wc -l) files"; exit 1; }
[ -f bk/keep-me.txt ] || { echo "FAIL deleted non-backup"; exit 1; }
[ -z "$(backup_rotate bk 5)" ] || { echo "FAIL rotated below keep"; exit 1; }
echo PASS
""",
    },
    {
        "name": "rotate",
        "sig": "rotate <file>",
        "prompt": "Rotates logs one level: <file>.1 becomes <file>.2, then <file> becomes <file>.1. If <file>.1 does not exist, just rename <file> to <file>.1.",
        "test": """
source solution.sh
echo old > log.1
echo new > log
rotate log
[ "$(cat log.2)" = "old" ] || { echo "FAIL .2"; exit 1; }
[ "$(cat log.1)" = "new" ] || { echo "FAIL .1"; exit 1; }
[ ! -e log ] || { echo "FAIL log still exists"; exit 1; }
echo solo > solo
rotate solo
[ "$(cat solo.1)" = "solo" ] && [ ! -e solo.2 ] || { echo "FAIL solo"; exit 1; }
echo PASS
""",
    },
    {
        "name": "csv_col",
        "sig": "csv_col <name>",
        "prompt": "Reads a CSV from stdin (first line is a header). Prints the values of the column named <name>, one per line, in order.",
        "test": """
source solution.sh
out=$(printf 'name,age,city\\nann,30,boston\\nbob,25,cambridge\\n' | csv_col age)
[ "$out" = "30
25" ] || { printf 'FAIL got:\\n%s\\n' "$out"; exit 1; }
out=$(printf 'name,age\\nann,30\\n' | csv_col name)
[ "$out" = "ann" ] || { echo "FAIL name: $out"; exit 1; }
echo PASS
""",
    },
    {
        "name": "sum_stdin",
        "sig": "sum_stdin",
        "prompt": "Reads integers from stdin, one per line, ignoring blank lines. Prints their sum.",
        "test": """
source solution.sh
[ "$(printf '1\\n\\n2\\n3\\n' | sum_stdin)" = "6" ] || { echo "FAIL basic"; exit 1; }
[ "$(printf -- '-5\\n10\\n' | sum_stdin)" = "5" ] || { echo "FAIL negative"; exit 1; }
[ "$(printf '\\n\\n' | sum_stdin)" = "0" ] || { echo "FAIL empty"; exit 1; }
echo PASS
""",
    },
    {
        "name": "json_get",
        "sig": "json_get <key>",
        "prompt": "Reads a flat JSON object on stdin and prints the string value of top-level <key>. python3 is available; jq is not.",
        "test": """
source solution.sh
[ "$(echo '{"name":"ada","lang":"c"}' | json_get name)" = "ada" ] || { echo "FAIL name"; exit 1; }
[ "$(echo '{"name":"ada","lang":"c"}' | json_get lang)" = "c" ] || { echo "FAIL lang"; exit 1; }
echo PASS
""",
    },
]


def extract_bash(text: str, func: str) -> str:
    blocks = re.findall(r"```[A-Za-z0-9+#]*[ \t]*\n(.*?)```", text, re.S)
    if not blocks:
        blocks = re.findall(r"```[A-Za-z0-9+#]*(.*?)\n```", text, re.S)
    for block in blocks:
        if f"{func}()" in block or f"{func} ()" in block or f"function {func}" in block:
            return block.strip()
    if blocks:
        return max(blocks, key=len).strip()
    return text.strip() if func in text else ""


def grade(task: dict[str, str], code: str, workdir: Path) -> tuple[str, str]:
    if not code:
        return "no_code", ""
    (workdir / "solution.sh").write_text(code)
    (workdir / "test.sh").write_text(task["test"])
    try:
        run = subprocess.run(
            ["bash", "test.sh"], cwd=workdir, capture_output=True, text=True,
            timeout=RUN_TIMEOUT,
        )
    except subprocess.TimeoutExpired:
        return "timeout", ""
    if run.returncode == 0 and "PASS" in run.stdout:
        return "pass", ""
    err = (run.stdout + run.stderr).strip().splitlines()
    return "fail", (err[-1][:120] if err else "")


def task_set(which: str) -> list[dict[str, str]]:
    if which == "easy":
        return [t for t in TASKS if not t.get("hard")]
    if which == "hard":
        return [t for t in TASKS if t.get("hard")]
    return TASKS


def eval_target(name: str, timeout: float, trials: int, dump_dir: str | None,
                tasks: list[dict[str, str]] | None = None) -> dict[str, Any]:
    cfg = TARGETS[name]
    tasks = tasks if tasks is not None else TASKS
    results: dict[str, list[str]] = {}
    notes: dict[str, str] = {}
    times: dict[str, list[float]] = {}
    tokens: dict[str, list[int]] = {}
    for task in tasks:
        func = task["name"]
        prompt = (
            f"Implement in bash: `{task['sig']}`. {task['prompt']}\n"
            "Reply with only a bash code block defining the function. No tests, no explanation."
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
            code = extract_bash(reply, func)
            with tempfile.TemporaryDirectory() as td:
                status, note = grade(task, code, Path(td))
            if status != "pass" and dump_dir and code:
                safe = "".join(c if c.isalnum() else "-" for c in name)
                Path(dump_dir).mkdir(parents=True, exist_ok=True)
                (Path(dump_dir) / f"{safe}-{task['name']}-t{trial}.sh").write_text(code)
            outcomes.append(status)
            if status != "pass" and not notes.get(task["name"]):
                notes[task["name"]] = note
        results[task["name"]] = outcomes
        marks = ",".join("✓" if s == "pass" else "✗" for s in outcomes)
        print(f"{name} {task['name']}: [{marks}] {notes.get(task['name'], '')}", flush=True)
    total_pass = sum(sum(1 for s in o if s == "pass") for o in results.values())
    return {
        "target": name, "model": cfg["model"], "lang": "bash",
        "passed": total_pass, "total": len(tasks) * trials,
        "trials": trials, "results": results, "notes": notes,
        "time_s": times, "tokens": tokens,
        "total_time_s": round(sum(sum(v) for v in times.values()), 1),
        "total_tokens": sum(sum(v) for v in tokens.values()),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Verifiable Bash coding eval")
    parser.add_argument("--target", choices=tuple(TARGETS), required=True)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--dump-failures", metavar="DIR", default=None)
    parser.add_argument("--set", choices=("easy", "hard", "all"), default="all", dest="task_set")
    args = parser.parse_args()
    row = eval_target(args.target, args.timeout, args.trials, args.dump_failures,
                      task_set(args.task_set))
    if args.json:
        print(json.dumps([row], indent=2))
    else:
        print(f"\n{args.target}: {row['passed']}/{row['total']}")


if __name__ == "__main__":
    main()

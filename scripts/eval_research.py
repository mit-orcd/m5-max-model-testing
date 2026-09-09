#!/usr/bin/env python3
"""Research eval: long-context extraction over real RHEL 10 documentation.

The model gets curated excerpts from the RHEL 10.0 release notes and the
"Managing file systems" NFS chapter (data/research/rhel10-nfs.txt) and must
summarize every item relevant to NFS server/client performance or bug fixes.

Deterministic scoring, no LLM judge:
  - HITS: must mention at least MIN_HITS of the genuinely NFS-relevant facts
  - TRAPS: must NOT drag in unrelated storage/cluster fixes from the same text
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).parent))
from bench import TARGETS, complete_openai_full  # noqa: E402
from eval_code import HARMONY_TARGETS, THINKING_TARGETS, strip_harmony  # noqa: E402

DATA = Path(__file__).parent.parent / "data" / "research" / "rhel10-nfs.txt"
MAX_TOKENS = 4096

# (label, regex) — genuinely NFS-relevant facts present in the source text
HITS: list[tuple[str, str]] = [
    ("nfs-tls", r"\bTLS\b|kTLS"),
    ("rsize-wsize-1mb", r"1,?048,?576|1\s?MB|1\s?MiB"),
    ("krb5p-overhead", r"krb5p"),
    ("fs-cache", r"FS-Cache|cachefilesd|cachefiles"),
    ("retrans-timeo", r"retrans|timeo"),
    ("rear-ipv6", r"ReaR|IPv6"),
    ("reparse", r"reparse"),
]
MIN_HITS = 5

# Unrelated fixes present in the same excerpts — mentioning them = extraction failure
TRAPS: list[tuple[str, str]] = [
    ("luks", r"LUKS|Kickstart"),
    ("multipath", r"multipath|ontap"),
    ("nvme", r"NVMe"),
    ("iscsi", r"iSCSI|iscsi_ibft"),
    ("fstrim", r"fstrim|ostree"),
    ("cluster", r"\bpcs\b|SBD|Booth|Pacemaker"),
]
MAX_TRAPS = 1

PROMPT = """Below are excerpts from Red Hat Enterprise Linux 10 documentation (release notes and the "Managing file systems" guide).

Study them and summarize every item relevant to **NFS server or client performance, or NFS-related bug fixes**, as a concise bullet list. For each item say what changed and why it matters. Do not include items unrelated to NFS.

--- DOCUMENTATION EXCERPTS ---
{doc}
--- END ---

Reply with only the bullet list."""


def score(reply: str) -> tuple[str, str]:
    hits = [label for label, rx in HITS if re.search(rx, reply, re.I)]
    traps = [label for label, rx in TRAPS if re.search(rx, reply)]
    ok = len(hits) >= MIN_HITS and len(traps) <= MAX_TRAPS
    note = f"hits {len(hits)}/{len(HITS)} ({','.join(hits)}) traps {len(traps)} ({','.join(traps)})"
    return ("pass" if ok else "fail"), note


def eval_target(name: str, timeout: float, trials: int, dump_dir: str | None) -> dict[str, Any]:
    cfg = TARGETS[name]
    doc = DATA.read_text()
    prompt = PROMPT.format(doc=doc)
    outcomes: list[str] = []
    notes: dict[str, str] = {}
    times: list[float] = []
    tokens: list[int] = []
    for trial in range(trials):
        temp = 0.0 if trial == 0 else 0.7
        try:
            resp = complete_openai_full(
                port=cfg["port"], model=cfg["model"], prompt=prompt,
                max_tokens=MAX_TOKENS, timeout=timeout, temperature=temp,
            )
            reply = resp["text"]
            times.append(round(resp["elapsed_s"], 2))
            if resp["completion_tokens"]:
                tokens.append(resp["completion_tokens"])
        except Exception as exc:  # noqa: BLE001
            outcomes.append("http_error")
            notes["rhel10_nfs"] = str(exc)[:100]
            continue
        if name in HARMONY_TARGETS:
            reply = strip_harmony(reply)
        if name in THINKING_TARGETS and "</think>" in reply:
            reply = reply.split("</think>", 1)[1]
        status, note = score(reply)
        if status != "pass" and dump_dir and reply:
            safe = "".join(c if c.isalnum() else "-" for c in name)
            Path(dump_dir).mkdir(parents=True, exist_ok=True)
            (Path(dump_dir) / f"{safe}-rhel10_nfs-t{trial}.md").write_text(reply)
        outcomes.append(status)
        notes["rhel10_nfs"] = note
        print(f"{name} rhel10_nfs t{trial}: {status} — {note}", flush=True)
    passed = sum(1 for s in outcomes if s == "pass")
    return {
        "target": name, "model": cfg["model"], "lang": "research",
        "passed": passed, "total": trials, "trials": trials,
        "results": {"rhel10_nfs": outcomes}, "notes": notes,
        "time_s": {"rhel10_nfs": times}, "tokens": {"rhel10_nfs": tokens},
        "total_time_s": round(sum(times), 1), "total_tokens": sum(tokens),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="RHEL 10 NFS research eval")
    parser.add_argument("--target", choices=tuple(TARGETS), required=True)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=300.0)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--dump-failures", metavar="DIR", default=None)
    args = parser.parse_args()
    row = eval_target(args.target, args.timeout, args.trials, args.dump_failures)
    if args.json:
        print(json.dumps([row], indent=2))
    else:
        print(f"\n{args.target}: {row['passed']}/{row['total']}")


if __name__ == "__main__":
    main()

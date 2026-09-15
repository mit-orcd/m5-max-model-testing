#!/usr/bin/env python3
"""Platform-aware server launcher for bench targets.

Picks the serving runtime from the target's `runtime` field in bench.TARGETS
(mlx / mlx-vlm on macOS; llamacpp / llamacpp-fork / vllm / ollama on Linux)
and builds the command. Used by run-all-benchmarks.sh, perplexity.sh and the
serve-*.sh wrappers so all platform knowledge lives in one place.

Usage:
  serve.py exec <target>           exec the server (replaces this process)
  serve.py print <target>          print the command without running it
  serve.py field <target> <name>   print one field: runtime|port|alias|serve_model|gguf
  serve.py list                    targets available on this platform

Binary resolution (Linux):
  llama-server:  $LLAMA_SERVER_BIN, else PATH, else ~/llama.cpp/build/bin/
  fork server:   $LLAMA_K2_SERVER_BIN, else ~/llama-k2/build/bin/
  vllm:          $VLLM_BIN, else .venv/bin/vllm, else PATH
GGUFs live in $MODELS_DIR (default ~/models).
"""

from __future__ import annotations

import os
import shlex
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from bench import MODELS_DIR, SYSTEM, TARGETS, resolve_gguf, serve_model  # noqa: E402

HOST = "127.0.0.1"


def _find_bin(env_var: str, *candidates: str) -> str:
    override = os.environ.get(env_var)
    if override:
        return override
    for cand in candidates:
        cand = os.path.expanduser(cand)
        if os.path.isabs(cand) or cand.startswith("~"):
            if os.path.exists(cand):
                return cand
        else:
            found = shutil.which(cand)
            if found:
                return found
    raise SystemExit(
        f"{env_var} not set and none of {candidates} found. "
        "On the Linux box, run scripts/linux-setup.sh first."
    )


def serve_argv(name: str) -> list[str]:
    t = TARGETS[name]
    runtime = t["runtime"]
    port = os.environ.get("PORT", str(t["port"]))  # PORT overrides (live stack)
    alias = t["model"]  # request-level model id (Linux: rewritten to alias)
    model = serve_model(name)
    ctx = str(t.get("ctx", 16384))
    extra = shlex.split(t.get("extra_args", ""))

    if runtime == "mlx":
        return [
            str(ROOT / ".venv/bin/mlx_lm.server"),
            "--model", model, "--host", HOST, "--port", port,
            "--max-tokens", "16384", *extra,
        ]
    if runtime == "mlx-vlm":
        return [
            str(ROOT / ".venv/bin/mlx_vlm.server"),
            "--model", model, "--host", HOST, "--port", port,
            "--max-tokens", "16384", "--max-kv-size", "65536", *extra,
        ]
    if runtime in ("llamacpp", "llamacpp-fork"):
        if runtime == "llamacpp-fork":
            binary = _find_bin("LLAMA_K2_SERVER_BIN", "~/llama-k2/build/bin/llama-server")
            default_ctx = "32768"
            parallel = ["--parallel", "8"]
        else:
            binary = _find_bin("LLAMA_SERVER_BIN", "llama-server", "~/llama.cpp/build/bin/llama-server")
            default_ctx = "16384"
            parallel = []
        ctx = os.environ.get("LLAMA_CTX") or str(t.get("ctx", int(default_ctx)))
        if os.environ.get("LLAMA_PARALLEL"):
            parallel = ["--parallel", os.environ["LLAMA_PARALLEL"]]
        return [
            binary,
            "-m", resolve_gguf(name), "--alias", alias,
            "--host", HOST, "--port", port,
            "-ngl", "99", "-c", ctx, "--flash-attn", "on",
            *parallel, *extra,
        ]
    if runtime == "vllm":
        binary = _find_bin("VLLM_BIN", str(ROOT / ".venv/bin/vllm"), "vllm")
        return [
            binary, "serve", model,
            "--served-model-name", alias,
            "--host", HOST, "--port", port,
            "--max-model-len", ctx,
            *extra,
        ]
    if runtime == "ollama":
        raise SystemExit(
            f"target {name!r} is served by the Ollama daemon; "
            f"run `ollama pull {model}` and leave `ollama serve` running."
        )
    raise SystemExit(f"target {name!r}: unknown runtime {runtime!r}")


def field(name: str, key: str) -> str:
    t = TARGETS[name]
    if key == "runtime":
        return t["runtime"]
    if key == "port":
        return str(t["port"])
    if key == "alias":
        return t["model"]
    if key == "serve_model":
        return serve_model(name)
    if key == "gguf":
        return resolve_gguf(name)
    raise SystemExit(f"unknown field {key!r} (runtime|port|alias|serve_model|gguf)")


def main() -> None:
    if len(sys.argv) < 3:
        print(__doc__.strip())
        raise SystemExit(2)
    cmd, name = sys.argv[1], sys.argv[2]
    if cmd == "exec":
        argv = serve_argv(name)
        os.execvp(argv[0], argv)
    elif cmd == "print":
        print(" ".join(shlex.quote(a) for a in serve_argv(name)))
    elif cmd == "field" and len(sys.argv) == 4:
        print(field(name, sys.argv[3]))
    elif cmd == "list":
        for tname in TARGETS:
            print(tname)
    else:
        print(__doc__.strip())
        raise SystemExit(2)


if __name__ == "__main__":
    main()

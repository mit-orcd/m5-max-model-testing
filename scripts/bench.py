#!/usr/bin/env python3
"""Compare MLX :8080 vs Ollama :11434 on the local 27B Qwen pair.

Warmup is discarded. Median of --trials runs.
Ollama uses the native /api/chat eval_* timings (not OpenAI chunk math).
Do not run both servers at once if you want a clean GPU comparison.
"""

from __future__ import annotations

import argparse
import glob
import json
import os
import platform
import statistics
import subprocess
import sys
import threading
import time
from collections.abc import Callable
from pathlib import Path
from typing import Any

import httpx

SYSTEM = "darwin" if platform.system() == "Darwin" else "linux"
MODELS_DIR = os.environ.get("MODELS_DIR", str(Path.home() / "models"))

# Pinned mode: the cross-machine apples-to-apples switch. Every trial at
# temperature 0 with a fixed seed, C compiled with the same flags on every
# libc, and a fingerprint of what actually served the request stored in the
# result. Set BENCH_PINNED=1; results go to results/pinned/ via the runners.
PINNED = os.environ.get("BENCH_PINNED", "") not in ("", "0")
PINNED_SEED = int(os.environ.get("BENCH_SEED", "42"))


def pinned_model(name: str, fallback: str) -> str:
    """In pinned mode every machine requests the Linux alias, so the Mac does
    not send its MLX repo id to a llama-server started with that alias."""
    if PINNED and name in LINUX:
        return LINUX[name].get("alias", fallback)
    return fallback


def pinned_temperature(trial: int, requested: float) -> float:
    """Temperature 0 on every trial in pinned mode; otherwise what was asked."""
    return 0.0 if PINNED else requested


def request_sampling() -> dict[str, Any]:
    """Extra sampling fields for a chat request in pinned mode."""
    return {"seed": PINNED_SEED} if PINNED else {}


def harness_fingerprint(port: int | None = None) -> dict[str, Any]:
    """What actually produced this result: server build, slots, compiler, OS.

    Stored beside every pinned score so a cross-machine difference can be
    traced to a concrete harness difference instead of guessed at.
    """
    fp: dict[str, Any] = {
        "pinned": PINNED,
        "seed": PINNED_SEED if PINNED else None,
        "os": f"{platform.system()} {platform.release()} {platform.machine()}",
        "python": platform.python_version(),
    }
    for key, argv in (("cc", ["cc", "--version"]), ("bash", ["bash", "--version"])):
        try:
            out = subprocess.run(argv, capture_output=True, text=True, timeout=10)
            fp[key] = (out.stdout or out.stderr).strip().splitlines()[0]
        except Exception:  # noqa: BLE001
            fp[key] = None
    fp["c_std"] = "gnu11" if PINNED else "c11"
    if port:
        try:
            with httpx.Client(timeout=5.0) as client:
                props = client.get(f"http://127.0.0.1:{port}/props").json()
            gen = props.get("default_generation_settings") or {}
            fp["server"] = {
                "build": props.get("build_info"),
                "model_path": props.get("model_path"),
                "n_ctx_per_slot": gen.get("n_ctx"),
                "total_slots": props.get("total_slots"),
                "chat_template_sha": __import__("hashlib").sha1(
                    (props.get("chat_template") or "").encode()).hexdigest()[:12],
            }
        except Exception:  # noqa: BLE001
            fp["server"] = None
    return fp

TARGETS = {
    "mlx": {
        "base": "http://127.0.0.1:8080/v1",
        "model": "mlx-community/Qwen3.8-27B-4bit",
        "port": 8080,
        "other": 11434,
        "kind": "openai",
    },
    "ollama": {
        "base": "http://127.0.0.1:11434",
        "model": "qwen3.8:27b-mlx",
        "port": 11434,
        "other": 8080,
        "kind": "ollama",
    },
    "k2horizon": {
        "base": "http://127.0.0.1:8085/v1",
        "model": "k2horizon",
        "port": 8085,
        "other": 8080,
        "kind": "openai",
    },
    "ornith": {
        "base": "http://127.0.0.1:8082/v1",
        "model": "mlx-community/Ornith-1.5-35B-A3B-OptiQ-4bit",
        "port": 8082,
        "other": 8080,
        "kind": "openai",
    },
    # Comparison set — served one at a time on :8083 (RAM budget).
    "coder": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3-Coder-30B-A3B-Instruct-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen35": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.5-35B-A3B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "gptoss": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/gpt-oss-20b-MXFP4-Q8",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "gptoss120": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/gpt-oss-120b-MXFP4-Q8",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "north": {
        "base": "http://127.0.0.1:11434",
        "model": "hf.co/unsloth/North-Mini-Code-1.0-GGUF:Q4_K_M",
        "port": 11434,
        "other": 8080,
        "kind": "ollama",
    },
    "llama33": {
        "base": "http://127.0.0.1:11434",
        "model": "llama3.3:70b",
        "port": 11434,
        "other": 8080,
        "kind": "ollama",
    },
    "qwen3-30b": {
        "base": "http://127.0.0.1:11434",
        "model": "qwen3:30b",
        "port": 11434,
        "other": 8080,
        "kind": "ollama",
    },
    "laguna": {
        "base": "http://127.0.0.1:8085/v1",
        "model": "laguna",
        "port": 8085,
        "other": 8080,
        "kind": "openai",
    },
    # The same weights as `laguna` above, served through MLX instead of the
    # llama.cpp fork. Scored separately so the XS.2-vs-2.1 comparison can hold the
    # runtime fixed: on its own, `laguna` differs from laguna21 by version *and*
    # stack, and the two cannot be told apart.
    "laguna-mlx": {
        "base": "http://127.0.0.1:8083/v1",
        "model": f"{MODELS_DIR}/Laguna-XS.2-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen38flash": {
        "base": "http://127.0.0.1:8085/v1",
        "model": "qwen38flash",
        "port": 8085,
        "other": 8080,
        "kind": "openai",
    },
    "gemma": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/gemma-4-26b-a4b-it-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "devstral": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Devstral-Small-2-24B-Instruct-2512-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen27": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.8-27B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
        # No `qwen27` class in mlx-lm's LM server; served with mlx_vlm.server.
        "runtime": "mlx-vlm",
    },
    # Hermes fast model (titles, compression) on :8081. Not benchmarked; the
    # entry exists so serve.py can launch it on either platform.
    "qwen8b-fast": {
        "base": "http://127.0.0.1:8081/v1",
        "model": "mlx-community/Qwen3-8B-4bit",
        "port": 8081,
        "other": 8080,
        "kind": "openai",
    },
    # Laguna XS 2.1 — the 33B-A3B successor to laguna (XS.2). mlx-lm has no
    # `laguna` class yet (ml-explore/mlx-lm#1223), so it is served with
    # mlx_vlm.server, the same runtime qwen27 uses.
    "laguna21": {
        "base": "http://127.0.0.1:8083/v1",
        "model": f"{MODELS_DIR}/Laguna-XS-2.1-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "aya": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/aya-23-35B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen36-27b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.6-27B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen36-35b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.6-35B-A3B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "devstral2": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Devstral-Small-2-24B-Instruct-2512-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "glm-flash": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/GLM-4.7-Flash-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "coder-next": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3-Coder-Next-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "deepseek-32b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/DeepSeek-R1-Distill-Qwen-32B-MLX-4Bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    # Second wave: families and sizes the first round never covered.
    "qwen35-122b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.5-122B-A10B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "qwen35-27b": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Qwen3.5-27B-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "laguna-s": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Laguna-S-2.1-oQ4e",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "nemotron3": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Nemotron-3-Super-120B-A12B-MLX-6bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "katcoder": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/KAT-Coder-V2.5-Dev-OptiQ-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "katcoder-reap": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/KAT-Coder-V2.5-Dev-OptiQ-4bit-REAP-18B",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "ling": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Ling-2.6-flash-mlx-4bit-DWQ",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    "seed-oss": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/Seed-OSS-36B-Instruct-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    # 119B / ~6B active. No MLX build; llama-k2 already has the mistral4 arch.
    # reasoning_effort=none keeps the standing thinking-off rule.
    "mistral-small4": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mistral-small-4",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
        "runtime": "llamacpp-fork",
        "alias": "mistral-small-4",
        "gguf": "mistralai_Mistral-Small-4-119B-2603*Q4_K_M*.gguf",
        "ctx": 16384,
        "extra_args": "--jinja --chat-template-kwargs '{\"reasoning_effort\":\"none\"}'",
    },
    # qwen3_5_moe arch, text-only MLX conversion of the multimodal original
    "nex25-mini": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "abenzerps/Nex-N2.5-mini-MLX-4bit",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
    # 97 GB of weights on a 128 GB box: no room for concurrent KV cache, and
    # mlx-lm 0.31.3 has no deepseek_v4 class — the repo ships one to drop in.
    "deepseek-v4": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "mlx-community/DeepSeek-V4-Flash-2bit-DQ",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
    },
}

# ---------------------------------------------------------------------------
# Linux serving map (orcd-office001: Rocky 10, RTX 6000 Pro Workstation 96 GB).
#
# Each entry overrides how a target is served when SYSTEM == "linux":
#   runtime: llamacpp | llamacpp-fork | vllm | ollama
#   model:   GGUF glob under $MODELS_DIR (llamacpp*; first sorted match is
#            served, so sharded quants work), HF repo id (vllm), ollama tag
#   alias:   model id sent in API requests (llama-server --alias /
#            vllm --served-model-name); defaults to the GGUF/HF name
#   ctx:     context size for the server (default 16384, fork targets 32768)
#   extra_args: extra server CLI flags (string, split on whitespace)
#
# Targets without an entry have no known Linux serving path (MLX-only quants)
# and are dropped from TARGETS on Linux. GGUF repo names live in
# scripts/models-linux.txt; verify them before downloading.
# ---------------------------------------------------------------------------
LINUX: dict[str, dict[str, Any]] = {
    # Live stack
    "gptoss": {"runtime": "llamacpp", "model": "gpt-oss-20b*.gguf", "alias": "gpt-oss-20b"},
    "qwen8b-fast": {"runtime": "llamacpp", "model": "Qwen3-8B*Q4_K_M*.gguf", "alias": "qwen-fast"},
    # Benchmark sweep (llama.cpp Vulkan)
    "gptoss120": {"runtime": "llamacpp", "model": "gpt-oss-120b*.gguf", "alias": "gpt-oss-120b"},
    "mlx": {"runtime": "llamacpp", "model": "Qwen3.8-27B*Q4_K_M*.gguf", "alias": "qwen3.8-27b"},
    "qwen27": {"runtime": "llamacpp", "model": "Qwen3.8-27B*Q4_K_M*.gguf", "alias": "qwen3.8-27b"},
    "coder": {"runtime": "llamacpp", "model": "Qwen3-Coder-30B-A3B-Instruct*Q4_K_M*.gguf", "alias": "qwen3-coder-30b"},
    "qwen35": {"runtime": "llamacpp", "model": "Qwen3.5-35B-A3B*Q4_K_M*.gguf", "alias": "qwen3.5-35b"},
    "gemma": {"runtime": "llamacpp", "model": "gemma-4-26B-A4B-it*Q4_K_M*.gguf", "alias": "gemma-4-26b"},
    "devstral": {"runtime": "llamacpp", "model": "Devstral-Small-2-24B-Instruct-2512*Q4_K_M*.gguf", "alias": "devstral-24b"},
    "mistral-small4": {"runtime": "llamacpp", "model": "mistralai_Mistral-Small-4-119B-2603*Q4_K_M*.gguf", "alias": "mistral-small-4", "ctx": 16384,
                       "extra_args": "--jinja --chat-template-kwargs '{\"reasoning_effort\":\"none\"}'"},
    "devstral2": {"runtime": "llamacpp", "model": "Devstral-Small-2-24B-Instruct-2512*Q4_K_M*.gguf", "alias": "devstral-24b"},
    "aya": {"runtime": "llamacpp", "model": "aya-23-35B*Q4_K_M*.gguf", "alias": "aya-35b"},
    "qwen36-27b": {"runtime": "llamacpp", "model": "Qwen3.6-27B*Q4_K_M*.gguf", "alias": "qwen3.6-27b"},
    "qwen36-35b": {"runtime": "llamacpp", "model": "Qwen3.6-35B-A3B*Q4_K_M*.gguf", "alias": "qwen3.6-35b"},
    "glm-flash": {"runtime": "llamacpp", "model": "GLM-4.7-Flash*Q4_K_M*.gguf", "alias": "glm-4.7-flash"},
    "coder-next": {"runtime": "llamacpp", "model": "Qwen3-Coder-Next*Q4_K_M*.gguf", "alias": "qwen3-coder-next"},
    "deepseek-32b": {"runtime": "llamacpp", "model": "DeepSeek-R1-Distill-Qwen-32B*Q4_K_M*.gguf", "alias": "deepseek-r1-32b"},
    "qwen35-122b": {"runtime": "llamacpp", "model": "Qwen3.5-122B-A10B*Q4_K_M*.gguf", "alias": "qwen3.5-122b"},
    "qwen35-27b": {"runtime": "llamacpp", "model": "Qwen3.5-27B*Q4_K_M*.gguf", "alias": "qwen3.5-27b"},
    "nemotron3": {"runtime": "llamacpp", "model": "NVIDIA-Nemotron-3-Super-120B-A12B*Q4_K_M*.gguf", "alias": "nemotron-3-120b"},
    # ling dropped on Linux: llama.cpp has no 'bailingmoe2.5' arch (MLX-only),
    # and the bf16 original is ~130 GB — too big for the 96 GB card.
    "seed-oss": {"runtime": "llamacpp", "model": "Seed-OSS-36B-Instruct*Q4_K_M*.gguf", "alias": "seed-oss-36b"},
    # GGUFs exist now; Mac used MLX 4-bit. Same weights, llama.cpp Q4_K_M.
    "katcoder": {"runtime": "llamacpp", "model": "Kwaipilot_KAT-Coder-V2.5-Dev*Q4_K_M*.gguf", "alias": "katcoder"},
    "ornith": {"runtime": "llamacpp", "model": "Ornith-1.5-35B*Q4_K_M*.gguf", "alias": "ornith"},
    # deepseek-v4 dropped on Linux: smallest GGUF (UD-IQ1_M, 87 GB) leaves no
    # KV headroom on a 96 GB card. The Mac ran it at 97 GB in unified memory.
    "laguna-s": {"runtime": "llamacpp", "model": "Laguna-S-2.1*Q4_K_M*.gguf", "alias": "laguna-s-2.1"},
    # MBZUAI-IFM fork targets (qwen4_exp / k2-horizon / laguna architectures)
    # No plain Q4_K_M exists for this 177B MoE; UD-Q2_K_XL (79 GB) is the
    # largest quant that leaves KV headroom on 96 GB. Mac used MLX 4-bit.
    # Fork targets get --parallel 8, so ctx must be 8x the per-request need
    # (the prefill bench prompt is ~11.4k tokens -> >=16k per slot).
    "qwen38flash": {"runtime": "llamacpp-fork", "model": "Qwen3.8-Flash-Next*Q2_K_XL*.gguf", "alias": "qwen38flash", "ctx": 131072},
    "k2horizon": {"runtime": "llamacpp-fork", "model": "K2-Horizon*Q4_K_M*.gguf", "alias": "k2horizon", "ctx": 131072},
    # Linux serves the official Laguna-XS-2.1 GGUF (poolside) where the Mac
    # benched XS.2 via the fork — mainline llama.cpp supports the arch, only
    # the chat template needs the file workaround.
    "laguna": {
        "runtime": "llamacpp",
        "model": "Laguna-XS-2.1*Q4_K_M*.gguf",
        "alias": "laguna",
        "extra_args": f"--chat-template-file {MODELS_DIR}/laguna-template.jinja",
    },
    # Ollama targets: same tags work on Linux Ollama
    "ollama": {"runtime": "ollama", "model": "qwen3.8:27b"},
    "north": {"runtime": "ollama", "model": "hf.co/unsloth/North-Mini-Code-1.0-GGUF:Q4_K_M"},
    "llama33": {"runtime": "ollama", "model": "llama3.3:70b"},
    "qwen3-30b": {"runtime": "ollama", "model": "qwen3:30b"},
}

# Linux-only targets: same weights as a llamacpp target, served through vLLM
# or SGLang (CUDA) so the stack comparison holds the model fixed.
# vLLM/SGLang serve HF originals (bf16; MXFP4 for gpt-oss), not GGUF.
# The Ollama-vs-vLLM-vs-SGLang pin is Qwen3.8-27B (qwen27-vllm / qwen27-sglang
# / ollama); Ollama is Q4 GGUF, the other two are the same HF bf16 tree.
LINUX_ONLY: dict[str, dict[str, Any]] = {
    "gptoss-vllm": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "gpt-oss-20b-vllm",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
        "runtime": "vllm",
        "linux_model": "openai/gpt-oss-20b",
    },
    # Hybrid linear-attention (Mamba) models: vLLM's CUDA graph capture needs
    # max_num_seqs <= available Mamba cache blocks, hence the cap.
    "qwen27-vllm": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "qwen3.8-27b-vllm",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
        "runtime": "vllm",
        "linux_model": "Qwen/Qwen3.8-27B",
        "extra_args": "--max-num-seqs 512",
    },
    "qwen27-sglang": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "qwen3.8-27b-sglang",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
        "runtime": "sglang",
        "linux_model": "Qwen/Qwen3.8-27B",
        "extra_args": "--mem-fraction-static 0.85",
    },
    "qwen35-vllm": {
        "base": "http://127.0.0.1:8083/v1",
        "model": "qwen3.5-35b-vllm",
        "port": 8083,
        "other": 8080,
        "kind": "openai",
        "runtime": "vllm",
        "linux_model": "Qwen/Qwen3.5-35B-A3B",
        "extra_args": "--max-num-seqs 512",
    },
}


def _default_runtime(t: dict[str, Any]) -> str:
    if t.get("kind") == "ollama":
        return "ollama"
    if t["port"] == 8085:
        # k2horizon / laguna / qwen38flash run on the MBZUAI-IFM llama.cpp
        # fork on the Mac too (mainline rejects those architectures).
        return "llamacpp-fork"
    return "mlx"


for _t in TARGETS.values():
    _t.setdefault("runtime", _default_runtime(_t))

if SYSTEM != "darwin":
    for _name in list(TARGETS):
        _lx = LINUX.get(_name)
        if _lx is None:
            # No Linux serving path mapped (MLX-only quant) — hide the target.
            del TARGETS[_name]
            continue
        _t = TARGETS[_name]
        _t["runtime"] = _lx["runtime"]
        _t["linux_model"] = _lx["model"]
        _t["model"] = _lx.get("alias", _lx["model"])
        for _k in ("ctx", "extra_args"):
            if _k in _lx:
                _t[_k] = _lx[_k]
    TARGETS.update(LINUX_ONLY)

# Mac stack pin: same bartowski Q4_K_M Linux/Strix use, served by llama-k2
# Metal and by Ollama from that file. MLX 4-bit stays `qwen27`. The existing
# `ollama` target remains qwen3.8:27b-mlx and is not replaced.
if SYSTEM == "darwin":
    TARGETS["qwen27-llamacpp"] = {
        "base": "http://127.0.0.1:8084/v1",
        "model": "qwen3.8-27b",
        "port": 8084,
        "other": 8080,
        "kind": "openai",
        "runtime": "llamacpp-fork",
        "alias": "qwen3.8-27b",
        "gguf": "Qwen3.8-27B*Q4_K_M*.gguf",
        "ctx": 131072,
    }
    TARGETS["qwen27-ollama"] = {
        "base": "http://127.0.0.1:11434",
        "model": "qwen3.8-27b-q4",
        "port": 11434,
        "other": 8080,
        "kind": "ollama",
        "runtime": "ollama",
    }


def serve_model(name: str) -> str:
    """Model identifier the server should load for this target, this platform."""
    t = TARGETS[name]
    if SYSTEM == "darwin":
        return t.get("gguf") or t["model"]
    return t.get("linux_model", t["model"])


def resolve_gguf(name: str) -> str:
    """Absolute path to a llamacpp target's GGUF (first shard if sharded)."""
    t = TARGETS[name]
    pat = t.get("gguf") or serve_model(name)
    if os.path.isabs(pat) and not any(c in pat for c in "*?"):
        return pat
    matches = sorted(glob.glob(os.path.join(MODELS_DIR, pat)))
    if not matches:
        raise SystemExit(
            f"no GGUF match for target {name!r}: {MODELS_DIR}/{pat}\n"
            "download it first: scripts/download-models.sh scripts/models-linux.txt"
        )
    return matches[0]


DECODE_PROMPT = (
    "Write a 1500-word paragraph about MIT and its influence on AI research and discoveries. Do not use a title."
)
PREFILL_UNIT = (
    "MIT has shaped AI research through pioneering labs, influential alumni, and decades of discovery. "
)
CASES = {
    "decode": {"max_tokens": 2048, "prompt": DECODE_PROMPT},
    "prefill": {
        "max_tokens": 128,
        "prompt": (PREFILL_UNIT * 670)
        + "\n\nSummarize the above in exactly three short bullet points.",
    },
}
MIN_DECODE_S = 0.05

# Deterministic quality probes. Each checker gets the raw reply text.
QUALITY_TASKS: list[tuple[str, Any]] = [
    ("arith", lambda t: "408" in t.replace(",", "")),
    ("exact", lambda t: t.strip() == "OK"),
    ("json", lambda t: _is_json_object(t)),
    ("primes", lambda t: all(p in t for p in ("2", "3", "5", "7", "11"))),
    ("code", lambda t: "[::-1]" in t.replace(" ", "")),
    ("follow", lambda t: t.strip().lower().startswith("red")),
]
QUALITY_PROMPTS = {
    "arith": "What is 17 * 24? Reply with only the number.",
    "exact": "Reply with exactly this token and nothing else: OK",
    "json": 'Output only a JSON object {"a": 1}. No prose, no code fences.',
    "primes": "List the first five prime numbers, comma separated, nothing else.",
    "code": "Write a Python expression (no prose) that reverses a string s.",
    "follow": "Answer in one lowercase word: what color is a ripe strawberry?",
}
QUALITY_MAX_TOKENS = 64


def _is_json_object(text: str) -> bool:
    try:
        return isinstance(json.loads(text.strip()), dict)
    except (json.JSONDecodeError, ValueError):
        return False


def _median(xs: list[float]) -> float | None:
    return statistics.median(xs) if xs else None


def _port_up(port: int) -> bool:
    try:
        r = httpx.get(f"http://127.0.0.1:{port}/v1/models", timeout=2.0)
        return r.status_code < 500
    except (httpx.RequestError, httpx.HTTPStatusError):
        return False


def _listen_pid(port: int) -> int | None:
    try:
        out = subprocess.check_output(
            ["lsof", "-tiTCP:%d" % port, "-sTCP:LISTEN"],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except subprocess.CalledProcessError:
        return None
    if not out:
        return None
    return int(out.split()[0])


def _pgrep(args: list[str]) -> list[int]:
    try:
        out = subprocess.check_output(
            ["pgrep", *args], text=True, stderr=subprocess.DEVNULL
        )
    except subprocess.CalledProcessError:
        return []
    pids = []
    for line in out.split():
        try:
            pids.append(int(line))
        except ValueError:
            continue
    return pids


def _mlx_pids() -> list[int]:
    pid = _listen_pid(8080)
    return [pid] if pid else []


def _openai_pids(port: int) -> list[int]:
    pid = _listen_pid(port)
    return [pid] if pid else []


def _ollama_pids() -> list[int]:
    pids = set(_pgrep(["-x", "ollama"]))
    pids.update(_pgrep(["-f", "ollama runner"]))
    listen = _listen_pid(11434)
    if listen:
        pids.add(listen)
        pids.update(_pgrep(["-P", str(listen)]))
    return [p for p in pids if p]


def _rss_mb(pid: int) -> float | None:
    try:
        out = subprocess.check_output(
            ["ps", "-o", "rss=", "-p", str(pid)],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except subprocess.CalledProcessError:
        return None
    if not out:
        return None
    return int(out.split()[0]) / 1024.0


class RssSampler:
    def __init__(self, pid_fn: Callable[[], list[int]], interval: float = 0.05):
        self.pid_fn = pid_fn
        self.interval = interval
        self.peak: float | None = None
        self._stop = threading.Event()
        self._thread: threading.Thread | None = None

    def start(self) -> None:
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _sum_rss(self) -> float | None:
        total = 0.0
        found = False
        for pid in self.pid_fn():
            mb = _rss_mb(pid)
            if mb is None:
                continue
            total += mb
            found = True
        return total if found else None

    def _run(self) -> None:
        while not self._stop.is_set():
            mb = self._sum_rss()
            if mb is not None:
                self.peak = mb if self.peak is None else max(self.peak, mb)
            self._stop.wait(self.interval)

    def stop(self) -> float | None:
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=1.0)
        return self.peak


def _first_str(*values: Any) -> str:
    for value in values:
        if isinstance(value, str) and value:
            return value
    return ""


def _openai_visible(event: dict[str, Any]) -> str:
    choices = event.get("choices") or []
    if not choices:
        return ""
    choice = choices[0]
    delta = choice.get("delta") or {}
    message = choice.get("message") or {}
    return _first_str(
        delta.get("content"),
        delta.get("reasoning_content"),
        delta.get("reasoning"),
        message.get("content"),
        message.get("reasoning_content"),
        message.get("reasoning"),
    )


def _ollama_visible(event: dict[str, Any]) -> str:
    message = event.get("message") or {}
    return _first_str(
        message.get("content"),
        message.get("thinking"),
        message.get("reasoning"),
        event.get("response"),
    )


def _sse_payloads(line: str) -> dict[str, Any] | None:
    line = line.strip()
    if not line.startswith("data:"):
        return None
    data = line[5:].strip()
    if not data or data == "[DONE]":
        return None
    try:
        obj = json.loads(data)
    except json.JSONDecodeError:
        return None
    return obj if isinstance(obj, dict) else None


def _ndjson(line: str) -> dict[str, Any] | None:
    line = line.strip()
    if not line:
        return None
    try:
        obj = json.loads(line)
    except json.JSONDecodeError:
        return None
    return obj if isinstance(obj, dict) else None


def _tok_s_from_ns(count: int | None, duration_ns: int | None) -> float | None:
    if not isinstance(count, int) or count < 1:
        return None
    if not isinstance(duration_ns, int) or duration_ns <= 0:
        return None
    return count / (duration_ns / 1e9)


def _tok_s_wall(completion_tokens: int, elapsed: float, ttft: float | None) -> float | None:
    if completion_tokens < 1 or elapsed <= 0:
        return None
    if ttft is None:
        return completion_tokens / elapsed
    decode_s = elapsed - ttft
    if decode_s < MIN_DECODE_S:
        return None
    return completion_tokens / decode_s


def stream_mlx(
    *,
    model: str,
    prompt: str,
    max_tokens: int,
    timeout: float,
    port: int = 8080,
    thinking: bool = False,
) -> dict[str, Any]:
    body: dict[str, Any] = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "max_tokens": max_tokens,
        "temperature": 0,
        "stream": True,
        "stream_options": {"include_usage": True},
        "chat_template_kwargs": {"enable_thinking": thinking},
    }
    sampler = RssSampler(lambda: _openai_pids(port))
    url = f"http://127.0.0.1:{port}/v1/chat/completions"
    start = time.perf_counter()
    ttft: float | None = None
    text_parts: list[str] = []
    completion_tokens: int | None = None
    prompt_tokens: int | None = None
    tok_s: float | None = None
    metric: str | None = None
    sampler.start()
    try:
        with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
            with client.stream("POST", url, json=body) as resp:
                resp.raise_for_status()
                for line in resp.iter_lines():
                    event = _sse_payloads(line)
                    if event is None:
                        continue
                    usage = event.get("usage")
                    if isinstance(usage, dict):
                        if isinstance(usage.get("completion_tokens"), int):
                            completion_tokens = usage["completion_tokens"]
                        if isinstance(usage.get("prompt_tokens"), int):
                            prompt_tokens = usage["prompt_tokens"]
                    timings = event.get("timings")
                    if isinstance(timings, dict):
                        predicted = timings.get("predicted_per_second")
                        if isinstance(predicted, (int, float)) and predicted > 0:
                            tok_s = float(predicted)
                            metric = "mlx_timings"
                    piece = _openai_visible(event)
                    if not piece:
                        continue
                    if ttft is None:
                        ttft = time.perf_counter() - start
                    text_parts.append(piece)
    finally:
        peak = sampler.stop()
    elapsed = time.perf_counter() - start
    text = "".join(text_parts)
    if completion_tokens is None:
        completion_tokens = max(1, (len(text) + 3) // 4) if text else 0
    if tok_s is None:
        tok_s = _tok_s_wall(completion_tokens, elapsed, ttft)
        if tok_s is not None:
            metric = "wall"
    return {
        "ttft_ms": None if ttft is None else ttft * 1000.0,
        "elapsed_s": elapsed,
        "tok_s": tok_s,
        "completion_tokens": completion_tokens,
        "prompt_tokens": prompt_tokens,
        "peak_rss_mb": peak,
        "chars": len(text),
        "metric": metric,
        "text": text,
    }


def stream_ollama(
    *,
    model: str,
    prompt: str,
    max_tokens: int,
    timeout: float,
) -> dict[str, Any]:
    body: dict[str, Any] = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "stream": True,
        "think": False,
        "keep_alive": "10m",
        "options": {
            "temperature": 0,
            "num_predict": max_tokens,
            "num_ctx": 32768,
        },
    }
    sampler = RssSampler(_ollama_pids)
    url = TARGETS["ollama"]["base"].rstrip("/") + "/api/chat"
    start = time.perf_counter()
    ttft: float | None = None
    text_parts: list[str] = []
    completion_tokens: int | None = None
    prompt_tokens: int | None = None
    tok_s: float | None = None
    metric: str | None = None
    sampler.start()
    try:
        with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
            with client.stream("POST", url, json=body) as resp:
                resp.raise_for_status()
                for line in resp.iter_lines():
                    event = _ndjson(line)
                    if event is None:
                        continue
                    if event.get("done"):
                        prompt_tokens = event.get("prompt_eval_count", prompt_tokens)
                        completion_tokens = event.get("eval_count", completion_tokens)
                        native = _tok_s_from_ns(
                            event.get("eval_count"), event.get("eval_duration")
                        )
                        if native is not None:
                            tok_s = native
                            metric = "ollama_eval"
                    piece = _ollama_visible(event)
                    if not piece:
                        continue
                    if ttft is None:
                        ttft = time.perf_counter() - start
                    text_parts.append(piece)
    finally:
        peak = sampler.stop()
    elapsed = time.perf_counter() - start
    text = "".join(text_parts)
    if completion_tokens is None:
        completion_tokens = max(1, (len(text) + 3) // 4) if text else 0
    if tok_s is None:
        tok_s = _tok_s_wall(completion_tokens, elapsed, ttft)
        if tok_s is not None:
            metric = "wall"
    return {
        "ttft_ms": None if ttft is None else ttft * 1000.0,
        "elapsed_s": elapsed,
        "tok_s": tok_s,
        "completion_tokens": completion_tokens,
        "prompt_tokens": prompt_tokens,
        "peak_rss_mb": peak,
        "chars": len(text),
        "metric": metric,
        "text": text,
    }


def complete_openai(
    *, port: int, model: str, prompt: str, max_tokens: int, timeout: float,
    temperature: float = 0.0,
) -> str:
    return complete_openai_full(
        port=port, model=model, prompt=prompt, max_tokens=max_tokens,
        timeout=timeout, temperature=temperature,
    )["text"]


def complete_openai_full(
    *, port: int, model: str, prompt: str, max_tokens: int, timeout: float,
    temperature: float = 0.0,
) -> dict[str, Any]:
    """Like complete_openai but also returns wall time and token usage."""
    body = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "max_tokens": max_tokens,
        "temperature": temperature,
        "stream": False,
        "chat_template_kwargs": (
            {"enable_thinking": False, "thinking_budget": 0}
            if "Seed-OSS" in model else
            {"enable_thinking": False}
        ),
        **request_sampling(),
    }
    if "mistral-small-4" in model:
        # The request's chat_template_kwargs replaces the server's
        # --chat-template-kwargs, so the reasoning_effort pin must ride along
        # or Mistral falls back to its template default.
        body["chat_template_kwargs"] = {"enable_thinking": False, "reasoning_effort": "none"}
    url = f"http://127.0.0.1:{port}/v1/chat/completions"
    start = time.perf_counter()
    with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
        resp = client.post(url, json=body)
        resp.raise_for_status()
        # some quant builds emit invalid UTF-8 bytes mid-stream; don't crash on them
        data = json.loads(resp.content.decode("utf-8", errors="replace"))
    elapsed = time.perf_counter() - start
    choices = data.get("choices") or []
    message = choices[0].get("message") if choices else {}
    content = (message or {}).get("content")
    usage = data.get("usage") or {}
    served = data.get("model")
    if served and served != model:
        # mlx-lm ignores the request's model field; a mismatch means a stale
        # server on this port is serving different weights than we asked for.
        print(f"WARNING: requested model {model!r} but server answered as "
              f"{served!r} — results may be misattributed", file=sys.stderr)
    return {
        "text": content if isinstance(content, str) else "",
        "elapsed_s": elapsed,
        "completion_tokens": usage.get("completion_tokens"),
        "prompt_tokens": usage.get("prompt_tokens"),
        "served_model": served,
    }


def complete_ollama(
    *, model: str, prompt: str, max_tokens: int, timeout: float,
    temperature: float = 0.0,
) -> str:
    body = {
        "model": model,
        "messages": [{"role": "user", "content": prompt}],
        "stream": False,
        "think": False,
        "options": {"temperature": temperature, "num_predict": max_tokens},
    }
    url = TARGETS["ollama"]["base"].rstrip("/") + "/api/chat"
    with httpx.Client(timeout=httpx.Timeout(timeout, connect=5.0)) as client:
        resp = client.post(url, json=body)
        resp.raise_for_status()
        data = resp.json()
    message = data.get("message") or {}
    content = message.get("content")
    return content if isinstance(content, str) else ""


def run_quality(name: str, timeout: float, trials: int = 3) -> dict[str, Any]:
    cfg = TARGETS[name]
    if not _port_up(cfg["port"]):
        raise SystemExit(f"{name} is not up on :{cfg['port']}")
    results: dict[str, list[bool]] = {}
    replies: dict[str, list[str]] = {}
    for task, checker in QUALITY_TASKS:
        outcomes: list[bool] = []
        snippets: list[str] = []
        for i in range(trials):
            temp = 0.0 if i == 0 else 0.7
            if cfg["kind"] == "ollama":
                reply = complete_ollama(
                    model=cfg["model"], prompt=QUALITY_PROMPTS[task],
                    max_tokens=QUALITY_MAX_TOKENS, timeout=timeout,
                    temperature=temp,
                )
            else:
                reply = complete_openai(
                    port=cfg["port"], model=cfg["model"],
                    prompt=QUALITY_PROMPTS[task],
                    max_tokens=QUALITY_MAX_TOKENS, timeout=timeout,
                    temperature=temp,
                )
            ok = bool(checker(reply))
            outcomes.append(ok)
            snippets.append(reply.strip()[:120])
            print(f"{name} quality {task} t{i}: {'pass' if ok else 'FAIL'}", flush=True)
        results[task] = outcomes
        replies[task] = snippets
    passed = sum(ok for outs in results.values() for ok in outs)
    total = trials * len(QUALITY_TASKS)
    return {
        "target": name,
        "model": cfg["model"],
        "case": "quality",
        "trials": trials,
        "passed": passed,
        "total": total,
        "score": passed / total if total else 0,
        "results": results,
        "replies": replies,
    }


def run_once(
    name: str, prompt: str, max_tokens: int, timeout: float, thinking: bool
) -> dict[str, Any]:
    cfg = TARGETS[name]
    if cfg["kind"] == "ollama":
        return stream_ollama(
            model=cfg["model"],
            prompt=prompt,
            max_tokens=max_tokens,
            timeout=timeout,
        )
    return stream_mlx(
        model=cfg["model"],
        prompt=prompt,
        max_tokens=max_tokens,
        timeout=timeout,
        port=cfg["port"],
        thinking=thinking,
    )


def run_target(
    name: str, cases: list[str], trials: int, timeout: float, thinking: bool
) -> list[dict[str, Any]]:
    cfg = TARGETS[name]
    if not _port_up(cfg["port"]):
        raise SystemExit(f"{name} is not up on :{cfg['port']}")
    if _port_up(cfg["other"]):
        print(
            f"warning: the other server is also listening on :{cfg['other']}; "
            "GPU results will be contaminated. Stop it for a fair run.",
            flush=True,
        )
    print(f"warmup {name}...", flush=True)
    run_once(name, "Reply with the single word ping.", 8, timeout, thinking)
    rows: list[dict[str, Any]] = []
    for case in cases:
        spec = CASES[case]
        runs = []
        for i in range(trials):
            print(f"{name} {case} {i + 1}/{trials}...", flush=True)
            try:
                runs.append(
                    run_once(name, spec["prompt"], spec["max_tokens"], timeout, thinking)
                )
            except httpx.HTTPStatusError as e:
                # Server hard-caps the context (llama.cpp clamps to
                # n_ctx_train) and rejects the prompt with a 400: record the
                # case as skipped, don't abort. (.text is unreadable on
                # streaming responses, so match on the status code alone.)
                if e.response.status_code == 400:
                    print(f"  {name} {case}: skipped (server rejected prompt, 400)", flush=True)
                    runs = []
                    break
                raise
        ttfts = [r["ttft_ms"] for r in runs if r["ttft_ms"] is not None]
        toks = [r["tok_s"] for r in runs if r["tok_s"] is not None]
        rss = [r["peak_rss_mb"] for r in runs if r["peak_rss_mb"] is not None]
        outs = [r["completion_tokens"] for r in runs if r["completion_tokens"]]
        prompt_n = next((r["prompt_tokens"] for r in runs if r["prompt_tokens"]), None)
        metric = next((r["metric"] for r in runs if r.get("metric")), None)
        if case == "decode":
            sample = next((r.get("text") for r in reversed(runs) if r.get("text")), None)
            if sample:
                outdir = Path(__file__).resolve().parent.parent / "results" / "speed-texts"
                outdir.mkdir(parents=True, exist_ok=True)
                (outdir / f"{name}.txt").write_text(sample)
        rows.append(
            {
                "target": name,
                "model": cfg["model"],
                "case": case,
                "trials": trials,
                "prompt_tokens": prompt_n,
                "out_tok": int(_median([float(x) for x in outs])) if outs else None,
                "ttft_ms": _median(ttfts),
                "tok_s": _median(toks),
                "peak_rss_mb": _median(rss),
                "metric": metric,
                "thinking": thinking,
                "runs": runs,
            }
        )
    return rows


def _fmt(v: float | None, digits: int = 1) -> str:
    return "—" if v is None else f"{v:.{digits}f}"


def print_table(rows: list[dict[str, Any]]) -> None:
    print()
    print(
        f"{'target':<8} {'case':<8} {'think':>5} {'ttft_ms':>10} {'tok/s':>8} "
        f"{'peak_rss_mb':>12} {'prompt_tok':>11} {'out_tok':>8} {'metric':<12}"
    )
    for row in rows:
        print(
            f"{row['target']:<8} {row['case']:<8} "
            f"{str(row.get('thinking')):>5} "
            f"{_fmt(row['ttft_ms']):>10} {_fmt(row['tok_s']):>8} "
            f"{_fmt(row['peak_rss_mb']):>12} "
            f"{'—' if row['prompt_tokens'] is None else str(row['prompt_tokens']):>11} "
            f"{'—' if row['out_tok'] is None else str(row['out_tok']):>8} "
            f"{row.get('metric') or '—':<12}"
        )


def print_quality(rows: list[dict[str, Any]]) -> None:
    print()
    print(f"{'target':<8} {'score':>10}  details")
    for row in rows:
        marks = " ".join(
            (f"{k}:{sum(v)}/{len(v)}" if isinstance(v, list) else f"{k}:{'✓' if v else '✗'}")
            for k, v in row["results"].items()
        )
        print(f"{row['target']:<8} {row['passed']}/{row['total']:<8}  {marks}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Bench MLX vs Ollama vs Ornith (local)")
    parser.add_argument("--target", choices=tuple(TARGETS) + ("both",), default="both")
    parser.add_argument("--case", choices=("decode", "prefill", "quality", "both"), default="both")
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--thinking", action="store_true", help="enable thinking on OpenAI targets")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    if args.case == "quality":
        names = ["mlx", "ollama"] if args.target == "both" else [args.target]
        rows = [run_quality(name, args.timeout, args.trials) for name in names]
        if args.json:
            print(json.dumps(rows, indent=2))
        else:
            print_quality(rows)
        return
    cases = ["decode", "prefill"] if args.case == "both" else [args.case]
    names = ["mlx", "ollama"] if args.target == "both" else [args.target]
    rows: list[dict[str, Any]] = []
    for name in names:
        rows.extend(run_target(name, cases, args.trials, args.timeout, args.thinking))
    if args.json:
        print(json.dumps(rows, indent=2))
    else:
        print_table(rows)


if __name__ == "__main__":
    main()

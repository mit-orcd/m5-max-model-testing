#!/usr/bin/env bash
# Linux main model via vLLM (CUDA) — the vLLM side of the llama.cpp-vs-vLLM
# comparison. Override the target with TARGET=<name>.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
TARGET="${TARGET:-gptoss-vllm}"

PORT="$("$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" field "$TARGET" port)"
kill_port "$PORT"

exec "$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" exec "$TARGET"

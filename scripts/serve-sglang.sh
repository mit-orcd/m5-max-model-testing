#!/usr/bin/env bash
# Linux Qwen3.8-27B via SGLang — the SGLang side of Ollama vs vLLM vs SGLang.
# Isolated interpreter: .venv-sglang (see scripts/linux-sglang-setup.sh).
# Override the target with TARGET=<name>.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
TARGET="${TARGET:-qwen27-sglang}"

PORT="$("$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" field "$TARGET" port)"
kill_port "$PORT"

exec "$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" exec "$TARGET"

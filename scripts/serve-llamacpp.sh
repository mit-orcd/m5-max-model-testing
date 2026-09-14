#!/usr/bin/env bash
# Linux main model for Hermes: gpt-oss-20b via llama.cpp (Vulkan) on :8080.
# Override the target with TARGET=<name>; defaults to the gptoss target.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
TARGET="${TARGET:-gptoss}"

PORT="$("$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" field "$TARGET" port)"
kill_port "$PORT"

exec "$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" exec "$TARGET"

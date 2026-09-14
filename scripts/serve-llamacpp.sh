#!/usr/bin/env bash
# Linux main model for Hermes: gpt-oss-20b via llama.cpp (Vulkan) on :8080.
# Override the target with TARGET=<name>; defaults to the gptoss target.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"
TARGET="${TARGET:-gptoss}"

# Live stack serves on :8080 (gateway's main backend); bench targets keep
# their TARGETS port. PORT is honoured by serve.py.
export PORT="${PORT:-8080}"
kill_port "$PORT"

exec "$ROOT/.venv/bin/python" "$ROOT/scripts/serve.py" exec "$TARGET"

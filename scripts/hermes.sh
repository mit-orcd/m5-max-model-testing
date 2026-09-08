#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HERMES="${HERMES:-$HOME/.local/bin/hermes}"
# shellcheck source=scripts/_ports.sh
source "$ROOT/scripts/_ports.sh"

if [[ ! -x "$HERMES" ]]; then
  echo "hermes not found at $HERMES" >&2
  exit 1
fi

if ! curl -fsS --max-time 2 http://127.0.0.1:4000/v1/models >/dev/null 2>&1; then
          echo "Starting MLX stack (gpt-oss + 8B + gateway)..."
  "$ROOT/scripts/serve-all.sh"
fi

exec "$HERMES" "$@"

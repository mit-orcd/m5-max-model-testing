#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CONFIG="${HERMES_HOME:-$HOME/.hermes}/config.yaml"
PYTHON="$ROOT/.venv/bin/python"

echo "== Hermes ($CONFIG)"
"$PYTHON" - <<'PY' "$CONFIG"
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    yaml = None

path = Path(sys.argv[1])
raw = path.read_text()
if yaml is None:
    print(raw)
    raise SystemExit(0)

cfg = yaml.safe_load(raw)
model = cfg.get("model") or {}
mlx = (cfg.get("providers") or {}).get("mlx") or {}
aux = cfg.get("auxiliary") or {}
print(f"provider:        {model.get('provider')}")
print(f"model:           {model.get('default')}")
print(f"base_url:        {model.get('base_url')}")
print(f"context_length:  {model.get('context_length')}")
print(f"api_mode:        {model.get('api_mode')}")
thinking = (
    ((mlx.get("extra_body") or {}).get("chat_template_kwargs") or {}).get("enable_thinking")
)
print(f"thinking:        {thinking}")
print(f"mlx context:     {((mlx.get('models') or {}).get(model.get('default') or '', {}) or {}).get('context_length')}")
for task in ("title_generation", "compression"):
    block = aux.get(task) or {}
    print(f"aux.{task}:  provider={block.get('provider')} model={block.get('model')}")
PY

echo
echo "== Serve scripts"
awk '/^HOST=|^PORT=|^MODEL=|--model |--host |--port |--max-tokens |--chat-template-args /' "$ROOT/scripts/serve-mlx.sh"
echo "-- fast --"
awk '/^HOST=|^PORT=|^MODEL=|--model |--host |--port |--max-tokens |--chat-template-args /' "$ROOT/scripts/serve-fast.sh"
echo "-- gateway --"
awk '/^HOST=|^PORT=|--host |--port |--main |--fast /' "$ROOT/scripts/serve-gateway.sh"

echo
echo "== Live"
for name_url in "gpt-oss|http://127.0.0.1:8080/v1/models" "8B|http://127.0.0.1:8081/v1/models" "gateway|http://127.0.0.1:4000/v1/models"; do
  name="${name_url%%|*}"
  url="${name_url##*|}"
  if curl -fsS --max-time 2 "$url" >/tmp/mlx-models.json 2>/dev/null; then
    "$PYTHON" -c "import json,sys; d=json.load(open('/tmp/mlx-models.json')); print(sys.argv[1]+':', 'up', ', '.join(m.get('id','?') for m in d.get('data',[])))" "$name"
  else
    echo "$name: down"
  fi
done

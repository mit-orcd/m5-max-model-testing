json_get() {
  local key="$1"
  KEY="$key" python3 - <<'PY'
import sys, json, os
data = json.load(sys.stdin)
print(data.get(os.getenv('KEY', ''), ''))
PY
}
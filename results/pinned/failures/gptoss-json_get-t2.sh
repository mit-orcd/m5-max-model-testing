json_get() {
  local key="$1"
  python3 - <<'PY' "$key"
import sys, json
key = sys.argv[1]
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(1)
value = data.get(key)
if value is not None:
    if isinstance(value, str):
        print(value)
    else:
        print(value)
PY
}
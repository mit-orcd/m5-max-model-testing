json_get() {
  local key="$1"
  python3 - <<'PY' "$key"
import sys, json
key = sys.argv[1]
try:
    data = json.load(sys.stdin)
except json.JSONDecodeError:
    sys.exit(1)
value = data.get(key)
if value is None:
    sys.exit(1)
# Print string values as-is, otherwise JSON-encode them
if isinstance(value, str):
    print(value)
else:
    print(json.dumps(value))
PY
}
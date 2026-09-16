json_get() {
  local key="$1"
  python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
data = json.load(sys.stdin)
value = data.get(key)
if value is not None:
    print(value)
PY
}
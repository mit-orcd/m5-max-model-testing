json_get() {
  local key="$1"
  python3 - <<'PY' "$key"
import sys, json
key = sys.argv[1]
data = json.load(sys.stdin)
if key in data:
    print(data[key])
PY
}
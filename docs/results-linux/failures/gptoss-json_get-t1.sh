json_get() {
  local key="$1"
  python3 - <<'PY' "$key"
import sys, json
data = json.load(sys.stdin)
key = sys.argv[1]
print(data.get(key, ""))
PY
}
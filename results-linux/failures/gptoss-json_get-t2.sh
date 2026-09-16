json_get() {
    local key="$1"
    python3 - <<'PY' "$key"
import sys, json
data = json.load(sys.stdin)
key = sys.argv[1]
if key not in data:
    sys.exit(1)
print(data[key])
PY
}
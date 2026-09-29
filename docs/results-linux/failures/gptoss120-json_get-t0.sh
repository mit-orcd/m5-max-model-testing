json_get() {
    local key=$1
    python3 - <<'PY' "$key"
import sys, json, os

key = sys.argv[1]
data = json.load(sys.stdin)

value = data.get(key)
if isinstance(value, str):
    sys.stdout.write(value)
elif value is not None:
    sys.stdout.write(json.dumps(value))
# else: key not present → output nothing
PY
}
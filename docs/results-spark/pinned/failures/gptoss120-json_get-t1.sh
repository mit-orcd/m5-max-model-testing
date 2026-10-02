json_get() {
    local key=$1
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
data = json.load(sys.stdin)
value = data.get(key)
if isinstance(value, str):
    print(value)
elif value is not None:
    print(json.dumps(value))
PY
}
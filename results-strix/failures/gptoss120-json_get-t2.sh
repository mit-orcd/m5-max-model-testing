json_get() {
    local key="$1"
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(1)

value = data.get(key)
if isinstance(value, (dict, list)):
    print(json.dumps(value))
elif value is not None:
    print(value)
# if value is None (key missing) print nothing
PY
}
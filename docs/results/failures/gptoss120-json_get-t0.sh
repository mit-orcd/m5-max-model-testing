json_get() {
    local key=$1
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
data = json.load(sys.stdin)
value = data.get(key, None)

if isinstance(value, str):
    sys.stdout.write(value)
elif value is None:
    sys.stdout.write('null')
else:
    sys.stdout.write(json.dumps(value))
PY
}
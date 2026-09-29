json_get() {
    local key=$1
    python3 - "$key" <<'PY'
import sys, json

key = sys.argv[1]

try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(1)

value = data.get(key)

if isinstance(value, str):
    sys.stdout.write(value)
elif value is not None:
    sys.stdout.write(str(value))
# If the key is missing, output nothing
PY
}
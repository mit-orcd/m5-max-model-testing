json_get() {
    local key=$1
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
data = json.load(sys.stdin)
value = data.get(key, "")
# Ensure we output a plain string (no surrounding quotes)
if isinstance(value, str):
    sys.stdout.write(value)
else:
    sys.stdout.write(str(value))
PY
}
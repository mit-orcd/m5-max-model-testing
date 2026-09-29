json_get() {
    local key="$1"
    python3 - <<'PY' "$key"
import sys, json
key = sys.argv[1]
obj = json.load(sys.stdin)
if key not in obj:
    sys.exit(1)
val = obj[key]
if isinstance(val, str):
    print(val)
else:
    print(json.dumps(val))
PY
}
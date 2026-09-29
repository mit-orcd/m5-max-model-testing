json_get() {
    local key=$1
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
try:
    obj = json.load(sys.stdin)
except Exception:
    sys.exit(1)

val = obj.get(key)
if isinstance(val, str):
    print(val)
elif val is not None:
    # print non‑string values as JSON
    print(json.dumps(val))
PY
}
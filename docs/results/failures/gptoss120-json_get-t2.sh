json_get() {
    local key=$1
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
data = json.load(sys.stdin)
val = data.get(key)
if isinstance(val, (dict, list)):
    print(json.dumps(val))
elif val is not None:
    print(val)
PY
}
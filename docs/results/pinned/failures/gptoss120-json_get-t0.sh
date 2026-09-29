json_get() {
    local key=$1
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
obj = json.load(sys.stdin)
val = obj.get(key)
if isinstance(val, str):
    print(val)
else:
    print(json.dumps(val))
PY
}
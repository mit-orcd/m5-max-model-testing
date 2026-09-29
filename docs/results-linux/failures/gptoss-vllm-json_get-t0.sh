json_get() {
    local key="$1"
    python3 - <<'PY' "$key"
import sys, json
key = sys.argv[1]
try:
    data = json.load(sys.stdin)
    val = data.get(key)
    if isinstance(val, str):
        print(val)
except Exception:
    pass
PY
}
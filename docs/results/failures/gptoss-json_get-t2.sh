json_get() {
    local key="$1"
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
data = json.load(sys.stdin)
print(data.get(key, ""))
PY
}
json_get() {
    local key="$1"
    python3 -c '
import sys, json
data = json.load(sys.stdin)
if isinstance(data, dict) and "$key" in data:
    val = data["$key"]
    if isinstance(val, str):
        print(val)
    else:
        print(json.dumps(val))
'
}
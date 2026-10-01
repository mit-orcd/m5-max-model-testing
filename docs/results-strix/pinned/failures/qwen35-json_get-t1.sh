json_get() {
    local key="$1"
    python3 -c "
import json, sys
data = json.load(sys.stdin)
if key in data and isinstance(data[key], str):
    print(data[key])
"
}
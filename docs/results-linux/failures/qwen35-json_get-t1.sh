json_get() {
    local key="$1"
    python3 -c "
import json
import sys
try:
    data = json.load(sys.stdin)
    if isinstance(data, dict) and key in data and isinstance(data[key], str):
        print(data[key])
except json.JSONDecodeError:
    sys.exit(1)
"
}
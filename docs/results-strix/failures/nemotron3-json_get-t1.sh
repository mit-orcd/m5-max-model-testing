json_get() {
    python3 - <<PY
import sys
import json
data = json.load(sys.stdin)
print(data.get("$1"))
PY
}
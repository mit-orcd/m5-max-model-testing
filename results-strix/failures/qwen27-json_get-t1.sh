json_get() {
  local key="$1"
  python3 -c '
import sys, json
data = json.load(sys.stdin)
if not isinstance(data, dict):
    sys.exit(1)
if "$key" not in data:
    sys.exit(1)
val = data["$key"]
if not isinstance(val, str):
    sys.exit(1)
print(val)
'
}
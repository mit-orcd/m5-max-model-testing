json_get() {
  python3 -c '
import json
import sys
data = json.load(sys.stdin)
print(json.dumps(data[sys.argv[1]]))
' "$1"
}
json_get() {
    local key="$1"
    python3 - "$key" <<'PY'
import sys, json
key = sys.argv[1]
try:
    data = json.load(sys.stdin)
except json.JSONDecodeError as e:
    sys.exit(f"Invalid JSON: {e}")
value = data.get(key, None)
if isinstance(value, (dict, list)):
    # output JSON representation for complex types
    print(json.dumps(value, ensure_ascii=False))
elif isinstance(value, str):
    # raw string without surrounding quotes
    print(value)
elif value is not None:
    # numbers, booleans, null -> print as JSON literal
    print(json.dumps(value, ensure_ascii=False))
# if key not present, nothing is printed
PY
}
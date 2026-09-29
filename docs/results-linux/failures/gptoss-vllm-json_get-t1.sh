json_get() {
  local key="$1"
  python3 - "$key" <<'PY'
import sys, json
try:
    data = json.load(sys.stdin)
    print(data[sys.argv[1]])
except Exception:
    sys.exit(1)
PY
}
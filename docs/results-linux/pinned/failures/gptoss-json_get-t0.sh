json_get() {
  local key="$1"
  python3 - <<'PY' "$key"
import sys, json, argparse
parser = argparse.ArgumentParser()
parser.add_argument("key")
args = parser.parse_args()
data = json.load(sys.stdin)
print(data.get(args.key, ""))
PY
}
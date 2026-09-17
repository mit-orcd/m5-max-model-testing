json_get() {
  python3 -c 'import json, sys; print(json.loads(sys.stdin.read())[sys.argv[1]])' -- "$1"
}
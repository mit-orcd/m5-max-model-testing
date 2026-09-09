json_get() {
    python3 -c "import json, sys; print(json.load(sys.stdin)[sys.argv[1]] if sys.argv[1] in json.load(sys.stdin) else '')" "$1"
}
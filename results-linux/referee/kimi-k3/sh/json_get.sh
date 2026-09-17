json_get() {
    python3 -c "import json,sys; print(json.load(sys.stdin)[sys.argv[1]])" "$1"
}

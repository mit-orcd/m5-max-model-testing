json_get() {
    python -c "import json, sys; data = json.load(sys.stdin); print(data.get('$1', ''))" "$1"
}
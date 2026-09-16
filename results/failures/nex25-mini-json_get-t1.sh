json_get() {
    python3 -c 'import json, sys; d = json.load(sys.stdin); k = sys.argv[1]; v = d.get(k); 
    if not isinstance(v, str):
        sys.exit(1)
    print(v)' "$1"
}
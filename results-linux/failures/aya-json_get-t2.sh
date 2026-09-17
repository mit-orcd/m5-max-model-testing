json_get() {
    local key="$1"
    local json="$(< /dev/stdin python3 -c 'import json,sys;print(json.dumps(json.load(sys.stdin.read())))')"
    echo "$json" | python3 -c "import json,sys;print(json.get(json.loads(sys.stdin.read()),'$key'))"
}
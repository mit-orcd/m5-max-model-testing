json_get() {
    local key="$1"
    local json=$(cat - | python3 -c 'import json,sys;print(json.load(sys.stdin))')
    echo "${json[$key]}"
}
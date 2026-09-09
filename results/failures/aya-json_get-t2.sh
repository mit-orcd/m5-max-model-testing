json_get() {
    local key="$1"
    local json="$(< /dev/stdin)"

    # Remove leading and trailing whitespace
    json="${json##*[^ ]}"
    json="${json%*[^ ]}"

    # Find the value associated with the given key
    local value=$(echo "$json" | python3 -c "import json, sys; print(json.loads(sys.stdin.read())['$key'])" )

    # Print the value
    echo "$value"
}
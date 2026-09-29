retry3() {
    local attempt=1
    local max=3
    local status

    while [ "$attempt" -le "$max" ]; do
        "$@"
        status=$?
        if [ "$status" -eq 0 ]; then
            exit 0
        fi
        attempt=$((attempt + 1))
    done

    exit "$status"
}
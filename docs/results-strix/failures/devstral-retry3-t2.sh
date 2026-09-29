retry3() {
    local attempts=0
    local max_attempts=3
    while [ $attempts -lt $max_attempts ]; do
        if "$@"; then
            return 0
        fi
        attempts=$((attempts + 1))
    done
    return $?
}
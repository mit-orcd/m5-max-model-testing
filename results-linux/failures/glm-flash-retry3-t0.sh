retry3() {
    local attempts=0
    local max_attempts=3
    local exit_code=0

    while [ $attempts -lt $max_attempts ]; do
        if "$@"; then
            return 0
        fi
        exit_code=$?
        attempts=$((attempts + 1))
        if [ $attempts -lt $max_attempts ]; then
            sleep 1
        fi
    done

    return $exit_code
}
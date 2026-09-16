retry3() {
    local attempts=0
    local max_attempts=3
    local exit_code=0

    while (( attempts++ < max_attempts )); do
        if "$@"; then
            return 0
        fi
        exit_code=$?
        if (( attempts < max_attempts )); then
            sleep 1
        fi
    done

    return $exit_code
}
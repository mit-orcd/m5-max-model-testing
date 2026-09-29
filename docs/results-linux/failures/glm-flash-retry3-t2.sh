retry3() {
    local attempts=0
    local max_attempts=3
    local exit_code=0
    local cmd=("$@")

    while (( attempts < max_attempts )); do
        if "${cmd[@]}"; then
            return 0
        fi
        exit_code=$?
        attempts=$((attempts + 1))
        if (( attempts < max_attempts )); then
            sleep 1
        fi
    done
    return $exit_code
}
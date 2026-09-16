retry3() {
    local attempts=3
    local exit_code=1
    while (( attempts > 0 )); do
        if "$@"; then
            return 0
        fi
        exit_code=$?
        (( attempts-- ))
    done
    return $exit_code
}
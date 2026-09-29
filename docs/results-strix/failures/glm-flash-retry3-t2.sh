retry3() {
    local retries=3
    local attempt=1
    local exit_code=1

    while [[ $attempt -le $retries ]]; do
        if "$@"; then
            return 0
        fi
        exit_code=$?
        ((attempt++))
    done

    return $exit_code
}
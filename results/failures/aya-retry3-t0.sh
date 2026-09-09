retry3() {
    local attempts=0
    local max_attempts=3
    local cmd="$@"
    local exit_code=0

    until [[ $attempts -ge $max_attempts ]]; do
        attempts=$((attempts + 1))
        eval "$cmd"
        exit_code=$?
        if [[ $exit_code -eq 0 ]]; then
            break
        fi
    done

    exit $exit_code
}
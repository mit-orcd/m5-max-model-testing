retry3() {
    local attempts=0
    local exit_code=0

    until [[ $attempts -ge 3 ]]; do
        attempts=$((attempts + 1))
        eval "$@"
        exit_code=$?
        if [[ $exit_code -eq 0 ]]; then
            break
        fi
    done

    exit $exit_code
}
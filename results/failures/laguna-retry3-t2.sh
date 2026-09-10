retry3() {
    local attempts=3
    local exit_code=0
    while [ $attempts -gt 0 ]; do
        if "$@"; then
            return 0
        fi
        exit_code=$?
        attempts=$((attempts - 1))
        if [ $attempts -gt 0 ]; then
            sleep 1
        fi
    done
    return $exit_code
}
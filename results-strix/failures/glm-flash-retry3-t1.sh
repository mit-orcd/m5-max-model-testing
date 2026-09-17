retry3() {
    local attempts=3
    local attempt=1
    local exit_code=0

    while [ $attempt -le $attempts ]; do
        if "$@"; then
            return 0
        fi
        exit_code=$?
        if [ $attempt -lt $attempts ]; then
            sleep 1
        fi
        attempt=$((attempt + 1))
    done

    return $exit_code
}
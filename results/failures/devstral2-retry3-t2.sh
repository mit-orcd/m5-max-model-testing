retry3() {
    local attempts=0
    local max_attempts=3
    local exit_code=0

    while [ $attempts -lt $max_attempts ]; do
        "$@" && exit_code=0 && break
        exit_code=$?
        ((attempts++))
        [ $exit_code -ne 0 ] && sleep 1
    done

    exit $exit_code
}
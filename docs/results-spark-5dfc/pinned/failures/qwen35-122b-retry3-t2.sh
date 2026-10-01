retry3() {
    local cmd=("$@")
    local max_attempts=3
    local attempt=1

    while [ $attempt -le $max_attempts ]; do
        if "${cmd[@]}"; then
            return 0
        fi
        ((attempt++))
    done

    return $?
}
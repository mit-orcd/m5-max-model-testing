retry3() {
    local max_attempts=3
    local attempt=1
    local exit_code=0

    while (( attempt <= max_attempts )); do
        "$@"
        exit_code=$?
        if (( exit_code == 0 )); then
            exit 0
        fi
        (( attempt++ ))
    done

    exit "$exit_code"
}
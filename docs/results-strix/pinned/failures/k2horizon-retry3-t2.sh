retry3() {
    local attempt
    for attempt in 1 2 3; do
        if "$@"; then
            return 0
        fi
        if [ "$attempt" -eq 3 ]; then
            return $?
        fi
    done
}
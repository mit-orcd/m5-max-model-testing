retry3() {
    local attempts=0
    while [ $attempts -lt 3 ]; do
        "$@"
        if [ $? -eq 0 ]; then
            return 0
        fi
        attempts=$((attempts + 1))
    done
    return $?
}
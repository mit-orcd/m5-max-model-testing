retry3() {
    local n=0
    local exit_code=0
    while [ $n -lt 3 ]; do
        "$@" && return 0
        exit_code=$?
        n=$((n + 1))
    done
    return $exit_code
}
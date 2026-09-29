retry3() {
    local attempt=1
    while [ $attempt -le 3 ]; do
        "$@" && return 0
        attempt=$((attempt + 1))
    done
    return $?
}
retry3() {
    local i
    for i in 1 2 3; do
        "$@" && return 0
    done
    return 1
}

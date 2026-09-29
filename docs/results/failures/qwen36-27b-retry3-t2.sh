retry3() {
    local cmd=("$@")
    for i in 1 2 3; do
        if "${cmd[@]}"; then
            return 0
        fi
    done
    return $?
}
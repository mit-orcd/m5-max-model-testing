retry3() {
    local attempt=1
    while true; do
        "$@" && return 0
        if (( attempt == 3 )); then
            return $?
        fi
        ((attempt++))
    done
}
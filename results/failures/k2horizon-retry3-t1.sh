retry3() {
    local n=0
    while [ "$n" -lt 3 ]; do
        "$@"
        local rc=$?
        if [ "$rc" -eq 0 ]; then
            return 0
        fi
        n=$((n + 1))
    done
    return "$rc"
}
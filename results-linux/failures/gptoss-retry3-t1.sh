retry3() {
    local attempts=3
    local rc
    for ((i=1; i<=attempts; i++)); do
        "$@"
        rc=$?
        if [[ $rc -eq 0 ]]; then
            exit 0
        fi
    done
    exit $rc
}
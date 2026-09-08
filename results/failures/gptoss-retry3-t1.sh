retry3() {
    local cmd=("$@")
    local attempt=1
    local max=3
    local last_rc=1

    while [ "$attempt" -le "$max" ]; do
        "${cmd[@]}"
        last_rc=$?
        if [ "$last_rc" -eq 0 ]; then
            exit 0
        fi
        attempt=$((attempt + 1))
    done
    exit "$last_rc"
}
retry3() {
    local -i attempts=0
    local -i max_attempts=3
    local -i status=0

    until [[ $attempts -ge $max_attempts ]]; do
        $@ && status=0 || status=$?
        attempts=$((attempts + 1))
    done

    exit $status
}
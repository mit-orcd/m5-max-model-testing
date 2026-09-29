retry3() {
    local i=0
    local max_attempts=3
    local cmd="$@"
    local exit_code

    until [[ $i -ge $max_attempts ]]; do
        exit_code="$?"
        if [[ $exit_code -eq 0 ]]; then
            return 0
        fi
        i=$((i+1))
    done

    return $exit_code
}
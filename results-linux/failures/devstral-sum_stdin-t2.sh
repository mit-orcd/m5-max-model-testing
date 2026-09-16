sum_stdin() {
    awk '/^[[:space:]]*$/ { next } { sum += $0 } END { print sum }'
}
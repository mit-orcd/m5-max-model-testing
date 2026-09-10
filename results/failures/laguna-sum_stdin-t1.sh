sum_stdin() {
    awk 'NF { sum += $1 } END { print sum }'
}
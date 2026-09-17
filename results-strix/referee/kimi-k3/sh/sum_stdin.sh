sum_stdin() {
    awk 'NF { s += $1 } END { print s + 0 }'
}

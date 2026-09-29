sum_stdin() {
    awk '{ if ($0 ~ /^[+-]?[0-9]+$/) sum += $0 } END { print sum }'
}
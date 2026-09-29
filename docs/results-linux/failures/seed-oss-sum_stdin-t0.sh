sum_stdin() {
  awk 'NF { sum += $0 } END { print sum }'
}
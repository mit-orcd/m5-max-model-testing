total_size() {
    (cd "$1" && find . -type f -exec ls -l {} + | awk '{sum += $5} END {print sum}')
}
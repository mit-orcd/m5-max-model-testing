total_size() {
    (cd "$1" && find . -type f -exec ls -l {} + 2>/dev/null | awk '{sum+=$5} END {print sum}');
}
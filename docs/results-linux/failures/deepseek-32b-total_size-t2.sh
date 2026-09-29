total_size() {
    find "$1" -type f -print0 | xargs -0 stat -f %z 2>/dev/null | awk '{sum += $1} END {print sum}'
}
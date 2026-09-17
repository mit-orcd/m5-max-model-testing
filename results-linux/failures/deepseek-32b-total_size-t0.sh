total_size() {
    find "$1" -type f -print0 | xargs -0 stat -f %z | awk '{sum += $1} END {print sum}'
}
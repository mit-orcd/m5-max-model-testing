function total_size() {
    find "$1" -type f -exec du -ch "$1/{}" \; | awk '{s+=$1} END {print s}'
}
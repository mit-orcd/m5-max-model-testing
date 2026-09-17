total_size() {
    find "$1" -type f -exec du -ch {} + | awk '{s+=$1} END {print s}'
}
total_size() {
    find "$1" -type f -exec du -h --bytes-only {} + | awk '{s+=$1} END {print s}';
}
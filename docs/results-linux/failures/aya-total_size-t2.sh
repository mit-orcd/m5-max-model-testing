function total_size() {
    find "$1" -type f -exec du -chz {} + | awk '{s+=$1} END {print s}'
}
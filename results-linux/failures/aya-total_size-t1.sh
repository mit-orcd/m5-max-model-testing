total_size() {
    find "$1" -type f -exec stat -f%z {} + | awk '{s+=$1} END {print s}'
}
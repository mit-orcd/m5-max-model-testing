total_size() {
    local dir="$1"
    find "$dir" -type f -exec stat -f%z {} + 2>/dev/null | awk '{s+=$1} END {print s}'
}
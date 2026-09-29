largest_file() {
    find "$1" -type f -exec du -b {} + 2>/dev/null | sort -nr | head -1 | cut -f2
}
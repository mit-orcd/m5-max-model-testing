largest_file() {
    find "$1" -type f -exec du -b {} + | sort -nr | head -n1 | cut -f2
}
largest_file() {
    find "$1" -type f -exec du -b {} + | sort -nr | awk 'NR==1 {print $2}'
}
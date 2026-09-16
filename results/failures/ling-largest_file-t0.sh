largest_file() {
    find "$1" -type f -exec du -b {} + | sort -t $'\t' -k1,1nr | head -1 | cut -f2-
}
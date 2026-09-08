largest_file() {
    find "$1" -type f -printf '%s %p\n' | sort -n | tail -n 1 | awk '{print $2}'
}
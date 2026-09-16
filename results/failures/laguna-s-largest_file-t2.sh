largest_file() {
    find "$1" -type f -printf '%s %p\n' | sort -nr | head -n1 | cut -d' ' -f2-
}
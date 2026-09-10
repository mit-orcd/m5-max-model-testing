largest_file() {
    find "$1" -type f -printf "%s\t%p\n" | sort -nr | head -n1 | cut -f2-
}
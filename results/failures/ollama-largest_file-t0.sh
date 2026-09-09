largest_file() {
    find "$1" -type f -printf '%s\t%p\n' | sort -rn | head -1 | cut -f2-
}
largest_file() {
    find "$1" -type f -printf '%s %p\n' | sort -rn | head -n1 | cut -d' ' -f2-
}
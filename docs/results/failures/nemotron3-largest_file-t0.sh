largest_file() {
    find "$1" -type f -printf '%s %p\0' 2>/dev/null | sort -z -n | tail -z -n 1 | cut -z -d' ' -f2- | tr -d '\0'
}
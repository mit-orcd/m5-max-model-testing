largest_file() {
    local dir="$1"
    find "$dir" -type f -printf '%s\t%p\n' 2>/dev/null | sort -nr | head -n 1 | cut -f2-
}
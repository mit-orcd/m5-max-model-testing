largest_file() {
    local dir="$1"
    find "$dir" -type f -printf "%s\t%p\n" | sort -nr | head -n1 | cut -f2-
}
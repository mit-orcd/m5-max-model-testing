largest_file() {
    local dir="$1"
    find "$dir" -type f -printf '%s %p\n' | sort -rn | head -n 1 | cut -d' ' -f2-
}
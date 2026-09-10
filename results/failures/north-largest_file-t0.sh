largest_file() {
    local dir="$1"
    find "$dir" -type f -printf '%s %p\n' 2>/dev/null |
        sort -nr |
        head -n1 |
        sed 's/^[0-9]* //'
}
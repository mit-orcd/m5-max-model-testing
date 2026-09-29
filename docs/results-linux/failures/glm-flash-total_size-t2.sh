total_size() {
    local dir="${1:-.}"
    local total=0
    find "$dir" -type f -exec wc -c {} + 2>/dev/null | awk '{sum += $1} END {print sum}'
}
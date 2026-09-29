total_size() {
    local dir="$1"
    local total=0
    local file
    while IFS= read -r -d '' file; do
        local size
        size=$(stat -f%z "$file" 2>/dev/null) || continue
        total=$((total + size))
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%d\n' "$total"
}
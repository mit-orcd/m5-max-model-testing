total_size() {
    local dir=$1
    local total=0
    while IFS= read -r -d '' file; do
        size=$(stat -f%z "$file")
        total=$((total + size))
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%s\n' "$total"
}
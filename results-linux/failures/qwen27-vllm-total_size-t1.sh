total_size() {
    local dir="$1"
    local total=0
    local f
    while IFS= read -r -d '' f; do
        # Get size in bytes of the regular file
        local size
        size=$(stat -f %z "$f" 2>/dev/null) || continue
        total=$((total + size))
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%d\n' "$total"
}
total_size() {
    local dir="${1%/}"
    local total=0
    local size
    while IFS= read -r -d '' file; do
        size=$(stat -f '%z' -- "$file" 2>/dev/null) || continue
        if [ -n "$size" ]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%d' "$total"
}
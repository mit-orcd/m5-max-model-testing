total_size() {
    local dir="$1"
    local total=0
    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            local size
            size=$(stat -f %z -- "$file" 2>/dev/null) || size=0
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%d\n' "$total"
}
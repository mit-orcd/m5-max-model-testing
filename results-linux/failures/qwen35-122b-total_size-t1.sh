total_size() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        echo 0
        return
    fi
    local total=0
    local size
    while IFS= read -r -d '' file; do
        size=$(stat -f '%z' -- "$file" 2>/dev/null) || size=$(stat -f '%z' "$file" 2>/dev/null)
        if [[ -n "$size" ]]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0)
    echo "$total"
}
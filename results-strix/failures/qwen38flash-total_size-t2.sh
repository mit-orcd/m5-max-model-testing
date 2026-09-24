total_size() {
    local dir="$1"
    local total=0
    local file
    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            local size
            size=$(stat -f '%z' "$file" 2>/dev/null)
            if [ -n "$size" ]; then
                total=$((total + size))
            fi
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    echo "$total"
}
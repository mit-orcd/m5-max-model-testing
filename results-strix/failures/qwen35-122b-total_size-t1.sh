total_size() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo 0
        return
    fi
    local sum=0
    while IFS= read -r -d '' file; do
        if [[ -f "$file" && ! -L "$file" ]]; then
            local size
            size=$(stat -f '%z' -- "$file" 2>/dev/null || stat -f '%z' "$file" 2>/dev/null)
            if [[ -n "$size" && "$size" =~ ^[0-9]+$ ]]; then
                sum=$((sum + size))
            fi
        fi
    done < <(find "$dir" -print0 2>/dev/null)
    echo "$sum"
}
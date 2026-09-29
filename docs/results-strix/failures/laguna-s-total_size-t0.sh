total_size() {
    local dir="$1"
    if [ ! -d "$dir" ]; then
        printf '0\n'
        return
    fi
    local total=0
    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            size=$(stat -f '%z' "$file" 2>/dev/null) || size=0
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0)
    printf '%d\n' "$total"
}
total_size() {
    local dir="$1"
    local total=0
    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            size=$(stat -f '%z' "$file")
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0)
    printf '%d\n' "$total"
}
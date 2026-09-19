total_size() {
    local dir="$1"
    local total=0
    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            total=$((total + $(stat -f%z "$file")))
        fi
    done < <(find "$dir" -type f -print0)
    printf "%d" "$total"
}
total_size() {
    local total=0
    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            size=$(stat -f%z "$file" 2>/dev/null) || size=0
            total=$((total + size))
        fi
    done < <(find "$1" -type f -print0 2>/dev/null)
    echo "$total"
}
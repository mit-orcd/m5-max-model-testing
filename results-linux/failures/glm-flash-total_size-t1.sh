total_size() {
    local dir="$1"
    local total=0
    
    while IFS= read -r -d '' entry; do
        if [ -f "$entry" ]; then
            total=$((total + $(stat -f%z "$entry" 2>/dev/null)))
        fi
    done < <(find "$dir" -type f -print0)
    
    printf "%d" "$total"
}
total_size() {
    local dir="$1"
    local total=0
    local file
    
    # Use find with -type f to get only regular files, -print0 for null-terminated output
    # Use stat -f %z to get file size in bytes (BSD stat)
    while IFS= read -r -d '' file; do
        size=$(stat -f %z -- "$file" 2>/dev/null)
        if [[ -n "$size" && "$size" =~ ^[0-9]+$ ]]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    echo "$total"
}
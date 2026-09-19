total_size() {
    local dir="$1"
    local total=0
    local size
    
    # Use find with -print0 to handle hostile filenames
    # Use stat -f %z to get size on BSD/macOS
    while IFS= read -r -d '' file; do
        # stat -f %z gets size in bytes for the file
        size=$(stat -f %z -- "$file" 2>/dev/null) || continue
        total=$((total + size))
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    echo "$total"
}
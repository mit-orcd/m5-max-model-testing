total_size() {
    local dir="$1"
    local total=0

    # Use find with -print0 to handle hostile filenames
    # Use stat on macOS (BSD) to get file size
    # stat -f %z gives size in bytes on macOS
    
    while IFS= read -r -d '' file; do
        # Check if it's a regular file
        if [ -f "$file" ]; then
            local size
            size=$(stat -f %z "$file" 2>/dev/null) || continue
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)

    echo "$total"
}
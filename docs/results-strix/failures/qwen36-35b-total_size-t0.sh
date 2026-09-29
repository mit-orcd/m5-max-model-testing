total_size() {
    local dir="$1"
    local total=0

    # Use find with -print0 to handle hostile filenames
    # Use stat on macOS (BSD) to get file size
    # -f %z gives the size in bytes for regular files
    
    while IFS= read -r -d '' file; do
        # Get size using stat
        # On macOS, stat -f %z file gives size in bytes
        local size
        size=$(stat -f %z -- "$file" 2>/dev/null || stat -f %z "$file" 2>/dev/null)
        if [[ -n "$size" ]]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)

    echo "$total"
}
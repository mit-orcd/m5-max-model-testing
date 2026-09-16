total_size() {
    local dir="$1"
    local total=0

    # Use find with -print0 to handle hostile filenames
    # Use stat on macOS (BSD) to get file size
    # -f %z gives the size in bytes for regular files
    while IFS= read -r -d '' file; do
        # Check if it's a regular file
        if [ -f "$file" ]; then
            # Get size using stat
            size=$(stat -f %z "$file" 2>/dev/null)
            if [ -n "$size" ]; then
                total=$((total + size))
            fi
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)

    echo "$total"
}
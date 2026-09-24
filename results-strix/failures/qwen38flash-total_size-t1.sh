total_size() {
    local dir="$1"
    local total=0
    local file_size

    # Use find to list all regular files, using -print0 to handle hostile filenames
    # We pipe to a while loop reading null-delimited input
    while IFS= read -r -d '' file; do
        # Get file size using stat (BSD/macOS syntax: -f %z for size in bytes)
        file_size=$(stat -f %z "$file" 2>/dev/null)
        if [ -n "$file_size" ]; then
            total=$((total + file_size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)

    echo "$total"
}
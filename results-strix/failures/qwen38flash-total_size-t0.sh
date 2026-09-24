total_size() {
    local dir="$1"
    local total=0
    local file_size

    # Use find to locate regular files, handling all special characters in filenames
    # -print0 outputs null-delimited paths, safe for any filename
    # We use a while loop with read -d '' to process each file
    # stat -f %z gets the size in bytes on macOS/BSD
    
    while IFS= read -r -d '' file; do
        # Get file size using BSD stat
        file_size=$(stat -f %z "$file" 2>/dev/null)
        if [ -n "$file_size" ]; then
            total=$((total + file_size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)

    echo "$total"
}
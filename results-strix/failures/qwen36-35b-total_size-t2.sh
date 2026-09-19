total_size() {
    local dir="$1"
    local total=0
    
    if [ ! -d "$dir" ]; then
        echo 0
        return
    fi
    
    # Use find with -print0 and handle null-delimited input
    # On macOS BSD find, we can use -exec or pipe with -print0
    # To avoid issues with filenames, we use -print0 and read with null delimiter
    
    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            # Use stat to get size; on macOS, stat -f %z gives size in bytes
            local size
            size=$(stat -f %z "$file" 2>/dev/null) || continue
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    echo "$total"
}
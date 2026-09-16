total_size() {
    local dir="$1"
    local total=0
    local file
    
    # Use find with -print0 and handle null-terminated entries
    while IFS= read -r -d '' file; do
        # Skip if not a regular file (e.g., symlinks to directories)
        if [[ -f "$file" && ! -d "$file" ]]; then
            # Get file size using stat with BSD format
            local size
            size=$(stat -f %z -- "$file" 2>/dev/null) || continue
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    echo "$total"
}
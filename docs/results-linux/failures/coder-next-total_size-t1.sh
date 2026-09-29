total_size() {
    local dir="$1"
    local total=0
    local file
    
    # Use find with -print0 to handle all special characters in filenames
    # Use stat -f %z on macOS to get file size in bytes
    while IFS= read -r -d '' file; do
        # Skip if not a regular file (e.g., symlinks to directories, etc.)
        if [[ -f "$file" && ! -d "$file" ]]; then
            local size
            size=$(stat -f %z -- "$file" 2>/dev/null) || size=0
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    printf '%d\n' "$total"
}
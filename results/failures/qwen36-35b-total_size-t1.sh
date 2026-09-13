total_size() {
    local dir="$1"
    local total=0
    local fsize

    # Use find with -print0 and read with null delimiter to handle hostile filenames
    # -type f ensures we only count regular files
    # -maxdepth is not strictly necessary but find traverses recursively by default
    # On macOS (BSD find), we avoid GNU-specific extensions
    
    while IFS= read -r -d '' fsize; do
        total=$((total + fsize))
    done < <(find "$dir" -type f -exec stat -f '%z' {} + 2>/dev/null)

    echo "$total"
}
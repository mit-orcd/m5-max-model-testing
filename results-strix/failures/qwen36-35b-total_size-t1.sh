total_size() {
    local dir="$1"
    
    # If the directory doesn't exist or is not a directory, output 0
    if [ ! -d "$dir" ]; then
        echo 0
        return
    fi

    # Use find with -print0 to handle hostile filenames.
    # Use stat on macOS (BSD) to get file size.
    # BSD stat format: %z is the size in bytes.
    # We sum them up using awk.
    
    find "$dir" -type f -print0 2>/dev/null | \
    while IFS= read -r -d '' file; do
        stat -f '%z' "$file" 2>/dev/null
    done | awk '{s+=$1} END {print s+0}'
}
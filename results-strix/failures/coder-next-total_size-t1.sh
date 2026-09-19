total_size() {
    local dir="$1"
    local sum=0
    local file
    local size
    
    # Use find to get all regular files, then use stat to get sizes
    # -print0 and null delimiter handling for hostile filenames
    while IFS= read -r -d '' file; do
        # Use stat with format to get size in bytes
        # BSD stat uses -f with format specifiers
        size=$(stat -f '%z' -- "$file" 2>/dev/null)
        if [[ -n "$size" && "$size" =~ ^[0-9]+$ ]]; then
            sum=$((sum + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    echo "$sum"
}
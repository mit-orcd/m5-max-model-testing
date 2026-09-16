total_size() {
    local dir="$1"
    local total=0

    # Use find with -print0 to handle hostile filenames.
    # Use stat on macOS (BSD) to get size.
    # macOS stat: %z is size in bytes.
    
    # Handle case where directory doesn't exist or is empty
    if [ ! -d "$dir" ]; then
        echo 0
        return
    fi

    # Use find to locate regular files, null-delimited
    # Then use a while loop to read them safely
    while IFS= read -r -d '' file; do
        # Use stat to get file size
        # On macOS/BSD: stat -f %z file
        # We must handle filenames starting with - by using ./ or -- if supported, 
        # but stat might not support --. Using ./$file is safer for most tools, 
        # but if path is absolute, it's fine. If relative and starts with -, ./ helps.
        # However, find outputs paths. If we cd into dir, we can use relative paths.
        
        local size
        size=$(stat -f %z "$file" 2>/dev/null)
        if [ $? -eq 0 ]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0)

    echo "$total"
}
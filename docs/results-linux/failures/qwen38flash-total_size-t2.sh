total_size() {
    local dir="$1"
    local sum=0
    local file
    # Use find with -print0 to handle all filenames safely
    # BSD find: -type f selects regular files
    while IFS= read -r -d '' file; do
        # Get file size in bytes using stat
        # BSD stat: -f %z gives size in bytes
        local size
        size=$(stat -f "%z" "$file" 2>/dev/null)
        if [ -n "$size" ]; then
            sum=$((sum + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf "%d\n" "$sum"
}
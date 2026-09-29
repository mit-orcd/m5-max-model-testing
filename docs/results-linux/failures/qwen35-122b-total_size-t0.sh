total_size() {
    local dir="$1"
    local total=0
    local size
    local file

    # Use find with -print0 to handle hostile filenames, then read with null delimiter
    while IFS= read -r -d '' file; do
        # Get size using stat (BSD version on macOS)
        # -f %z gives size in bytes
        size=$(stat -f %z "$file" 2>/dev/null)
        if [[ -n "$size" ]]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)

    echo "$total"
}
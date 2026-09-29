total_size() {
    local dir="$1"
    local total=0
    local size

    # Use find with -print0 to handle hostile filenames
    # Use stat -f%z to get size in bytes on BSD/macOS
    while IFS= read -r -d '' file; do
        # Check if it is a regular file
        if [[ -f "$file" ]]; then
            size=$(stat -f%z -- "$file" 2>/dev/null)
            if [[ -n "$size" && "$size" =~ ^[0-9]+$ ]]; then
                total=$((total + size))
            fi
        fi
    done < <(find -- "$dir" -type f -print0 2>/dev/null)

    echo "$total"
}
total_size() {
    local dir="$1"
    local total=0
    local size

    # Use find with -print0 to handle hostile filenames
    # Use stat to get file size (BSD syntax)
    while IFS= read -r -d '' file; do
        # stat -f%z works on BSD/macOS for file size in bytes
        size=$(stat -f%z "$file" 2>/dev/null)
        if [[ -n "$size" ]]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)

    printf '%d\n' "$total"
}
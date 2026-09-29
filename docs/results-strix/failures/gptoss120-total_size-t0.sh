total_size() {
    local dir=$1
    local sum=0

    # If no directory supplied, treat as empty
    [ -z "$dir" ] && { printf '0\n'; return; }

    while IFS= read -r -d '' file; do
        # Get size in bytes; ignore files we can't stat
        size=$(stat -f%z "$file" 2>/dev/null) || continue
        sum=$((sum + size))
    done < <(find "$dir" -type f -print0 2>/dev/null)

    printf '%s\n' "$sum"
}
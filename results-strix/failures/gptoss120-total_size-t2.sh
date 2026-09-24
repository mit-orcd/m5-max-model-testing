total_size() {
    local dir=$1
    local total=0

    if [ -z "$dir" ]; then
        echo "Usage: total_size <dir>" >&2
        return 1
    fi

    while IFS= read -r -d '' file; do
        # stat on macOS (BSD) returns size in bytes with %z
        size=$(stat -f %z "$file" 2>/dev/null) || size=0
        total=$(( total + size ))
    done < <(find "$dir" -type f -print0 2>/dev/null)

    printf '%s\n' "$total"
}
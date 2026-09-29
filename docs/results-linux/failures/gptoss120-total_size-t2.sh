total_size() {
    local dir=$1
    local total=0

    # If no directory given, output 0
    [ -z "$dir" ] && { printf '0\n'; return; }

    # Recursively find regular files and sum their sizes
    while IFS= read -r -d '' file; do
        size=$(stat -f %z "$file")
        total=$((total + size))
    done < <(find "$dir" -type f -print0 2>/dev/null)

    printf '%s\n' "$total"
}
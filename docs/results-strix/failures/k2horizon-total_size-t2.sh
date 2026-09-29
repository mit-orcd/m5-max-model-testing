total_size() {
    local dir=$1
    local total=0
    local entry
    local size
    while IFS= read -r -d '' entry; do
        if [ -f "$entry" ]; then
            size=$(stat -f %z -- "$entry")
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%s\n' "$total"
}
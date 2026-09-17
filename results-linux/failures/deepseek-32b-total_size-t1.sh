total_size() {
    local dir=$1
    local total=0
    find "$dir" -type f -print0 2>/dev/null | while IFS= read -d '' -r file; do
        size=$(stat -f %z "$file")
        total=$((total + size))
    done
    echo "$total"
}
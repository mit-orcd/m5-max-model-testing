total_size() {
    local dir=$1
    local total=0
    while IFS= read -r -d '' file; do
        total=$((total + $(stat -f %z "$file")))
    done < <(find "$dir" -type f -print0)
    printf '%d\n' "$total"
}
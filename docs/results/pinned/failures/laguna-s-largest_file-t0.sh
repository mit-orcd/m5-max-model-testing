largest_file() {
    local dir="$1"
    local largest_path=""
    local largest_size=0
    while IFS= read -r -d '' file; do
        local size
        size=$(stat -c %s "$file" 2>/dev/null)
        if [[ -n "$size" && "$size" -gt "$largest_size" ]]; then
            largest_size="$size"
            largest_path="$file"
        fi
    done < <(find "$dir" -type f -print0)
    printf '%s\n' "$largest_path"
}
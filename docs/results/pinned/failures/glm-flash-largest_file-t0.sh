largest_file() {
    local dir="$1"
    local largest_path=""
    local largest_size=0

    while IFS= read -r -d '' file; do
        local size=$(stat -c %s "$file" 2>/dev/null)
        if [ "$size" -gt "$largest_size" ]; then
            largest_size="$size"
            largest_path="$file"
        fi
    done < <(find "$dir" -type f -printf '%p\0' 2>/dev/null)

    echo "$largest_path"
}
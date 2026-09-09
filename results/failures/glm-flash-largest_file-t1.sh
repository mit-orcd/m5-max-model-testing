largest_file() {
    local dir="$1"
    local max_size=0
    local max_path=""
    local current_path=""
    while IFS= read -r -d '' file; do
        if [ -f "$file" ] && [ ! -L "$file" ]; then
            local size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
            if [ "$size" -gt "$max_size" ]; then
                max_size=$size
                current_path=$file
            fi
        fi
    done < <(find "$dir" -type f -print -exec sh -c 'echo "$@"' _ {} + 2>/dev/null)
    if [ -n "$current_path" ]; then
        echo "$current_path"
        return 0
    fi
    return 1
}
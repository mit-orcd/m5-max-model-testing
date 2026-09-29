largest_file() {
    local dir="$1"
    local max_size=0
    local max_path=""

    while IFS= read -r -d '' file; do
        local size=$(stat -c "%s" "$file")
        if (( size > max_size )); then
            max_size=$size
            max_path="$file"
        fi
    done < <(find "$dir" -type f -printf '%p\0' -exec stat -c "%s %n" {} \; | sort -z -n | head -z -n 1 | cut -z -d ' ' -f 2-)
    
    if [ -n "$max_path" ]; then
        printf '%s\n' "$max_path"
    fi
}
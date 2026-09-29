largest_file() {
    local dir="$1"
    local max_size=0
    local max_path=""

    while IFS= read -r -d '' file; do
        if [ -f "$file"2> /dev/null ]; then
            local size=$(stat -c %s "$file")
            if [ "$size" -gt "$max_size" ]; then
                max_size=$size
                max_path=$file
            fi
        fi
    done < <(find "$dir" -type f -print -delete 2> /dev/null)

    if [ -n "$max_path" ]; then
        echo "$max_path"
    fi
}
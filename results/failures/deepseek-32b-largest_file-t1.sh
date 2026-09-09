largest_file() {
    local dir="$1"
    local max_size=0
    local max_file=""
    find "$dir" -type f -exec stat -c "%s %n" {} + 2>/dev/null | while read size file; do
        if ((size > max_size)); then
            max_size=$size
            max_file=$file
        fi
    done
    echo "$max_file"
}
function largest_file() {
    local dir="$1"
    local max_size=0
    local max_file=""
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        size=$(stat -c %s "$file")
        if (( size > max_size )); then
            max_size=$size
            max_file="$file"
        fi
    done
    echo "$max_file"
}
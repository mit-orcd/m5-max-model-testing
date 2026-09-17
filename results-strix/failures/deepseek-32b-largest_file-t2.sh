largest_file() {
    local max_size=0 max_file=""
    find "$1" -type f | while read -r file; do
        size=$(stat -c %s "$file")
        if [[ $size -gt $max_size ]] || [[ $size -eq $max_size && "$file" < "$max_file" ]]; then
            max_size=$size
            max_file="$file"
        fi
    done
    echo "$max_file"
}
largest_file() {
    local dir="$1"
    local max_size=0
    local max_file=""
    find "$dir" -type f -size +1M -exec du -ch {} + | sort -rh | awk '{$1=""; print $0}' | while read -r file_size file_path; do
        if [[ $file_size -gt $max_size ]]; then
            max_size=$file_size
            max_file=$file_path
        fi
    done
    echo "$max_file"
}
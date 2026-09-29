function largest_file() {
    local dir="$1"
    local max_size=0
    local max_file=""
    find "$dir" -type f -exec du -ch {} + | sort -rh | awk '{ if ($1 > '$(printf "%d\n" "$max_size")') { max_file = $2; max_size = $1 } }'
}
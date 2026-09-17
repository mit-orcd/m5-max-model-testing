function largest_file() {
    local dir="$1"
    local max_size=0
    local max_file=""
    find "$dir" -type f -exec du -ch {} + | sort -rh | awk '{ if ($1 > '$(du -ch "$dir" | awk '{print $1}')') { max_size = $1; max_file = $2 } }'
    echo "$max_file"
}
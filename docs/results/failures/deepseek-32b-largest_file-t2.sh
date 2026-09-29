largest_file() {
    local max_size=0 max_file=
    find "$1" -type f -exec stat -c "%s %n" {} + | 
    while read -r size file; do
        ((size > max_size)) && max_size=$size && max_file=$file
    done
    echo "$max_file"
}
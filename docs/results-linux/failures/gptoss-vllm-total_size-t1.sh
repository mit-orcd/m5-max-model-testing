total_size() {
    local dir=$1
    if [ -z "$dir" ]; then
        printf 1>&2 'total_size: missing directory argument\n'
        return 1
    fi
    find "$dir" -type f -print0 |
    while IFS= read -r -d '' file; do
        stat -f%z "$file"
    done | awk 'BEGIN{sum=0} {sum+=$1} END{print sum}'
}
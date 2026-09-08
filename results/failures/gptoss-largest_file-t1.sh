largest_file() {
    local dir="$1"
    find "$dir" -type f -printf "%s\t%p\n" \
        | sort -nr -k1,1 \
        | head -n1 \
        | cut -f2-
}
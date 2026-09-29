largest_file() {
    local max_size=0 max_file=
    find "$1" -type f -exec sh -c 's=$(stat -c%s "$1"); if [ "$s" -gt "$max_size" ]; then max_size="$s"; max_file="$1"; fi' _ {} \;
    printf '%s\n' "$max_file"
}
largest_file() {
    local dir=${1:-.}
    find "$dir" -type f -printf '%s %p\0' 2>/dev/null |
        sort -z -nr |
        head -z -n1 |
        cut -z -d' ' -f2- |
        tr -d '\0'
}
largest_file() {
    local dir=${1:-.}
    if [[ ! -d $dir ]]; then
        return 1
    fi
    find "$dir" -type f -printf '%s %p\n' |
        sort -nr |
        awk 'NR==1{ $1=""; sub(/^ /,""); print }'
}
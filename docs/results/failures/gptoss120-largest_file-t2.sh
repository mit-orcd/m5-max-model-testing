largest_file() {
    local dir=${1:-.}
    local maxsize=-1 maxpath=
    while IFS= read -r -d '' size path; do
        if (( size > maxsize )); then
            maxsize=$size
            maxpath=$path
        fi
    done < <(find "$dir" -type f -printf '%s %p\0')
    printf '%s\n' "$maxpath"
}
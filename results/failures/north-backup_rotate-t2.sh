```bash
function backup_rotate {
    local dir=$1
    local keep=$2
    shopt -s nullglob
    local files=( "$dir"/backup-*.tar.gz )
    # Sort filenames lexicographically (ascending)
    local sorted
    sorted=$(printf '%s\n' "${files[@]}" | sort)
    local count
    count=$(printf '%s\n' "$sorted" |
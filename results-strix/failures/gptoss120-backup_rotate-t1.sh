backup_rotate() {
    local dir=$1 keep=$2
    (( keep < 0 )) && keep=0

    shopt -s nullglob
    local files=("$dir"/backup-*.tar.gz)
    shopt -u nullglob

    local total=${#files[@]}
    (( total <= keep )) && return

    # sort lexicographically (oldest first)
    IFS=$'\n' read -r -d '' -a sorted < <(printf '%s\0' "${files[@]}" | sort -z)

    local to_del=$((total - keep))
    for (( i=0; i<to_del; i++ )); do
        local f="${sorted[i]}"
        rm -f -- "$f"
        echo "$(basename "$f")"
    done
}
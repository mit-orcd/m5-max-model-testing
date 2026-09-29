backup_rotate() {
    local dir="$1"
    local keep="$2"
    local -a files
    IFS=$'\n' read -r -d '' -a files < <(compgen -c ls --color=never -- "$dir"/backup-*.tar.gz 2>/dev/null | sort)
    local count=${#files[@]}
    if (( count <= keep )); then
        return
    fi
    local start=$(( count - keep ))
    for (( i=start; i<count; i++ )); do
        local file="${files[$i]}"
        local name="${file##*/}"
        rm -- "$file"
        printf '%s\n' "$name"
    done
}
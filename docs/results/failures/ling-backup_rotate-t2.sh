backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()

    shopt -s nullglob
    for f in "$dir"/backup-*.tar.gz; do
        files+=("$(basename -- "$f")")
    done
    shopt -u nullglob

    if ((${#files[@]} <= keep)); then
        return
    fi

    IFS=$'\n' sorted=($(printf '%s\n' "${files[@]}" | sort))
    unset IFS

    local to_delete_count=$((${#sorted[@]} - keep))
    for ((i = 0; i < to_delete_count; i++)); do
        printf '%s\n' "${sorted[i]}"
    done
}
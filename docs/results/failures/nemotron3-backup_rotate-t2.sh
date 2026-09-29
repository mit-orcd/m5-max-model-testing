backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    local f
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$dir" -maxdepth 1 -type f -name "backup-*.tar.gz" -print0 | sort -rz)

    if (( ${#files[@]} <= keep )); then
        return
    fi

    for (( i=keep; i<${#files[@]}; i++ )); do
        basename "${files[i]}"
    done | while read -r fname; do
        rm -f "$dir/$fname"
        printf '%s\n' "$fname"
    done
}
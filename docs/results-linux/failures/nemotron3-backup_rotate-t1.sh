backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    local f
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -print0 | sort -z -r)
    if (( ${#files[@]} <= keep )); then
        return
    fi
    for (( i=keep; i<${#files[@]}; i++ )); do
        basename "${files[i]}"
        rm -f "${files[i]}"
    done
}
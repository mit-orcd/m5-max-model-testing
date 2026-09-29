backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0 | sort -rz)
    local count=${#files[@]}
    if (( count <= keep )); then
        return 0
    fi
    local to_delete=$((count - keep))
    local i
    for ((i = 0; i < to_delete; i++)); do
        local file="${files[i]}"
        rm -f "$file"
        basename "$file"
    done
}
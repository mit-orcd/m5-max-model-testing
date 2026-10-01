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
    local deleted=()
    for ((i = count - 1; i >= 0; i--)); do
        if (( ${#deleted[@]} >= to_delete )); then
            break
        fi
        deleted+=("${files[i]}")
    done
    for ((i = ${#deleted[@]} - 1; i >= 0; i--)); do
        rm -f "${deleted[i]}"
        basename "${deleted[i]}"
    done
}
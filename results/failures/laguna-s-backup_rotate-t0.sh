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
    local i
    for ((i = keep; i < count; i++)); do
        local basename
        basename=$(basename "${files[$i]}")
        rm -f "${files[$i]}"
        printf '%s\n' "$basename"
    done
}
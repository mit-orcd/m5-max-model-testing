backup_rotate() {
    local dir="$1" keep="$2"
    local files=()
    while IFS= read -r -d '' f; do
        files+=("$(basename "$f")")
    done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0 | sort -rz)
    if (( ${#files[@]} <= keep )); then
        return 0
    fi
    for ((i = keep; i < ${#files[@]}; i++)); do
        echo "${files[i]}"
    done | sort -r | while IFS= read -r name; do
        rm -f "$dir/$name"
    done
}
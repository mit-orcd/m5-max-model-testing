backup_rotate() {
    local dir="$1" keep="$2"
    local backups=()
    while IFS= read -r -d '' f; do
        backups+=("$f")
    done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0 | sort -rz)
    local count=${#backups[@]}
    if (( count <= keep )); then
        return 0
    fi
    local to_delete=$(( count - keep ))
    local i
    for (( i = 0; i < to_delete; i++ )); do
        local file="${backups[$i]}"
        rm -f "$file"
        printf '%s\n' "$(basename "$file")"
    done
}
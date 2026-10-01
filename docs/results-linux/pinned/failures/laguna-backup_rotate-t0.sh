backup_rotate() {
    local dir="$1"
    local keep="$2"
    local backups=()
    
    while IFS= read -r -d '' file; do
        backups+=("$(basename "$file")")
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -print0 | sort -z)
    
    local count=${#backups[@]}
    
    if (( count <= keep )); then
        return
    fi
    
    local to_delete=$((count - keep))
    
    for ((i = 0; i < to_delete; i++)); do
        echo "${backups[$i]}"
    done
}
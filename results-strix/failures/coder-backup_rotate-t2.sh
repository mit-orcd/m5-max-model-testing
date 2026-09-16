backup_rotate() {
    local dir="$1"
    local keep="$2"
    local -a backups
    local i=0
    
    # Find all backup-*.tar.gz files and store them
    while IFS= read -r -d '' file; do
        backups[i]="${file##*/}"
        ((i++))
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -print0 | sort -rz)
    
    local total=${#backups[@]}
    
    # If we have <= keep backups, do nothing
    if (( total <= keep )); then
        return
    fi
    
    # Delete the oldest backups (all but the keep newest)
    local delete_count=$((total - keep))
    for ((i=0; i<delete_count; i++)); do
        echo "${backups[i]}"
    done
}
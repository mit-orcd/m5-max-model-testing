backup_rotate() {
    local dir="$1"
    local keep="$2"
    local -a backups
    local i=0
    
    # Find all backup files and store in array
    while IFS= read -r -d '' file; do
        backups[i]="${file##*/}"  # Extract just the filename
        ((i++))
    done < <(find "$dir" -maxdepth 1 -name "backup-*.tar.gz" -print0 | sort -rz)
    
    local total=${#backups[@]}
    
    # If we have <= keep backups, do nothing
    if (( total <= keep )); then
        return
    fi
    
    # Delete the oldest backups (all but the last keep ones)
    local delete_count=$((total - keep))
    for ((i=0; i<delete_count; i++)); do
        echo "${backups[i]}"
        rm "$dir/${backups[i]}"
    done
}
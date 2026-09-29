backup_rotate() {
    local dir="$1"
    local keep="$2"
    local -a backups
    local i=0
    
    # Find all backup-*.tar.gz files
    while IFS= read -r -d '' file; do
        backups[i]="${file##*/}"  # Store just the filename
        ((i++))
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -print0 2>/dev/null)
    
    local total=${#backups[@]}
    
    # If we have <= keep backups, do nothing
    if (( total <= keep )); then
        return
    fi
    
    # Sort backups lexicographically (newest first)
    IFS=$'\n' sorted=($(sort -r <<<"${backups[*]}"))
    unset IFS
    
    # Delete oldest backups (keep the newest keep ones)
    local delete_count=$((total - keep))
    for ((i=total-delete_count; i<total; i++)); do
        echo "${sorted[i]}"
        rm "$dir/${sorted[i]}" 2>/dev/null
    done
}
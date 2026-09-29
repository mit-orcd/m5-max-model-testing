backup_rotate() {
    local dir="$1"
    local keep="$2"
    local -a backups
    local i=0
    
    # Find all backup-*.tar.gz files
    while IFS= read -r -d '' file; do
        backups[i]="${file##*/}"
        ((i++))
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -print0 2>/dev/null)
    
    # Sort lexicographically (newest first)
    IFS=$'\n' sorted=($(sort -r <<<"${backups[*]}"))
    unset IFS
    
    # If we have <= keep backups, do nothing
    if (( ${#sorted[@]} <= keep )); then
        return
    fi
    
    # Delete the oldest backups (keep the newest keep ones)
    for ((i=keep; i<${#sorted[@]}; i++)); do
        echo "${sorted[i]}"
        rm -f "$dir/${sorted[i]}"
    done
}
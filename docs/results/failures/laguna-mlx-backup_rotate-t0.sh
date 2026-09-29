backup_rotate() {
    local dir="$1"
    local keep="$2"
    local count
    local deleted=()
    local file
    
    # Count matching files
    count=$(find "$dir" -maxdepth 1 -name "backup-*.tar.gz" -type f | wc -l)
    
    # If count is less than or equal to keep, do nothing
    if [ "$count" -le "$keep" ]; then
        return
    fi
    
    # Get files sorted lexicographically (oldest first)
    while IFS= read -r file; do
        deleted+=("$file")
    done < <(find "$dir" -maxdepth observer -name "backup-*.tar.gz" -type f | sort)
    
    # Calculate how many to delete
    local to_delete=$((count - keep))
    
    # Delete oldest files (first 'to_delete' files)
    for ((i=0; i<to_delete; i++)); do
        rm "${deleted[$i]}"
        basename "${deleted[$i]}"
    done
}
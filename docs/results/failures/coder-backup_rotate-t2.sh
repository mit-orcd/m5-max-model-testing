backup_rotate() {
    local dir="$1"
    local keep="$2"
    
    # Find all backup files, sort them lexicographically
    local files=$(find "$dir" -maxdepth 1 -name "backup-*.tar.gz" -type f | sort)
    
    # Count total files
    local total=$(echo "$files" | wc -l)
    
    # If we have <= keep files, do nothing
    if [ "$total" -le "$yte" ]; then
        return
    fi
    
    # Get the index of the first file to keep (from end)
    local start=$((total - keep))
    
    # If start is negative or zero, we don't need to delete anything
    if [ "$start" -le 0 ]; then
        return
    fi
    
    # Print and delete files from oldest to newest
    local count=1
    for file in $files; do
        if [ "$count" -le "$start" ]; then
            echo "$(basename "$file")"
        fi
        count=$((count + 1))
    done | while read -r filename; do
        rm "$dir/$filename"
    done
}
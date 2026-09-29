backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    
    # Get all matching files, sorted lexicographically descending (newest first)
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0 | sort -z -r)
    
    # If total files <= keep, exit without doing anything
    if [[ ${#files[@]} -le $keep ]]; then
        return 0
    fi
    
    # Delete oldest files (those beyond the first 'keep' in the descending list)
    # Since files array is newest first, delete from index 'keep' onward
    for ((i=keep; i<${#files[@]}; i++)); do
        rm -f "${files[i]}"
        basename "${files[i]}"
    done
}
backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files
    local count
    local i=0

    # Check if directory exists
    if [[ ! -d "$dir" ]]; then
        return 1
    fi

    # Find all matching files, sorted lexicographically (which corresponds to newest first for YYYYMMDD formats, but we need to be careful)
    # The problem states "newest = lexicographically greatest names"
    # So we sort them in reverse order (newest first)
    
    # Get list of matching files
    files=()
    while IFS= read -r file; do
        # Extract just the filename
        local fname
        fname="$(basename "$file")"
        files+=("$fname")
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -type f | sort -r)

    count=${#files[@]}

    # If less than or equal to keep, do nothing
    if (( count <= keep )); then
        return 0
    fi

    # Calculate how many to delete
    local delete_count=$(( count - keep ))

    # The files array is sorted with newest first (reverse lex order)
    # We want to delete the oldest ones first
    # Oldest are at the end of the reversed list
    # So we iterate from index 'keep' to 'count - 1'
    
    while (( i < delete_count )); do
        local file_index=$(( keep + i ))
        local fname="${files[$file_index]}"
        echo "$fname"
        rm -f "$dir/$fname"
        ((i++))
    done
}
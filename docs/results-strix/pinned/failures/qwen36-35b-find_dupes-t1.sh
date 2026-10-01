find_dupes() {
    local dir="${1:-.}"
    
    # Use find to get all regular files, then compute md5 for each
    # We need to handle filenames with spaces/newlines carefully
    # Using null-delimited output from find and processing with while read
    
    declare -A md5_to_files
    
    while IFS= read -r -d '' file; do
        # Compute MD5 using md5 -r (BSD style)
        # md5 -r outputs: "MD5 (filename) = hash" or "MD5: hash" depending on version
        # On macOS, md5 -r outputs: "MD5 (file) = hash"
        local hash
        hash=$(md5 -r "$file" 2>/dev/null | awk '{print $NF}')
        
        if [[ -n "$hash" ]]; then
            if [[ -n "${md5_to_files[$hash]}" ]]; then
                md5_to_files[$hash]+=$'\n'"$file"
            else
                md5_to_files[$hash]="$file"
            fi
        fi
    done < <(find "$dir" -type f -print0)
    
    # Collect groups with 2+ files
    local groups=()
    for hash in "${!md5_to_files[@]}"; do
        local files_str="${md5_to_files[$hash]}"
        # Count number of files
        local count
        count=$(echo "$files_str" | wc -l)
        if [[ "$count" -ge 2 ]]; then
            # Sort the files in this group
            local sorted_files
            sorted_files=$(echo "$files_str" | sort)
            # Convert newlines to spaces
            local space_separated
            space_separated=$(echo "$sorted_files" | tr '\n' ' ' | sed 's/ $//')
            groups+=("$space_separated")
        fi
    done
    
    # Sort groups by first path
    # Each group is a line of space-separated paths
    # We need to sort these lines by the first field
    if [[ ${#groups[@]} -gt 0 ]]; then
        printf '%s\n' "${groups[@]}" | sort -t ' ' -k1,1
    fi
}
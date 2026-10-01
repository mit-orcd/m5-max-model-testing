find_dupes() {
    local dir="${1:-.}"
    
    # Use find to get all regular files, then compute md5 for each
    # We need to handle filenames with spaces/newlines carefully
    # Using null-delimited output from find and processing with while read
    
    declare -A hash_to_files
    
    while IFS= read -r -d '' file; do
        # Compute MD5 using md5 -r (BSD style)
        # md5 -r outputs: "MD5 (filename) = hash" or "hash  filename"
        # On macOS, md5 -r outputs: "MD5 (file) = hash"
        local hash
        hash=$(md5 -r "$file" 2>/dev/null | awk '{print $1}')
        
        if [[ -n "$hash" ]]; then
            if [[ -v hash_to_files["$hash"] ]]; then
                hash_to_files["$hash"]="${hash_to_files["$hash"]}"$'\n'"$file"
            else
                hash_to_files["$hash"]="$file"
            fi
        fi
    done < <(find "$dir" -type f -print0)
    
    # Collect groups with 2+ files
    local groups=()
    for hash in "${!hash_to_files[@]}"; do
        local files_str="${hash_to_files["$hash"]}"
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
    # We need to sort these lines based on the first field
    if [[ ${#groups[@]} -gt 0 ]]; then
        printf '%s\n' "${groups[@]}" | sort -t ' ' -k1,1
    fi
}
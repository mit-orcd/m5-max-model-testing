find_dupes() {
    local dir="${1:-.}"
    
    # Find all regular files, compute MD5, then group by MD5
    # Using find on macOS (BSD find) which doesn't have -printf
    # We'll use a temporary file or process substitution approach
    
    # Step 1: Generate MD5 hashes for all regular files
    # md5 -r outputs: <hash>  <filename>
    # We need to handle filenames with spaces/newlines carefully
    # Using null-delimited output where possible, but md5 -r doesn't support null
    
    # Let's use a two-pass approach or associative arrays
    
    declare -A hash_files
    
    while IFS= read -r -d '' file; do
        # Get MD5 hash
        local hash
        hash=$(md5 -r "$file" 2>/dev/null | cut -d' ' -f1)
        if [[ -n "$hash" ]]; then
            if [[ -n "${hash_files[$hash]}" ]]; then
                hash_files[$hash]="${hash_files[$hash]}"$'\n'"$file"
            else
                hash_files[$hash]="$file"
            fi
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    # Step 2: Find groups with 2+ files, sort paths within groups, sort groups by first path
    local groups=()
    
    for hash in "${!hash_files[@]}"; do
        local file_list="${hash_files[$hash]}"
        # Count files in group
        local count
        count=$(echo "$file_list" | wc -l)
        count=$(echo "$count" | tr -d ' ')
        
        if [[ "$count" -ge 2 ]]; then
            # Sort the file paths
            local sorted_files
            sorted_files=$(echo "$file_list" | sort)
            # Convert newlines to spaces
            local space_sep
            space_sep=$(echo "$sorted_files" | tr '\n' ' ' | sed 's/ $//')
            groups+=("$space_sep")
        fi
    done
    
    # Sort groups by first path (which is the first space-separated element)
    if [[ ${#groups[@]} -gt 0 ]]; then
        printf '%s\n' "${groups[@]}" | sort
    fi
}
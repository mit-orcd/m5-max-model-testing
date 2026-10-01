find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    
    # Find all regular files, compute MD5, store as "md5 path"
    # Using md5 -r on macOS which outputs "md5 filename"
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$tmpfile"
    
    # Group by MD5, collect paths, filter groups with 2+ files
    # Then sort groups by first path
    
    local current_md5=""
    local current_paths=()
    local groups=()
    
    while IFS= read -r line; do
        # Extract md5 (first field) and path (rest of line)
        local md5="${line%% *}"
        local path="${line#* }"
        
        if [[ "$md5" == "$current_md5" ]]; then
            current_paths+=("$path")
        else
            # Process previous group
            if [[ ${#current_paths[@]} -ge 2 ]]; then
                # Sort paths within group
                local sorted_paths
                sorted_paths=$(printf '%s\n' "${current_paths[@]}" | sort)
                local group_str
                group_str=$(echo "$sorted_paths" | tr '\n' ' ')
                # Trim trailing space
                group_str="${group_str% }"
                groups+=("$group_str")
            fi
            # Start new group
            current_md5="$md5"
            current_paths=("$path")
        fi
    done < "$tmpfile"
    
    # Process last group
    if [[ ${#current_paths[@]} -ge 2 ]]; then
        local sorted_paths
        sorted_paths=$(printf '%s\n' "${current_paths[@]}" | sort)
        local group_str
        group_str=$(echo "$sorted_paths" | tr '\n' ' ')
        group_str="${group_str% }"
        groups+=("$group_str")
    fi
    
    rm -f "$tmpfile"
    
    # Sort groups by first path (which is the first word in each group string)
    if [[ ${#groups[@]} -gt 0 ]]; then
        printf '%s\n' "${groups[@]}" | sort
    fi
}
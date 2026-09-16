find_dupes() {
    local dir="$1"
    [[ -z "$dir" ]] && return 1
    [[ ! -d "$dir" ]] && return 1

    # Use associative array to store "hash -> space-separated paths"
    declare -A hash_map

    # Find all regular files, handle spaces in names safely
    while IFS= read -r -d '' file; do
        # Calculate MD5 using BSD md5 -q (quiet) -r (stdin)
        # md5 -r output format: "hash  filename"
        local hash_line
        hash_line=$(md5 -r "$file" 2>/dev/null) || continue
        local hash="${hash_line%% *}"
        
        if [[ -n "${hash_map[$hash]}" ]]; then
            hash_map[$hash]="${hash_map[$hash]}"$'\n'"$file"
        else
            hash_map[$hash]="$file"
        fi
    done < <(find "$dir" -type f -print0)

    # Process groups
    for hash in "${!hash_map[@]}"; do
        local group="${hash_map[$hash]}"
        local count=0
        local first_path=""
        
        # Count files and find the first path for sorting
        while IFS= read -r path; do
            ((count++))
            if [[ $count -eq 1 ]]; then
                first_path="$path"
            fi
        done <<< "$group"

        # Only print groups with 2+ files
        if [[ $count -ge 2 ]]; then
            # Sort the paths within the group
            local sorted_paths
            sorted_paths=$(echo "$group" | sort)
            
            # Store for global sorting by first path
            echo "$first_path|$sorted_paths"
        fi
    done | sort -t'|' -k1,1 | cut -d'|' -f2- | tr '\n' ' ' | sed 's/ $/\n/'
}
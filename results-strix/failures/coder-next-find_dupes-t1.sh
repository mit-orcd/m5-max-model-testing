find_dupes() {
    local dir="$1"
    [[ -z "$dir" || ! -d "$dir" ]] && { echo "Usage: find_dupes <dir>" >&2; return 1; }
    
    # Generate md5 hashes with filenames, handle special characters in filenames
    # Use md5 -r for BSD compatibility and escape newlines in filenames
    declare -A hash_map
    while IFS= read -r -d '' file; do
        hash=$(md5 -q "$file" 2>/dev/null) || continue
        # Append file path to the array for this hash (using newline as separator)
        if [[ -n "${hash_map[$hash]}" ]]; then
            hash_map[$hash]+=$'\n'"$file"
        else
            hash_map[$hash]="$file"
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    # Collect groups with 2+ files
    local groups=()
    for hash in "${!hash_map[@]}"; do
        # Count files in the group
        local count=0
        while IFS= read -r line; do
            ((count++))
        done <<< "${hash_map[$hash]}"
        
        if ((count >= 2)); then
            # Sort the files in the group and join with spaces
            local sorted_group
            sorted_group=$(printf '%s\n' "${hash_map[$hash]}" | sort | tr '\n' ' ' | sed 's/ $//')
            groups+=("$sorted_group")
        fi
    done
    
    # Sort groups by first path and print
    printf '%s\n' "${groups[@]}" | sort
}
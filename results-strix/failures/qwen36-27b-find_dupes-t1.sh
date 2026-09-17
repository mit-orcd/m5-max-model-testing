find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        return 1
    fi

    # Associative array to map md5 -> list of files
    declare -A md5_files

    # Find all regular files recursively
    while IFS= read -r -d '' file; do
        # Compute MD5 using BSD md5 -r (outputs: hash  filename)
        local md5_output
        md5_output=$(md5 -r "$file" 2>/dev/null)
        if [[ $? -ne 0 ]]; then
            continue
        fi
        local hash
        hash=$(echo "$md5_output" | cut -d' ' -f1)
        
        # Append file to the hash group
        if [[ -n "${md5_files[$hash]+_}" ]]; then
            md5_files[$hash]+=$'\n'"$file"
        else
            md5_files[$hash]="$file"
        fi
    done < <(find "$dir" -type f -print0)

    # Collect groups with 2+ files
    local groups=()
    for hash in "${!md5_files[@]}"; do
        local files_str="${md5_files[$hash]}"
        # Count number of files (lines)
        local count
        count=$(echo "$files_str" | wc -l | tr -d ' ')
        if [[ "$count" -ge 2 ]]; then
            # Sort the files within the group
            local sorted_files
            sorted_files=$(echo "$files_str" | sort)
            # Join with spaces
            local joined
            joined=$(echo "$sorted_files" | tr '\n' ' ' | sed 's/ $//')
            groups+=("$joined")
        fi
    done

    # Sort groups by first path (which is the first element in each group string)
    if [[ ${#groups[@]} -gt 0 ]]; then
        printf '%s\n' "${groups[@]}" | sort
    fi
}
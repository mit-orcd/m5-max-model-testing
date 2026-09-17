find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        return 1
    fi

    # Associative array to map md5 -> list of files
    declare -A md5_files

    # Find all regular files recursively
    while IFS= read -r -d '' file; do
        # Compute MD5 using BSD md5 -r (format: hash  filename)
        md5_output=$(md5 -r "$file" 2>/dev/null)
        if [[ $? -ne 0 ]]; then
            continue
        fi
        md5_hash=$(echo "$md5_output" | cut -d ' ' -f 1)
        
        # Append file to the list for this hash
        if [[ -n "${md5_files[$md5_hash]}" ]]; then
            md5_files[$md5_hash]="${md5_files[$md5_hash]}"$'\n'"$file"
        else
            md5_files[$md5_hash]="$file"
        fi
    done < <(find "$dir" -type f -print0)

    # Collect groups with 2+ files
    local groups=()
    for md5_hash in "${!md5_files[@]}"; do
        local files_str="${md5_files[$md5_hash]}"
        # Count number of files (lines)
        local count
        count=$(echo "$files_str" | wc -l | tr -d ' ')
        if [[ "$count" -ge 2 ]]; then
            # Sort the files within the group
            local sorted_files
            sorted_files=$(echo "$files_str" | sort)
            # Join with spaces
            local group_line
            group_line=$(echo "$sorted_files" | tr '\n' ' ' | sed 's/ $//')
            groups+=("$group_line")
        fi
    done

    # Sort groups by first path (which is the first word in each line)
    if [[ ${#groups[@]} -gt 0 ]]; then
        printf '%s\n' "${groups[@]}" | sort -k1,1
    fi
}
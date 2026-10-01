find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        return 1
    fi

    # Associative array to map md5 hash to list of files
    declare -A hash_files

    # Find all regular files recursively
    while IFS= read -r -d '' file; do
        # Compute MD5 hash using BSD md5 -r (format: hash  filename)
        # We only need the hash part
        local hash
        hash=$(md5 -r "$file" | cut -d ' ' -f 1)
        
        # Append file to the list for this hash
        if [[ -n "${hash_files[$hash]}" ]]; then
            hash_files[$hash]+=$'\n'"$file"
        else
            hash_files[$hash]="$file"
        fi
    done < <(find "$dir" -type f -print0)

    # Collect groups with 2+ files
    local groups=()
    for hash in "${!hash_files[@]}"; do
        local files_str="${hash_files[$hash]}"
        # Count number of files (lines)
        local count
        count=$(echo "$files_str" | wc -l | tr -d ' ')
        if (( count >= 2 )); then
            # Sort the files within the group
            local sorted_files
            sorted_files=$(echo "$files_str" | sort)
            # Join with spaces
            local group_line
            group_line=$(echo "$sorted_files" | tr '\n' ' ' | sed 's/ $//')
            groups+=("$group_line")
        fi
    done

    # Sort groups by the first path (which is the first word in each line)
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort -k1,1
    fi
}
find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        return 1
    fi

    # Associative array to map MD5 -> list of files
    declare -A hash_to_files

    # Find all regular files recursively
    while IFS= read -r -d '' file; do
        # Compute MD5 using BSD md5 -r (which outputs: hash  filename)
        # We only need the hash
        local hash
        hash=$(md5 -q "$file" 2>/dev/null)
        if [[ -z "$hash" ]]; then
            continue
        fi
        # Append file path to the list for this hash
        if [[ -n "${hash_to_files[$hash]}" ]]; then
            hash_to_files[$hash]+=$'\n'"$file"
        else
            hash_to_files[$hash]="$file"
        fi
    done < <(find "$dir" -type f -print0)

    # Process each group: only keep groups with 2+ files
    local -a groups=()
    for hash in "${!hash_to_files[@]}"; do
        local files_str="${hash_to_files[$hash]}"
        # Count number of files (lines)
        local count
        count=$(echo "$files_str" | wc -l)
        if (( count >= 2 )); then
            # Sort the files in this group
            local sorted_files
            sorted_files=$(echo "$files_str" | sort)
            # Join with spaces
            local line
            line=$(echo "$sorted_files" | tr '\n' ' ' | sed 's/ $//')
            # Extract the first path for sorting groups later
            local first_path
            first_path=$(echo "$sorted_files" | head -n 1)
            groups+=("${first_path}|${line}")
        fi
    done

    # Sort groups by first path
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort -t'|' -k1,1 | cut -d'|' -f2-
    fi
}
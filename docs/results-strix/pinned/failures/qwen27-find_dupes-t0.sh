find_dupes() {
    local dir="$1"
    local -a files=()
    local -A md5_map=()
    local -A path_map=()

    # Find all regular files
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -type f -print0)

    # Compute MD5 for each file and group by hash
    for file in "${files[@]}"; do
        local hash
        hash=$(md5 -r "$file" | awk '{print $1}')
        if [[ -n "${md5_map[$hash]+x}" ]]; then
            md5_map[$hash]="${md5_map[$hash]} $file"
        else
            md5_map[$hash]="$file"
        fi
    done

    # Collect groups with 2+ files
    local -a groups=()
    for hash in "${!md5_map[@]}"; do
        local -a group_files=()
        read -ra group_files <<< "${md5_map[$hash]}"
        if (( ${#group_files[@]} >= 2 )); then
            # Sort the files in this group
            local sorted
            sorted=$(printf '%s\n' "${group_files[@]}" | sort)
            groups+=("$sorted")
        fi
    done

    # Sort groups by first path
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort | while IFS= read -r line; do
            printf '%s\n' "$line"
        done
    fi
}
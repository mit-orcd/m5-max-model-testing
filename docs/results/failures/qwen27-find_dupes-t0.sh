find_dupes() {
    local dir="$1"
    local -a files=()
    local -A md5_to_files=()

    # Find all regular files recursively
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -type f -print0)

    # Compute MD5 for each file and group by hash
    local file md5
    for file in "${files[@]}"; do
        md5=$(md5 -r "$file" | awk '{print $1}')
        if [[ -n "${md5_to_files[$md5]+x}" ]]; then
            md5_to_files[$md5]+=" $file"
        else
            md5_to_files[$md5]="$file"
        fi
    done

    # Collect groups with 2+ files
    local -a groups=()
    local group
    for group in "${md5_to_files[@]}"; do
        # Count files in this group
        local -a group_files=($group)
        if (( ${#group_files[@]} >= 2 )); then
            # Sort the files in this group
            local sorted_group
            sorted_group=$(printf '%s\n' "${group_files[@]}" | sort)
            groups+=("$sorted_group")
        fi
    done

    # Sort groups by their first path
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort -k1,1
    fi
}
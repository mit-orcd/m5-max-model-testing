find_dupes() {
    local dir="$1"
    local -a files=()
    local -A md5_map=()
    local -a md5_list=()

    # Find all regular files
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -type f -print0)

    # Compute MD5 for each file and group by hash
    local file md5
    for file in "${files[@]}"; do
        md5=$(md5 -r "$file" | awk '{print $1}')
        if [[ -n "${md5_map[$md5]+x}" ]]; then
            md5_map[$md5]+=" $file"
        else
            md5_map[$md5]="$file"
            md5_list+=("$md5")
        fi
    done

    # Collect groups with 2+ files
    local -a groups=()
    local md5 group
    for md5 in "${md5_list[@]}"; do
        group="${md5_map[$md5]}"
        # Count files in group
        local -a group_files=()
        read -ra group_files <<< "$group"
        if (( ${#group_files[@]} >= 2 )); then
            # Sort the files in this group
            local sorted_group
            sorted_group=$(printf '%s\n' "${group_files[@]}" | sort)
            groups+=("$sorted_group")
        fi
    done

    # Sort groups by first path
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort
    fi
}
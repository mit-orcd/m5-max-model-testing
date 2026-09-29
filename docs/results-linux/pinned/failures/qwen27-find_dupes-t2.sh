find_dupes() {
    local dir="$1"
    local -a files=()
    local -A md5_map=()
    local -a md5s=()

    # Find all regular files recursively
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$dir" -type f -print0)

    # Compute MD5 for each file and group by hash
    local f hash
    for f in "${files[@]}"; do
        hash=$(md5 -r "$f" | awk '{print $1}')
        if [[ -n "${md5_map[$hash]+x}" ]]; then
            md5_map[$hash]+=" $f"
        else
            md5_map[$hash]="$f"
            md5s+=("$hash")
        fi
    done

    # Collect groups with 2+ files
    local -a groups=()
    local h
    for h in "${md5s[@]}"; do
        local -a group_files=()
        read -ra group_files <<< "${md5_map[$h]}"
        if (( ${#group_files[@]} >= 2 )); then
            # Sort the files in this group
            local sorted
            sorted=$(printf '%s\n' "${group_files[@]}" | sort)
            groups+=("$sorted")
        fi
    done

    # Sort groups by their first path
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort | while IFS= read -r line; do
            printf '%s\n' "$line"
        done
    fi
}
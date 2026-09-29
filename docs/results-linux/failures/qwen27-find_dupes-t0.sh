find_dupes() {
    local dir="$1"
    local -a files=()
    local -A md5_map=()
    local -a md5s=()

    # Collect all regular files
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

    # Collect groups with 2+ files, sort each group's paths, then sort groups by first path
    local -a groups=()
    local h
    for h in "${md5s[@]}"; do
        local paths_str="${md5_map[$h]}"
        local count
        count=$(echo "$paths_str" | wc -w | tr -d ' ')
        if (( count >= 2 )); then
            # Sort the paths within this group
            local sorted_paths
            sorted_paths=$(echo "$paths_str" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//')
            groups+=("$sorted_paths")
        fi
    done

    # Sort groups by first path
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort | while IFS= read -r line; do
            echo "$line"
        done
    fi
}
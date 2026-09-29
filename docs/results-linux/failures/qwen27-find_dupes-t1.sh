find_dupes() {
    local dir="$1"
    # Collect all regular files
    local files=()
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$dir" -type f -print0 | sort -z)

    # Map: md5 -> list of files
    declare -A md5_map

    for f in "${files[@]}"; do
        local hash
        hash=$(md5 -r "$f" | awk '{print $1}')
        if [[ -n "${md5_map[$hash]:-}" ]]; then
            md5_map[$hash]+=" $f"
        else
            md5_map[$hash]="$f"
        fi
    done

    # Collect groups with 2+ files
    local groups=()
    for hash in "${!md5_map[@]}"; do
        local list="${md5_map[$hash]}"
        # Count files in the list
        local count
        count=$(echo "$list" | wc -w | tr -d ' ')
        if (( count >= 2 )); then
            # Sort the paths within the group
            local sorted
            sorted=$(echo "$list" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//')
            groups+=("$sorted")
        fi
    done

    # Sort groups by first path
    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort
    fi
}
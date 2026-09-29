find_dupes() {
    local dir="$1"
    local file md5
    declare -A groups=()
    while IFS= read -r -d '' file; do
        md5=$(md5 -q "$file" 2>/dev/null) || continue
        groups["$md5"]+="$file"$'\n'
    done < <(find "$dir" -type f -print0 2>/dev/null)
    for md5 in "${!groups[@]}"; do
        local count=0
        local paths=()
        while IFS= read -r path; do
            [ -n "$path" ] && paths+=("$path")
        done <<< "${groups[$md5]}"
        if (( ${#paths[@]} > 1 )); then
            printf '%s\n' "${paths[@]}" | sort | tr '\n' ' ' | sed 's/ $//'
        fi
    done | sort
}
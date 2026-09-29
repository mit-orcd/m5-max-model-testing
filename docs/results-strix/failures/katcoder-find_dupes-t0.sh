find_dupes() {
    local dir="$1"
    declare -A seen
    while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | awk '{print $1}')
        seen["$hash"]+="$file"$'\n'
    done < <(find "$dir" -type f -print0)
    local groups=()
    for hash in "${!seen[@]}"; do
        local lines=()
        while IFS= read -r line; do
            [[ -n "$line" ]] && lines+=("$line")
        done <<< "${seen[$hash]}"
        if (( ${#lines[@]} >= 2 )); then
            IFS=$'\n' sorted=($(sort <<< "${lines[*]}")); unset IFS
            groups+=("$(IFS=' '; echo "${sorted[*]}")")
        fi
    done
    IFS=$'\n' sorted_groups=($(sort <<< "$(printf '%s\n' "${groups[@]}")")); unset IFS
    printf '%s\n' "${sorted_groups[@]}"
}
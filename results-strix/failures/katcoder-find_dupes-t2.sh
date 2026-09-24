find_dupes() {
    local dir="$1"
    declare -A seen
    while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | cut -d' ' -f1)
        seen["$hash"]+="$file"$'\n'
    done < <(find "$dir" -type f -print0)
    local groups=()
    for hash in "${!seen[@]}"; do
        local files=()
        while IFS= read -r f; do
            [[ -n "$f" ]] && files+=("$f")
        done < <(printf '%s' "${seen[$hash]}" | sort)
        if (( ${#files[@]} >= 2 )); then
            groups+=("$(IFS=' '; echo "${files[*]}")")
        fi
    done
    printf '%s\n' "${groups[@]}" | sort
}
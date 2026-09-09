function find_dupes() {
    local dir="$1"
    declare -A hash_map

    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | cut -d ' ' -f1)
        hash_map[$hash]="${hash_map[$hash]}$file\n"
    done

    groups=()
    for hash in "${!hash_map[@]}"; do
        files=()
        while IFS= read -r line; do
            files+=("$line")
        done <<< "${hash_map[$hash]}"

        if [ "${#files[@]}" -ge 2 ]; then
            sorted_files=($(printf '%s\n' "${files[@]}" | sort))
            group_line=$(printf '%s ' "${sorted_files[@]}")
            group_line=${group_line% }
            groups+=("$group_line")
        fi
    done

    IFS=$'\n' sorted_groups=($(sort <<< "${groups[*]}"))
    unset IFS

    for group in "${sorted_groups[@]}"; do
        echo "$group"
    done
}
find_dupes() {
    local dir="${1:?missing directory}"
    declare -A hash_map
    # Gather files and their MD5 hashes
    while IFS= read -r -d '' file; do
        # md5 -r prints: "<hash> <filename>"
        local hash
        hash=$(md5 -r "$file" | awk '{print $1}')
        hash_map["$hash"]+=$file$'\0'
    done < <(find "$dir" -type f -print0)

    # Build groups of duplicates
    local -a groups
    for hash in "${!hash_map[@]}"; do
        IFS=$'\0' read -ra files <<< "${hash_map[$hash]}"
        (( ${#files[@]} > 1 )) || continue
        # Sort paths within the group
        local sorted
        sorted=$(printf '%s\n' "${files[@]}" | sort | tr '\n' ' ')
        sorted=${sorted% }   # strip trailing space
        groups+=("$sorted")
    done

    # Output groups sorted by their first path
    printf '%s\n' "${groups[@]}" | sort -k1,1
}
find_dupes() {
    local dir="$1"
    if [ ! -d "$dir" ]; then
        echo "Error: Directory not found." >&2
        return 1
    fi

    declare -A hash_map

    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            hash=$(md5 -r "$file")
            hash_map["$hash"]+="$file "
        fi
    done < <(find "$dir" -type f -print0)

    for hash in "${!hash_map[@]}"; do
        files=(${hash_map[$hash]})
        if [ "${#files[@]}" -ge 2 ]; then
            IFS=$'\n' sorted_files=($(sort <<< "${hash_map[$hash]}"))
            unset IFS
            groups+=("${sorted_files[*]}")
        fi
    done

    IFS=$'\n' sorted_groups=($(sort <<< "${groups[*]}"))
    unset IFS

    for group in "${sorted_groups[@]}"; do
        echo "$group"
    done
}
find_dupes() {
    local dir="$1"
    declare -A md5_map

    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | awk '{print $1}')
        md5_map["$hash"]+="$file\n"
    done

    local groups=()
    for hash in "${!md5_map[@]}"; do
        local files=()
        while IFS= read -r file; do
            files+=("$file")
        done <<< "${md5_map[$hash]}"
        if [ "${#files[@]}" -lt 2 ]; then
            continue
        fi
        local sorted_files=($(printf "%s\n" "${files[@]}" | sort))
        local group=$(IFS=' '; echo "${sorted_files[*]}")
        groups+=("$group")
    done

    IFS=$'\n' sorted_groups=($(sort -k1,1 <<< "${groups[*]}"))

    for group in "${sorted_groups[@]}"; do
        echo "$group"
    done
}
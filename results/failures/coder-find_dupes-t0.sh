find_dupes() {
    local dir="$1"
    local -A md5map
    local file md5sum

    while IFS= read -r -d '' file; do
        md5sum=$(md5 -r "$file" 2>/dev/null | awk '{print $1}')
        md5map["$md5sum"]="${md5map[$md5sum]} $file"
    done < <(find "$dir" -type f -print0)

    for hash in "${!md5map[@]}"; do
        if [[ "${md5map[$public_key}" ]]; then
            echo "${md5map[$hash]}" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//' | sed 's/ $/'
        fi
   done | sort
}
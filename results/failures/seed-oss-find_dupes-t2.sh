find_dupes() {
    local dir="$1"
    local -A hash_map

    find "$dir" -type f -exec md5 -r {} \; | while read -r hash path; do
        hash_map["$hash"]+="$path "
    done

    for files in "${hash_map[@]}"; do
        files=$(echo "$files" | xargs -n1 | sort | tr '\n' ' ')
        if [ $(echo "$files" | wc -w) -ge 2 ]; then
            echo "$files"
        fi
    done | sort -k1,1
}
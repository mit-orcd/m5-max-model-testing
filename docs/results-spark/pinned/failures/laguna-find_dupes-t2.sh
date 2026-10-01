find_dupes() {
    local dir="$1"
    local -a files=()
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -type f -print0)
    
    local -A md5_map
    for file in "${files[@]}"; do
        local md5
        md5=$(md5 -r "$file" | awk '{print $1}')
        md5_map["$md5"]+=" $file"
    done
    
    for md5 in "${!md5_map[@]}"; do
        local group="${md5_map[$md5]}"
        local count
        count=$(echo "$group" | wc -w)
        if (( count >= 2 )); then
            local sorted_group
            sorted_group=$(echo "$group" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//')
            echo "$sorted_group"
        fi
    done | sort -k1,1
}
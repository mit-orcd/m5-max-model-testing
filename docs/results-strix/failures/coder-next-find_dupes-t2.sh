find_dupes() {
    local dir="$1"
    local -A hash_map
    
    # Find all regular files, compute MD5, group by hash
    while IFS= read -r -d '' file; do
        hash=$(md5 -q "$file" 2>/dev/null) || continue
        hash_map["$hash"]+="$file"$'\0'
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    # Process each hash group
    local groups=()
    for hash in "${!hash_map[@]}"; do
        files=()
        while IFS= read -r -d '' file; do
            files+=("$file")
        done < <(printf '%s\0' "${hash_map[$hash]}" | tr '\0' '\n' | sort -z | tr '\n' '\0')
        
        if (( ${#files[@]} >= 2 )); then
            groups+=("$(printf '%s ' "${files[@]}" | sed 's/ $//')")
        fi
    done
    
    # Sort groups by first path and print
    printf '%s\n' "${groups[@]}" | sort
}
find_dupes() {
    local dir="$1"
    local -A hash_files
    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            hash=$(md5 -r "$file" | cut -d' ' -f1)
            hash_files["$hash"]+="$file"$'\n'
        fi
    done < <(find "$dir" -type f -print0 | sort -z)
    
    for hash in "${!hash_files[@]}"; do
        local files="${hash_files[$hash]}"
        local count=$(echo -n "$files" | grep -c .)
        if (( count >= 2 )); then
            echo "$files" | sort | tr '\n' ' ' | sed 's/ $//'
        fi
    done | sort -k1,1
}
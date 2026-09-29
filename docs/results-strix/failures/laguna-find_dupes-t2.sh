find_dupes() {
    local dir="$1"
    local -A hash_files=()
    
    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            hash=$(md5 -r "$file" | cut -d' ' -f1)
            hash_files["$hash"]+="$file"$'\n'
        fi
    done < <(find "$dir" -type f -print0)
    
    for files in "${hash_files[@]}"; do
        count=$(echo -n "$files" | grep -c .)
        if (( count >= 2 )); then
            sorted=$(echo -n "$files" | sort -z | tr '\0' '\n' | sed '/^$/d')
            echo "$sorted"
        fi
    done | sort
}
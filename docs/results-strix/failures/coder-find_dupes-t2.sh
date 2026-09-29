find_dupes() {
    local dir="$1"
    local -A md5map
    local -a files
    local md5
    
    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            md5=$(md5 -r "$file" 2>/dev/null | cut -d' ' -f1)
            md5map["$md5"]+="$file"$'\n'
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    
    for files in "${md5map[@]}"; do
        if [[ $(wc -l <<< "$files") -gt 1 ]]; then
            printf '%s\n' "$files" | sort | tr '\n' ' ' | sed 's/ $//'
        fi
    done | sort
}
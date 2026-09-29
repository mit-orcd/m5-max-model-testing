backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    
    while IFS= read -r -d '' file; do
        files+=("$(basename "$file")")
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -type f -print0 | sort -rz)
    
    local count=${#files[@]}
    
    if (( count > keep )); then
        for (( i = keep; i < count; i++ )); do
            echo "${files[$i]}"
        done | while read -r name; do
            rm "$dir/$name"
        done
    fi
}
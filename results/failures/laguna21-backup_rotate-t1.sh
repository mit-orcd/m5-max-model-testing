backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    
    while IFS= read -r -d '' file; do
        files+=("$(basename "$file")")
    done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0 | sort -rz)
    
    local total=${#files[@]}
    local to_delete=$((total - keep))
    
    if [ "$to_delete" -gt 0 ]; then
        for ((i = total - 1; i >= total - to_delete; i--)); do
            echo "${files[$i]}"
        done
    fi
}
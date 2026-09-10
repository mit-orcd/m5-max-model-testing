backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    local count=0
    
    while IFS= read -r -d '' file; do
        files+=("$file")
        ((count++))
    done < <(find "$dir" -maxdepth 1 -type f -name "backup-*.tar.gz" -print0 | sort -z -r)
    
    if (( count <= keep )); then
        return
    fi
    
    local to_delete=$((count - keep))
    for (( i = keep; i < count; i++ )); do
        local basename="${files[$i]##*/}"
        echo "$basename"
    done
    
    for (( i = keep; i < count; i++ )); do
        rm -f "${files[$i]}"
    done
}
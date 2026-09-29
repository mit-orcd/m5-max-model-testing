backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    local count=0
    
    while IFS= read -r -d '' file; do
        files+=("$file")
        ((count++))
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -type f -print0 2>/dev/null | sort -rz)
    
    if (( count <= keep )); then
        return
    fi
    
    local to_delete=$((count - keep))
    for (( i = to_delete - 1; i >= 0; i-- )); do
        local basename="${files[i]##*/}"
        echo "$basename"
    done
    
    for (( i = to_delete - 1; i >= 0; i-- )); do
        rm -f "${files[i]}"
    done
}
total_size() {
    local dir="$1"
    local total=0
    local file_size
    
    if [[ ! -d "$dir" ]]; then
        echo 0
        return
    fi
    
    # Use find to locate regular files and xargs to handle them safely
    # Use stat -f %z to get file size in bytes
    while IFS= read -r -d '' file; do
        file_size=$(stat -f %z -- "$file" 2>/dev/null) || file_size=0
        total=$((total + file_size))
    done < <(find "$dir" -type f -print0)
    
    echo "$total"
}
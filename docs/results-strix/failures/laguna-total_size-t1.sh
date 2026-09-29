total_size() {
    local dir="$1"
    local total=0
    local file
    if [[ ! -d "$dir" ]]; then
        echo 0
        return
    fi
    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            size=$(stat -f%z -- "$file")
            total=$((total + size))
        fi
    done < <(find "$dir" -mindepth 1 -print0 2>/dev/null)
    echo "$total"
}
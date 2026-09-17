total_size() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        printf '0\n'
        return
    fi
    local total=0
    local f
    while IFS= read -r -d '' f; do
        local size
        size=$(stat -f '%z' -- "$f" 2>/dev/null)
        if [[ -n "$size" ]]; then
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%d\n' "$total"
}
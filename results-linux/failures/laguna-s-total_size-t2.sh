total_size() {
    local dir="$1"
    if [ ! -d "$dir" ]; then
        printf '0\n'
        return
    fi
    local total=0
    local f
    while IFS= read -r -d '' f; do
        if [ -f "$f" ]; then
            local sz
            sz=$(stat -f%z "$f" 2>/dev/null) || sz=0
            total=$((total + sz))
        fi
    done < <(find "$dir" -type f -print0)
    printf '%d\n' "$total"
}
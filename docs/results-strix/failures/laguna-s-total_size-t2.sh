total_size() {
    local dir="$1"
    [ -d "$dir" ] || { echo 0; return; }
    local total=0
    local f
    while IFS= read -r -d '' f; do
        if [ -f "$f" ] && [ ! -L "$f" ]; then
            local size
            size=$(stat -f%z "$f" 2>/dev/null) || continue
            total=$((total + size))
        fi
    done < <(find "$dir" -type f -print0)
    echo "$total"
}
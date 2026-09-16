total_size() {
    local dir=$1
    local total=0
    local f
    while IFS= read -r -d '' f; do
        [[ $f -f ]] && total=$((total + $(stat -f %z "$f")))
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%d\n' "$total"
}
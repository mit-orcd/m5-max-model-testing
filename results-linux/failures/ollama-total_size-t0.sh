total_size() {
    local dir="$1"
    case "$dir" in
        -*) dir="./$dir" ;;
    esac
    local total=0
    local size
    while IFS= read -r -d '' file; do
        size=$(stat -f%z "$file")
        total=$((total + size))
    done < <(find "$dir" -type f -print0)
    printf '%d\n' "$total"
}
total_size() {
    local dir="$1" total=0
    [[ ! -d "$dir" ]] && echo 0 && return
    while IFS= read -r -d '' file; do
        [[ -f "$file" ]] && ((total += $(stat -f%z "$file")))
    done < <(find "$dir" -type f -print0)
    echo "$total"
}
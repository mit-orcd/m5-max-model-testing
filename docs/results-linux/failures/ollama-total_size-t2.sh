total_size() {
    local total=0 f sz
    while IFS= read -r -d '' f; do
        sz=$(stat -f%z -- "$f")
        total=$((total + sz))
    done < <(find -- "$1" -type f -print0)
    printf '%d\n' "$total"
}
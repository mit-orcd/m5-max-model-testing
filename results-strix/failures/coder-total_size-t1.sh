total_size() {
    local total=0
    find "$1" -type f -exec stat -f '%z' {} + 2>/dev/null | while read -r size; do
        total=$((total + size))
    done
    echo "$total"
}
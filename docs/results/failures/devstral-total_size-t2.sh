total_size() {
    if [ ! -d "$1" ]; then
        echo 0
        return
    fi
    find "$1" -type f -print0 | while IFS= read -r -d '' file; do
        stat -f "%z" "$file"
    done | awk '{sum += $1} END {print sum}'
}
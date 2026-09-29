total_size() {
    dir="$1"
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        stat -f "%z" "$file" 2>/dev/null || echo 0
    done | awk '{s+=$1} END {print s}'
}
total_size() {
    local dir="$1"
    local sum=0
    local size
    while IFS= read -r -d '' file; do
        size=$(stat -f "%z" "$file" 2>/dev/null)
        if [ -n "$size" ]; then
            sum=$((sum + size))
        fi
    done < <(find "$dir" -type f -print0)
    echo "$sum"
}
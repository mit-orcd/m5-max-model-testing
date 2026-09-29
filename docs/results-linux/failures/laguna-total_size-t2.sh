total_size() {
    local dir="$1" sum=0
    [ -d "$dir" ] || { echo 0; return; }
    while IFS= read -r -d '' file; do
        if [ -f "$file" ]; then
            size=$(stat -f%z -- "$file")
            sum=$((sum + size))
        fi
    done < <(find "$dir" -mindepth 1 -print0 2>/dev/null | tr '\0' '\0')
    echo "$sum"
}
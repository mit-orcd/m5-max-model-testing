largest_file() {
    local dir="$1"
    local largest=""
    local largest_size=-1

    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            local size
            size=$(stat -c %s -- "$file" 2>/dev/null) || continue
            if (( size > largest_size )); then
                largest_size=$size
                largest=$file
            fi
        fi
    done < <(find "$dir" -type f -print0)

    printf '%s\n' "$largest"
}
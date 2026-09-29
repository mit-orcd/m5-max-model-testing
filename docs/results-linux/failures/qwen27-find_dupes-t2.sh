find_dupes() {
    local dir="$1"
    local tmp
    tmp=$(mktemp)

    # Find all regular files, compute md5, store as "md5 path"
    while IFS= read -r -d '' file; do
        local md5
        md5=$(md5 -q "$file")
        printf '%s %s\n' "$md5" "$file"
    done < <(find "$dir" -type f -print0) | sort > "$tmp"

    # Process groups: for each md5, collect paths
    local prev_md5=""
    local -a paths=()

    flush_group() {
        if [ "${#paths[@]}" -ge 2 ]; then
            # Sort paths
            local sorted
            sorted=$(printf '%s\n' "${paths[@]}" | sort)
            # Join with spaces
            local line
            line=$(printf '%s\n' "$sorted" | tr '\n' ' ' | sed 's/ $//')
            printf '%s\n' "$line"
        fi
        paths=()
    }

    while IFS=' ' read -r md5 file; do
        if [ "$md5" != "$prev_md5" ]; then
            flush_group
            prev_md5="$md5"
        fi
        paths+=("$file")
    done < "$tmp"
    flush_group

    rm -f "$tmp"
}
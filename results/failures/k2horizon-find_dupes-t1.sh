find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1

    # Collect all regular files (excluding symlinks) and compute MD5
    local tmpfile
    tmpfile=$(mktemp)
    find "$dir" -type f -print0 | while IFS= read -r -d '' f; do
        md5=$(md5 -q "$f" 2>/dev/null) || continue
        printf '%s\t%s\0' "$md5" "$f"
    done > "$tmpfile"

    # Group by hash
    : > "$tmpfile.sorted"
    sort -z -k1,1 "$tmpfile" > "$tmpfile.sorted"

    # Print groups of 2+
    local current_hash=""
    local group=()
    local line=""
    while IFS= read -r -d '' entry; do
        local hash="${entry%%	*}"
        local path="${entry#*	}"
        if [ "$hash" != "$current_hash" ]; then
            if [ ${#group[@]} -ge 2 ]; then
                printf '%s\n' "${group[*]}"
            fi
            current_hash="$hash"
            group=("$path")
        else
            group+=("$path")
        fi
    done < "$tmpfile.sorted"

    # Last group
    if [ ${#group[@]} -ge 2 ]; then
        printf '%s\n' "${group[*]}"
    fi

    rm -f "$tmpfile" "$tmpfile.sorted"
}
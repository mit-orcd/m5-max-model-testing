find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    local tmpout=$(mktemp)

    # Find all regular files, compute MD5, store "md5 path"
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | while IFS= read -r line; do
        # md5 -r outputs: <hash> <filename>
        # The hash is the first field, the rest is the filename
        hash=$(echo "$line" | awk '{print $1}')
        # Get the filename by removing the first field and leading space
        filepath=$(echo "$line" | sed 's/^[^ ]* //')
        echo "$hash $filepath"
    done > "$tmpfile"

    # Sort by hash, then by filepath
    sort -k1,1 -k2 "$tmpfile" > "$tmpout"

    # Group by hash and print groups with 2+ files
    local current_hash=""
    local current_group=()

    while IFS= read -r line; do
        hash=$(echo "$line" | awk '{print $1}')
        filepath=$(echo "$line" | sed 's/^[^ ]* //')

        if [[ "$hash" == "$current_hash" ]]; then
            current_group+=("$filepath")
        else
            if [[ ${#current_group[@]} -ge 2 ]]; then
                # Sort the group paths
                printf '%s\n' "${current_group[@]}" | sort | tr '\n' ' ' | sed 's/ $//'
                echo
            fi
            current_hash="$hash"
            current_group=("$filepath")
        fi
    done < "$tmpout"

    # Print the last group
    if [[ ${#current_group[@]} -ge 2 ]]; then
        printf '%s\n' "${current_group[@]}" | sort | tr '\n' ' ' | sed 's/ $//'
        echo
    fi

    rm -f "$tmpfile" "$tmpout"
}
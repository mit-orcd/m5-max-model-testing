find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find regular files, compute MD5, format as "hash  path"
    # Using 'md5 -r' which outputs "hash  filename" on macOS
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$tmpfile"

    # Process the sorted list to group by hash
    local prev_hash=""
    local group=()
    local first_path=""

    while IFS= read -r line; do
        # Extract hash (first field) and path (rest of line)
        local hash="${line%% *}"
        local path="${line#* }"

        if [[ "$hash" != "$prev_hash" ]]; then
            # If we have a previous group with 2+ files, print it
            if [[ ${#group[@]} -ge 2 ]]; then
                # Sort the group by path and print space-separated
                printf '%s\n' "${group[@]}" | sort | paste -sd ' ' -
            fi
            # Start new group
            group=("$path")
            first_path="$path"
            prev_hash="$hash"
        else
            group+=("$path")
        fi
    done < "$tmpfile"

    # Handle the last group
    if [[ ${#group[@]} -ge 2 ]]; then
        printf '%s\n' "${group[@]}" | sort | paste -sd ' ' -
    fi
}
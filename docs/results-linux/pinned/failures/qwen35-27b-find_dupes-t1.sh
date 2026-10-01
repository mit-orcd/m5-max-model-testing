find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT

    # Find regular files, compute MD5, output "hash path"
    # macOS 'find' does not support -printf, so we use -exec
    # macOS 'md5' with -r outputs "MD5 (file) = hash" or "MD5 (file) = hash"
    # We parse the output to get "hash path"
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
    sed 's/^MD5 (\(.*\)) = \(.*\)$/\2 \1/' | \
    sort > "$tmpfile"

    # Process the sorted list to find duplicates
    local prev_hash=""
    local group=()
    local first_path=""

    while IFS=' ' read -r hash path; do
        if [[ "$hash" == "$prev_hash" ]]; then
            group+=("$path")
        else
            # Output previous group if it has 2+ files
            if [[ ${#group[@]} -ge 2 ]]; then
                # Sort paths within the group and print
                printf '%s\n' "${group[@]}" | sort | tr '\n' ' ' | sed 's/ $/\n/'
            fi
            group=("$path")
            first_path="$path"
            prev_hash="$hash"
        fi
    done < "$tmpfile"

    # Handle the last group
    if [[ ${#group[@]} -ge 2 ]]; then
        printf '%s\n' "${group[@]}" | sort | tr '\n' ' ' | sed 's/ $/\n/'
    fi
}
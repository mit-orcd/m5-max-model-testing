find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    local temp_list=$(mktemp)

    # Generate MD5 hashes for all regular files recursively
    find "$dir" -type f -exec md5 -r {} + | awk '{print $1, $2}' > "$temp_file"

    # Sort by hash, then by filename to group identical files together
    sort "$temp_file" > "$temp_list"

    # Iterate through sorted list, grouping files with identical hashes
    local prev_hash=""
    local current_group=""
    local first_path=""

    while IFS=' ' read -r hash path; do
        if [ "$hash" = "$prev_hash" ]; then
            # Add to current group
            current_group="$current_group $path"
        else
            # If we have a previous group, check if it has more than 1 file
            if [ -n "$current_group" ]; then
                if [ $(echo "$current_group" | wc -w) -gt 1 ]; then
                    # Sort the group paths and print
                    echo "$current_group" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
                    echo
                fi
            fi
            # Start new group
            prev_hash="$hash"
            current_group="$path"
            first_path="$path"
        fi
    done < "$temp_list"

    # Handle the last group
    if [ -n "$current_group" ]; then
        if [ $(echo "$current_group" | wc -w) -gt 1 ]; then
            echo "$current_group" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
            echo
        fi
    fi

    # Cleanup
    rm "$temp_file" "$temp_list"
}
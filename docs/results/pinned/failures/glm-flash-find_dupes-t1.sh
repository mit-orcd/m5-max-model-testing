find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    local temp_list=$(mktemp)

    # Generate MD5 hashes for all regular files recursively
    find "$dir" -type f -exec md5 -r {} + | awk '{print $1, $2}' > "$temp_file"

    # Sort by hash, then by filename to group identical files together
    sort "$temp_file" > "$temp_list"

    # Iterate through sorted list, grouping files with identical hashes
    local current_hash=""
    local current_group=""
    local first_file=""

    while IFS=' ' read -r hash file; do
        if [ "$hash" != "$current_hash" ]; then
            # If we have a previous group with 2+ files, print it
            if [ -n "$current_group" ] && [ $(echo "$current_group" | wc -w) -ge 2 ]; then
                echo "$current_group"
            fi
            # Start a new group
            current_hash="$hash"
            current_group="$file"
            first_file="$file"
        else
            # Add file to current group
            current_group="$current_group $file"
        fi
    done < "$temp_list"

    # Print the last group if it has 2+ files
    if [ -n "$current_group" ] && [ $(echo "$current_group" | wc -w) -ge 2 ]; then
        echo "$current_group"
    fi

    # Cleanup
    rm "$temp_file" "$temp_list"
}
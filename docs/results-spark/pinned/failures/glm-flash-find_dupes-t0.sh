find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    local temp_list=$(mktemp)

    # Generate MD5 hashes for all regular files recursively
    find "$dir" -type f -exec md5 -r {} + | awk '{print $1, $2}' > "$temp_file"

    # Sort by hash, then by filename to group identical files together
    sort "$temp_file" > "$temp_list"

    # Iterate through sorted list, grouping files with the same hash
    while IFS= read -r hash file; do
        # Check if the next line has the same hash
        if read -r next_hash next_file; then
            if [ "$hash" = "$next_hash" ]; then
                # Group found: print current and next file
                if [ "$file" -lt "$next_file" ]; then
                    echo "$file $next_file"
                else
                    echo "$next_file $file"
                fi
            fi
        fi
    done < "$temp_list"

    rm -f "$temp_file" "$temp_list"
}
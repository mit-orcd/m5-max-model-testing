find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    local temp_list=$(mktemp)

    # Generate MD5 hashes and store paths
    find "$dir" -type f -exec md5 -r {} + | while read -r hash file; do
        echo "$hash $file"
    done > "$temp_file"

    # Sort by hash, then by path
    sort "$temp_file" > "$temp_list"

    # Process groups
    awk '
    BEGIN { prev_hash = ""; first_file = ""; count = 0; }
    {
        if ($1 != prev_hash) {
            if (count > 1) {
                # Print the previous group
                printf "%s", first_file;
                for (i = 2; i <= count; i++) {
                    printf " %s", files[i];
                }
                printf "\n";
            }
            # Start new group
            prev_hash = $1;
            first_file = $2;
            count = 1;
            files[1] = $2;
        } else {
            # Add to current group
            count++;
            files[count] = $2;
        }
    }
    END {
        if (count > 1) {
            printf "%s", first_file;
            for (i = 2; i <= count; i++) {
                printf " %s", files[i];
            }
            printf "\n";
        }
    }
    ' "$temp_list"

    rm -f "$temp_file" "$temp_list"
}
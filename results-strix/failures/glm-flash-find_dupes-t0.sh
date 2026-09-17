find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    local output_file=$(mktemp)

    # Generate MD5 hashes and store paths
    find "$dir" -type f -exec md5 -r {} + | awk '{print $1, $NF}' > "$temp_file"

    # Sort by hash, then by path
    sort "$temp_file" > "$output_file"

    # Process groups
    awk '
    BEGIN { prev_hash = ""; first_path = ""; count = 0; }
    {
        if ($1 != prev_hash) {
            if (count > 1) {
                # Print the previous group
                printf "%s", first_path;
                for (i = 2; i <= count; i++) printf " %s", paths[i];
                printf "\n";
            }
            # Start new group
            prev_hash = $1;
            first_path = $2;
            count = 1;
            paths[1] = $2;
        } else {
            # Add to current group
            count++;
            paths[count] = $2;
        }
    }
    END {
        if (count > 1) {
            printf "%s", first_path;
            for (i = 2; i <= count; i++) printf " %s", paths[i];
            printf "\n";
        }
    }
    ' "$output_file"

    rm -f "$temp_file" "$output_file"
}
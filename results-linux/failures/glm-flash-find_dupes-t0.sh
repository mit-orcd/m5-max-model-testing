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
    BEGIN { prev_hash = ""; first_path = ""; group = ""; count = 0; first = 1; }
    {
        if ($1 != prev_hash) {
            if (count > 1) {
                if (!first) printf "\n";
                print group;
                first = 0;
            }
            prev_hash = $1;
            group = $2;
            count = 1;
        } else {
            group = group " " $2;
            count++;
        }
    }
    END {
        if (count > 1) {
            if (!first) printf "\n";
            print group;
        }
    }
    ' "$output_file"

    rm -f "$temp_file" "$output_file"
}
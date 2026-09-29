find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    local output_file=$(mktemp)
    local count_file=$(mktemp)

    # 1. Generate list of md5 hashes and filenames
    # -type f: only regular files
    # -exec: execute md5 -r for each file
    find "$dir" -type f -exec md5 -r {} + | awk '{print $1, $2}' > "$temp_file"

    # 2. Sort by hash, then by filename
    sort "$temp_file" > "$output_file"

    # 3. Find groups of identical hashes (groups of 2+)
    # We iterate through the sorted list. If the hash matches the previous line,
    # we add the current filename to a temporary group. If the hash changes,
    # we finalize any pending group if it has 2+ members.
    while read -r hash file; do
        if [ "$hash" = "$prev_hash" ]; then
            if [ -z "$group" ]; then
                group="$file"
            else
                group="$group $file"
            fi
        else
            if [ -n "$group" ]; then
                # Count files in group
                echo "$group" | tr ' ' '\n' | wc -l | xargs -I {} sh -c 'if [ {} -ge 2 ]; then echo "$group"; fi'
            fi
            prev_hash="$hash"
            group="$file"
        fi
    done < "$output_file"

    # Finalize the last group
    if [ -n "$group" ]; then
        echo "$group" | tr ' ' '\n' | wc -l | xargs -I {} sh -c 'if [ {} -ge 2 ]; then echo "$group"; fi'
    fi

    rm -f "$temp_file" "$output_file" "$count_file"
}
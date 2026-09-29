find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    
    # Find all regular files, compute MD5, store hash and path
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | while IFS= read -r line; do
        # md5 -r outputs: <hash> <path>
        local hash="${line%% *}"
        local path="${line#* }"
        printf '%s\t%s\n' "$hash" "$path"
    done > "$tmpfile"
    
    # Sort by hash, then by path
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | while IFS=$'\t' read -r hash path; do
        if [[ "$hash" == "$prev_hash" ]]; then
            # Append to current group
            if [[ -z "$group" ]]; then
                group="$prev_path"
            fi
            group="$group $path"
        else
            # Output previous group if it has 2+ files
            if [[ -n "$group" ]]; then
                # Check if group has at least 2 files (count spaces)
                local count=$(echo "$group" | wc -w)
                if [[ $count -ge 2 ]]; then
                    echo "$group"
                fi
            fi
            # Start new group
            group="$path"
        fi
        prev_hash="$hash"
        prev_path="$path"
    done
    
    # Output last group
    if [[ -n "$group" ]]; then
        local count=$(echo "$group" | wc -w)
        if [[ $count -ge 2 ]]; then
            echo "$group"
        fi
    fi
    
    rm -f "$tmpfile"
}
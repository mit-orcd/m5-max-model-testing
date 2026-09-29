find_dupes() {
    local dir="$1"
    if [ ! -d "$dir" ]; then
        echo "Usage: find_dupes <directory>" >&2
        return 1
    fi

    # Find all regular files, compute MD5, output: hash filepath
    # md5 -r outputs: hash filename
    # We need to handle spaces in filenames carefully.
    
    # Create a temporary file to store results
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" RETURN

    # Find regular files recursively
    # Use find to get files, then compute md5 for each
    # To handle spaces in filenames, we use find with -exec
    
    find "$dir" -type f | while IFS= read -r filepath; do
        # Skip if not readable
        if [ ! -r "$filepath" ]; then
            continue
        fi
        # Compute md5 hash using BSD md5
        local hash
        hash=$(md5 -r "$filepath" 2>/dev/null | awk '{print $1}')
        if [ -n "$hash" ]; then
            # Store hash and filepath separated by a tab (safer than space)
            printf '%s\t%s\n' "$hash" "$filepath" >> "$tmpfile"
        fi
    done

    # Sort by hash, then by filepath
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" > "${tmpfile}.sorted"

    # Group by hash, print groups with 2+ files
    local prev_hash=""
    local group=""
    local count=0
    
    while IFS=$'\t' read -r hash filepath; do
        if [ -z "$hash" ]; then
            continue
        fi
        
        if [ "$hash" != "$prev_hash" ]; then
            # Print previous group if it had 2+ files
            if [ "$count" -ge 2 ] && [ -n "$group" ]; then
                echo "$group"
            fi
            prev_hash="$hash"
            group="$filepath"
            count=1
        else
            group="$group $filepath"
            count=$((count + 1))
        fi
    done < "${tmpfile}.sorted"

    # Print last group
    if [ "$count" -ge 2 ] && [ -n "$group" ]; then
        echo "$group"
    fi

    rm -f "${tmpfile}.sorted"
}
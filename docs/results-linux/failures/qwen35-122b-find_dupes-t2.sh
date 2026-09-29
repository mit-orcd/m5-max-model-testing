find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Use find to list regular files, then compute MD5 using BSD md5 -r
    # Format output: <hash> <path>
    find "$dir" -type f -exec sh -c '
        for f do
            hash=$(md5 -r "$f" 2>/dev/null | cut -d" " -f1)
            if [ -n "$hash" ]; then
                printf "%s\t%s\n" "$hash" "$f"
            fi
        done
    ' sh {} + | sort -k2 > "$tmpfile"

    # Process the sorted list to find duplicates
    awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            group = group " " path
            count++
        } else {
            if (count >= 2) {
                print group
            }
            group = path
            count = 1
            prev_hash = hash
        }
    }
    END {
        if (count >= 2) {
            print group
        }
    }
    ' "$tmpfile" | sort -k1,1
}
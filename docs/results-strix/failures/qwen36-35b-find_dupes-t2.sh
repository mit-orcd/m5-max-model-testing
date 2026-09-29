find_dupes() {
    local dir="$1"
    if [ -z "$dir" ]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # Use find to get all regular files, then compute md5 for each
    # md5 -r on macOS outputs: "hash  filename"
    # We need to handle filenames with spaces/newlines carefully
    # Strategy: use null-delimited find output, process each file

    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT

    # Find all regular files, compute md5, store hash and path
    # Using -print0 and read with null delimiter to handle special chars
    while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" 2>/dev/null | awk '{print $1}')
        if [ -n "$hash" ]; then
            printf '%s\t%s\0' "$hash" "$file" >> "$tmpfile"
        fi
    done < <(find "$dir" -type f -print0)

    # Now process tmpfile to find duplicates
    # Sort by hash, then by path
    # Use sort with null delimiter
    # Then group by hash, and for groups with 2+ files, sort paths and print
    
    # We'll use awk to process the null-delimited records
    # First, let's create a sorted version by hash then path
    # sort -z -t$'\t' -k1,1 -k2,2
    
    sort -z -t$'\t' -k1,1 -k2,2 "$tmpfile" | \
    awk -v RS='\0' -v FS='\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            paths[++count] = path
        } else {
            if (count >= 2) {
                # Sort paths within the group
                n = asort(paths)
                line = ""
                for (i = 1; i <= n; i++) {
                    if (i > 1) line = line " "
                    line = line paths[i]
                }
                print line
            }
            prev_hash = hash
            count = 1
            delete paths
            paths[1] = path
        }
    }
    END {
        if (count >= 2) {
            n = asort(paths)
            line = ""
            for (i = 1; i <= n; i++) {
                if (i > 1) line = line " "
                line = line paths[i]
            }
            print line
        }
    }
    '
}
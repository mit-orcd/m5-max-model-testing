find_dupes() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        return 1
    fi

    # Use a temporary file to store hash:path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' RETURN

    # Find regular files, compute MD5 using BSD syntax (md5 -r), format as "hash path"
    find "$dir" -type f -exec sh -c '
        for f do
            hash=$(md5 -r "$f" 2>/dev/null | cut -d" " -f1)
            if [ -n "$hash" ]; then
                printf "%s\t%s\n" "$hash" "$f"
            fi
        done
    ' sh {} + > "$tmpfile"

    # Sort by hash, then by path to ensure consistent grouping and ordering within groups
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
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
    }' | sort
}
find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)

    trap 'rm -f "$tmpfile"' EXIT

    # Find all regular files, compute MD5, output "hash  path"
    # Using -exec with sh to handle filenames with spaces safely
    find "$dir" -type f -exec sh -c '
        for f; do
            hash=$(md5 -r "$f" 2>/dev/null | cut -d" " -f1)
            if [ -n "$hash" ]; then
                printf "%s\t%s\n" "$hash" "$f"
            fi
        done
    ' sh {} + > "$tmpfile"

    # Sort by hash, then by path
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            paths = paths " " path
            count++
        } else {
            if (count > 1) {
                print paths
            }
            paths = path
            count = 1
            prev_hash = hash
        }
    }
    END {
        if (count > 1) {
            print paths
        }
    }' | sort
}
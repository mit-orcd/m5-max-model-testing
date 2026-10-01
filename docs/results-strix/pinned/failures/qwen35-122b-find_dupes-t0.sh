find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)

    trap 'rm -f "$tmpfile"' EXIT

    # Find regular files, compute MD5, output "hash  path"
    # Using 'md5 -r' for BSD/macOS compatibility
    find "$dir" -type f -exec sh -c '
        for f do
            hash=$(md5 -r "$f" 2>/dev/null | cut -d" " -f1)
            if [ -n "$hash" ]; then
                printf "%s\t%s\n" "$hash" "$f"
            fi
        done
    ' sh {} + > "$tmpfile"

    # Sort by hash, then by path
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
    {
        if ($1 == prev_hash) {
            paths = paths " " $2
            count++
        } else {
            if (count >= 2) {
                print paths
            }
            prev_hash = $1
            paths = $2
            count = 1
        }
    }
    END {
        if (count >= 2) {
            print paths
        }
    }'
}
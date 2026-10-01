find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find regular files, compute MD5, format as "hash  path"
    # Using -print0 and read -d '' to handle filenames with spaces/newlines
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        # md5 -r outputs "hash  filename"
        hash=$(md5 -r "$file" 2>/dev/null | cut -d' ' -f1)
        if [[ -n "$hash" ]]; then
            printf '%s\t%s\n' "$hash" "$file"
        fi
    done > "$tmpfile"

    # Sort by hash, then by path to group duplicates and ensure consistent ordering
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            paths[hash] = paths[hash] " " path
            count[hash]++
        } else {
            if (count[prev_hash] > 1) {
                print paths[prev_hash]
            }
            prev_hash = hash
            paths[hash] = path
            count[hash] = 1
        }
    }
    END {
        if (count[prev_hash] > 1) {
            print paths[prev_hash]
        }
    }' | sort
}